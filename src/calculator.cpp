/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <unistd.h>

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <config.hpp>
#include <debug.hpp>
#include <function.hpp>
#include <input.hpp>
#include <numeric>
#include <parser.hpp>
#include <string>
#include <ui.hpp>
#include <user_function.hpp>

class file_wrapper
{
  public:
    file_wrapper() = delete;
    file_wrapper(file_wrapper&&) = delete;
    file_wrapper(const std::filesystem::path& fname, const std::string& mode)
    {
        f = fopen(fname.c_str(), mode.c_str());
        if (!f)
        {
            throw std::runtime_error(std::format("Failed to open {}", fname));
        }
    }
    ~file_wrapper()
    {
        if (f)
        {
            fclose(f);
        }
    }
    operator bool() const
    {
        return f != nullptr;
    }
    operator FILE*() const
    {
        return f;
    }

    FILE* f;
};

namespace smrty
{

void Calculator::save_state(const std::filesystem::path& filename)
{
    file_wrapper cfgout(filename.c_str(), "w+");
    if (!cfgout)
    {
        lg::error("Failed to save settings to {}\n", filename);
        return;
    }
    // do this first so final settings override
    if (config.save_stack)
    {
        // traverse stack from top to bottom
        int current_base = Settings::default_base;
        int current_fixed_bits = Settings::default_fixed_bits;
        bool current_is_signed = Settings::default_is_signed;
        int current_precision = builtin_default_precision;
        for (auto it = stack.rbegin(); it != stack.rend(); it++)
        {
            const auto& entry = *it;
            // each entry has a separate precision, base, unit, signed, and bits
            // manually enter those here to ensure proper reproduction of values

            if (entry.base != current_base)
            {
                current_base = entry.base;
                std::print(cfgout, "{} base\n", entry.base);
            }
            if ((entry.fixed_bits != current_fixed_bits) ||
                (entry.is_signed != current_is_signed))
            {
                current_fixed_bits = entry.fixed_bits;
                current_is_signed = entry.is_signed;
                std::print(cfgout, "{}{}\n", (entry.is_signed ? 's' : 'u'),
                           entry.fixed_bits);
            }
            if (entry.precision != current_precision)
            {
                current_precision = entry.precision;
                std::print(cfgout, "{} precision\n", entry.precision);
            }

            std::print(cfgout, "{}\n", format_stack_entry(entry, 0));
        }
    }

    std::print(cfgout, "\n# variables\n");
    // save vars
    auto& scope = variables.front();
    for (const auto& [k, v] : scope)
    {
        std::print(cfgout, "{} '{}' sto\n", v, k);
    }

    // user-defined functions
    std::print(cfgout, "\n# user-defined functions\n");
    auto user_fns = fn_get_all_user();
    if (user_fns.size() != 0)
    {
        std::vector<std::string_view> fn_names;
        fn_names.reserve(user_fns.size());
        for (const auto& ptr : user_fns)
        {
            auto fn = dynamic_pointer_cast<const UserFunction>(ptr);
            std::print(cfgout, "{} '{}' def\n", fn->function, fn->name());
        }
    }

    // settings
    std::print(cfgout, "\n# settings\n");
    std::print(cfgout, "{} save_stack\n", config.save_stack);
    std::print(cfgout, "{} base\n", config.base);
    std::print(cfgout, "{}{}\n", (config.is_signed ? 's' : 'u'),
               config.fixed_bits);
    std::print(cfgout, "{} precision\n", config.precision);
    if (config.angle_mode == e_angle_mode::degrees)
    {
        std::print(cfgout, "deg\n");
    }
    else if (config.angle_mode == e_angle_mode::gradians)
    {
        std::print(cfgout, "grad\n");
    }
    else
    {
        std::print(cfgout, "rad\n");
    }
    if (config.mpq_mode == e_mpq_mode::quotient)
    {
        std::print(cfgout, "q\n");
    }
    else
    {
        std::print(cfgout, "f\n");
    }
    if (config.mpc_mode == e_mpc_mode::polar)
    {
        std::print(cfgout, "polar\n");
    }
    else if (config.mpc_mode == e_mpc_mode::ij)
    {
        std::print(cfgout, "ij\n");
    }
    else
    {
        std::print(cfgout, "rectangular\n");
    }
    if (config.debug)
    {
        std::print(cfgout, "debug\n");
    }
}

Calculator::Calculator()
{
    config.interactive = isatty(STDIN_FILENO);
    input = Input::make_shared(config.interactive,
                               [this](std::string_view in, int state) {
                                   return auto_complete(in, state);
                               });

    config.precision = builtin_default_precision;
    set_default_precision(builtin_default_precision);

    // add a top-level variable scope
    variables.emplace_front();

    // add all the functions
    setup_catalog();
}

Calculator::~Calculator()
{
}

std::string binary_to_hex(std::string_view v)
{
    std::string out;
    out.reserve(v.size() / 4 + 4);
    // top nibble might not be full
    // The rest will be a full 4-bits each
    size_t bits = v.size();
    auto next = v.begin();
    if (v.starts_with("0b"))
    {
        next += 2;
        bits -= 2;
    }
    // pick off first bits
    int nibble = 0;
    for (size_t i = (bits & 0xfffffffc); i < bits; i++)
    {
        nibble <<= 1;
        nibble += (*next++ & 1);
    }
    // round down to nearest 4 bits
    bits &= 0xfffffffc;
    constexpr char itohex[] = "0123456789abcdef";
    out.push_back('0');
    out.push_back('x');
    out.push_back(itohex[nibble]);
    while (bits)
    {
        nibble = 0;
        for (size_t i = 0; i < 4; i++)
        {
            nibble <<= 1;
            nibble += (*next++ & 1);
        }
        out.push_back(itohex[nibble]);
        bits -= 4;
    }
    return out;
}

bool Calculator::run_help(std::string_view fn)
{
    auto ui = ui::get();
    // get the next token, but drain the input so stack doesn't get printed
    lg::debug("help item is: {}\n", fn);
    if (fn.size())
    {
        auto help_op = fn_get_fn_ptr_by_name(fn);
        if (help_op)
        {
            ui->out("{}\n\t{}\n", help_op->name(), help_op->help());
            return false;
        }
    }
    auto size = ui->size();
    auto& tcols = std::get<1>(size);
    auto op_names = fn_get_all_names();
    column_layout l = find_best_layout(op_names, tcols);
    size_t col_count = l.cols.size();
    size_t row_count =
        std::ceil(static_cast<double>(op_names.size()) / col_count);
    for (size_t i = 0; i < row_count; i++)
    {
        for (size_t j = 0; j < col_count; j++)
        {
            size_t idx = row_count * j + i;
            if (idx >= op_names.size())
            {
                break;
            }
            ui->out("{0: <{1}}", op_names[idx], l.cols[j]);
        }
        ui->out("\n");
    }
    return false;
}

bool Calculator::run_one(const simple_instruction& itm)
{
    lg::debug("Calculator::run_one({})\n", itm);
    if (lg::debug_level == lg::level::debug)
    {
        try
        {
            show_stack();
            lg::debug("flags before: z({}) c({}) o({}) s({})\n", flags.zero,
                      flags.carry, flags.overflow, flags.sign);
        }
        catch (const std::exception& e)
        {
            lg::error("Exception: {}\n", e.what());
        }
    }
    if (auto n = std::get_if<function_parts>(&itm); n)
    {
        // operation should always be present,
        auto fn = n->fn_ptr;
        auto fname = fn_get_name(fn);
        if (n->re_args.size())
        {
            lg::debug("executing function '{}({})'\n", fname, n->re_args);
            return fn->reop(*this, n->re_args);
        }
        else
        {
            size_t min_items = std::abs(fn->num_args());
            if (min_items > stack.size())
            {
                lg::error("{} requires at least {} items on the stack; only {} "
                          "items are currently present\n",
                          fname, min_items, stack.size());
                return false;
            }
            lg::debug("executing function '{}'\n", fname);
            return fn->op(*this);
        }
        return false;
    }
    try
    {
        stack_entry e;
        e.base = config.base;
        e.precision = config.precision;
        e.fixed_bits = config.fixed_bits;
        e.is_signed = config.is_signed;
        if (auto n = std::get_if<mpx>(&itm); n)
        {
            // put it on the stack
            e.value(variant_cast(*n), flags);
        }
        else if (auto n = std::get_if<matrix>(&itm); n)
        {
            // put it on the stack
            e.value(*n, flags);
        }
        else if (auto n = std::get_if<list>(&itm); n)
        {
            // put it on the stack
            e.value(*n, flags);
        }
        else if (auto n = std::get_if<time_parts>(&itm); n)
        {
            // put it on the stack
            e.value(make_numeric(*n), flags);
        }
        else if (auto n = std::get_if<program>(&itm); n)
        {
            // put it on the stack
            e.value(*n, flags);
        }
        else if (auto n = std::get_if<bool>(&itm); n)
        {
            e.value(*n, flags);
        }
        else if (auto n = std::get_if<symbolic>(&itm); n)
        {
            e.value(*n, flags);
        }
        stack.push_front(std::move(e));
    }
    catch (const std::exception& e)
    {
        lg::error("failed to parse '{}': {}\n", itm, e.what());
        return false;
    }
    lg::debug("flags after: z({}) c({}) o({}) s({})\n", flags.zero, flags.carry,
              flags.overflow, flags.sign);
    return true;
}

bool Calculator::run(std::string_view command_line)
{
    // command line presence removes interactivity
    if (command_line.size() > 0)
    {
        if (config.interactive)
        {
            config.interactive = false;
            // force end-of-file so input->readline() returns without a keypress
            std::cin.setstate(std::ios_base::eofbit);
        }
        input->set_interactive(false);
    }
    auto ui = ui::get();
    auto& cfg = Config::get();
    if (config.interactive)
    {
        std::optional<std::string_view> cfgline;
        while ((cfgline = cfg->readline()))
        {
            std::string errmsg{};
            auto maybe_program = parser::parse_user_input(
                *cfgline, [&errmsg](std::string_view msg) { errmsg = msg; });
            if (!maybe_program || errmsg.size())
            {
                if (errmsg.size())
                {
                    ui->out("{}\n", errmsg);
                }
                else
                {
                    ui->out("Invalid config line: {}\n", *cfgline);
                }
                continue;
            }
            // attempt to execute the program
            // if the program is aborted, do not print the stack
            try
            {
                maybe_program->execute(
                    [this](const simple_instruction& itm,
                           execution_flags& eflags) {
                        bool retval = run_one(itm);
                        eflags = flags;
                        return retval;
                    },
                    flags);
            }
            catch (const std::exception& e)
            {
                lg::error("Parsing config: exception: {}\n", e.what());
            }
        }
        if (stack.size())
        {
            try
            {
                show_stack();
            }
            catch (const std::exception& e)
            {
                lg::error("Exception: {}\n", e.what());
            }
        }
    }
    while (_running)
    {
        std::optional<std::string> nextline = input->readline();
        if (!nextline)
        {
            // squish the command line in as the last bit of stdin
            if (command_line.size())
            {
                nextline = std::string{command_line};
                command_line = "";
            }
            else
            {
                break;
            }
        }
        bool exe_ok = true;
        if (nextline->size())
        {
            std::string errmsg{};
            auto maybe_program = parser::parse_user_input(
                *nextline, [&errmsg](std::string_view msg) { errmsg = msg; });
            if (!maybe_program || errmsg.size())
            {
                if (errmsg.size())
                {
                    ui->out("{}\n", errmsg);
                }
                else
                {
                    ui->out("Invalid input: {}\n", *nextline);
                }
                continue;
            }
            // attempt to execute the program
            // if the program is aborted, do not print the stack
            try
            {
                saved_stacks.push_front(stack);
                exe_ok = maybe_program->execute(
                    [this](const simple_instruction& itm,
                           execution_flags& eflags) {
                        bool retval = run_one(itm);
                        eflags = flags;
                        return retval;
                    },
                    flags);
            }
            catch (const std::exception& e)
            {
                lg::error("Exception: {}\n", e.what());
                saved_stacks.pop_front();
            }
        }
        if (exe_ok && config.interactive)
        {
            try
            {
                show_stack();
            }
            catch (const std::exception& e)
            {
                lg::error("Exception: {}\n", e.what());
            }
        }
    }
    if (!config.interactive)
    {
        try
        {
            show_stack();
        }
        catch (const std::exception& e)
        {
            lg::error("Exception: {}\n", e.what());
        }
    }
    save_state(cfg->path_of("config"));
    return true;
}

void Calculator::var_scope_enter()
{
    variables.emplace_front();
}

void Calculator::var_scope_exit()
{
    variables.pop_front();
}

std::vector<std::string_view> Calculator::get_var_names()
{
    std::unordered_set<std::string_view> nameset;
    for (const auto& scope : variables)
    {
        for (const auto& [k, v] : scope)
        {
            nameset.emplace(k);
        }
    }
    std::vector<std::string_view> names(nameset.begin(), nameset.end());
    std::sort(names.begin(), names.end());
    return names;
}

std::optional<numeric> Calculator::get_var(std::string_view name)
{
    for (const auto& scope : variables)
    {
        for (const auto& [k, v] : scope)
        {
            if (k == name)
            {
                return v;
            }
        }
    }
    return std::nullopt;
}

void Calculator::set_var(std::string_view name, const numeric& value)
{
    auto& scope = variables.front();
    scope[std::string(name)] = value;
    lg::debug("set_var('{}', {})\n", name, value);
}

// unset only affects the current scope
void Calculator::unset_var(std::string_view name)
{
    lg::debug("unset_var('{}')\n", name);
    auto& scope = variables.front();
    if (auto p = scope.find(std::string(name)); p != scope.end())
    {
        scope.erase(p);
    }
}

bool Calculator::undo()
{
    if (saved_stacks.size() == 0)
    {
        return false;
    }
    // first, remove the stack that was saved just prior to this executing
    saved_stacks.pop_front();
    // then, restore the stack that would have been there
    // prior to the previous command
    stack = saved_stacks[0];
    saved_stacks.pop_front();
    return true;
}

bool Calculator::debug()
{
    auto ui = ui::get();
    config.debug = !config.debug;
    ui->out("debug mode {}\n", (config.debug ? "on" : "off"));
    if (config.debug)
    {
        ui->out("using {} for numeric backend\n", MATH_BACKEND);
        lg::debug_level = lg::level::debug;
    }
    else
    {
        lg::debug_level = lg::level::error;
    }
    return true;
}

bool Calculator::signed_mode(bool is_signed)
{
    config.is_signed = is_signed;
    return true;
}

bool Calculator::angle_mode(e_angle_mode mode)
{
    config.angle_mode = mode;
    return true;
}

bool Calculator::mpq_mode(e_mpq_mode mode)
{
    config.mpq_mode = mode;
    return true;
}

bool Calculator::mpc_mode(e_mpc_mode mode)
{
    config.mpc_mode = mode;
    return true;
}

bool Calculator::base(unsigned int b)
{
    switch (b)
    {
        case 2:
        case 8:
        case 10:
        case 16:
            config.base = b;
            parser::set_current_base(b);
            return true;
    }
    return false;
}

bool Calculator::cbase()
{
    stack_entry& e = stack.front();
    e.base = config.base;
    return true;
}

bool Calculator::fixed_bits(unsigned int bits)
{
    bits = std::min(bits, max_bits);
    config.fixed_bits = bits;
    return true;
}

bool Calculator::precision(unsigned int p)
{
    p = std::min(p, max_precision);
    config.precision = p;
    set_default_precision(p);
    return false;
}

bool Calculator::tz_mode()
{
    config.local_time ^= true;
    return true;
}

std::string Calculator::format_stack_entry(const stack_entry& e,
                                           size_t first_col)
{
    auto& v = e.value();
    auto& u = e.unit();
    if (auto q = std::get_if<mpq>(&v); q)
    {
        // mpq gets special treatment to print a quotient or float
        if (config.mpq_mode == e_mpq_mode::quotient)
        {
            return std::format("{:q}{}", *q, u);
        }
        // floating
        return std::format("{0:.{1}f}{2}", *q, e.precision, u);
    }
    if (auto c = std::get_if<mpc>(&v); c)
    {
        // mpc gets special treatment with three print styles
        if (config.mpc_mode == e_mpc_mode::polar)
        {
            return std::format("{0:.{1}p}{2}", *c, e.precision, u);
        }
        if (config.mpc_mode == e_mpc_mode::rectangular)
        {
            return std::format("{0:.{1}r}{2}", *c, e.precision, u);
        }
        // ij mode
        return std::format("{0:.{1}i}{2}", *c, e.precision, u);
    }
    if (auto f = std::get_if<mpf>(&v); f)
    {
        return std::format("{0:.{1}f}{2}", *f, e.precision, u);
    }
    if (auto z = std::get_if<mpz>(&v); z)
    {
        if (e.base == 2)
        {
            return std::format("{0:#{1}b}{2}", *z, e.fixed_bits, u);
        }
        if (e.base == 8)
        {
            return std::format("{0:#{1}o}{2}", *z, e.fixed_bits, u);
        }
        if (e.base == 10)
        {
            return std::format("{0:{1}d}{2}", *z, e.fixed_bits, u);
        }
        if (e.base == 16)
        {
            return std::format("{0:#{1}x}{2}", *z, e.fixed_bits, u);
        }
        return std::format("{}{}", *z, u);
    }
    if (auto m = std::get_if<matrix>(&v); m)
    {
        if (first_col == 0)
        {
            // one-line format
            return std::format("{}", *m);
        }
        // fancy format
        const auto& [rows, cols] = ui::get()->size();
        return std::format_runtime("{0:{1}.{2}{:d}{:.5f}{:.5f}{:.5i}}{3}", *m,
                                   first_col, cols, u);
    }
    if (auto lst = std::get_if<list>(&v); lst)
    {
        return std::format_runtime("{:{:d}{:.5f}{:.5f}{:.5i}}{}", *lst, u);
    }
    if (auto sym = std::get_if<symbolic>(&v); sym)
    {
        return std::format("{}{}", *sym, u);
    }
    if (auto tm = std::get_if<time_>(&v); tm)
    {
        if (config.local_time)
        {
            return std::format("{:l}", *tm);
        }
        else
        {
            return std::format("{:g}", *tm);
        }
    }
    return std::visit([&u](const auto& a) { return std::format("{}{}", a, u); },
                      v);
}

void Calculator::show_stack()
{
    // on higher lines; truncate?
    // only show n lines where n is the screen height
    auto ui = ui::get();
    size_t c = stack.size();
    for (auto it = stack.rbegin(); it != stack.rend(); it++)
    {
        size_t first_col = 0;
        try
        {
            if (config.debug)
            {
                const char* base{};
                switch (it->base)
                {
                    case 2:
                        base = "bin";
                        break;
                    case 8:
                        base = "oct";
                        break;
                    case 10:
                        base = "dec";
                        break;
                    case 16:
                        base = "hex";
                        break;
                }
                auto debug_prefix =
                    std::format("{}{},p:{},{},{} | ", it->is_signed ? 's' : 'u',
                                it->fixed_bits, it->precision, base,
                                numeric_types[it->value().index()]);
                ui->out(debug_prefix);
                first_col += debug_prefix.size();
            }
            std::string row_idx{};
            if (config.interactive)
            {
                row_idx = std::format("{:d}: ", c);
                first_col += row_idx.size();
            }
            ui->out("{}{}\n", row_idx, format_stack_entry(*it, first_col));
        }
        catch (const std::exception& e)
        {
            lg::error("show_stack[{}]: {}\n", c, e.what());
        }
        c--;
    }
}

std::optional<std::string_view> Calculator::auto_complete(std::string_view in,
                                                          int state)
{
    static size_t last_idx = 0;
    static std::span<std::string_view> matches;
    if (state == 0)
    {
        last_idx = 0;
        // find all the new matches
        matches = fn_list_all_starts_with(in);
    }
    if (last_idx < matches.size())
    {
        return matches[last_idx++];
    }
    return std::nullopt;
}

} // namespace smrty
