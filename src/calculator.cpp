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
#include <debug.hpp>
#include <function.hpp>
#include <input.hpp>
#include <iostream>
#include <numeric>
#include <parser.hpp>
#include <string>
#include <ui.hpp>

namespace smrty
{

struct column_layout
{
    explicit column_layout(size_t n) : cols(n), len(0), valid(true)
    {
    }
    std::vector<size_t> cols;
    size_t len;
    bool valid;
};

column_layout find_best_layout(std::span<std::string_view> words, size_t width)
{
    constexpr size_t PADDING_SIZE = 2; // 2 spaces
    // max columns should be less than the ideal if there is minimal padding
    size_t total_chars =
        std::accumulate(words.begin(), words.end(), 0,
                        [](size_t sum, const std::string_view& w) {
                            return sum + w.size() + PADDING_SIZE;
                        });
    size_t max_columns = words.size() * width / total_chars;
    std::vector<column_layout> layouts;
    layouts.reserve(max_columns);
    for (size_t i = 0; i < max_columns; i++)
    {
        layouts.emplace_back(i + 1);
    }
    // place each word in each layout
    for (size_t idx = 0; idx < words.size(); idx++)
    {
        size_t w_len = words[idx].size() + PADDING_SIZE;
        for (auto& layout : layouts)
        {
            size_t col_count = layout.cols.size();
            size_t row_count =
                std::ceil(static_cast<double>(words.size()) / col_count);
            size_t this_col = idx / row_count;
            if (layout.valid)
            {
                if (w_len > layout.cols[this_col])
                {
                    layout.len += w_len - layout.cols[this_col];
                    layout.cols[this_col] = w_len;
                    if (layout.len > width)
                    {
                        layout.valid = false;
                    }
                }
            }
        }
    }
    // return the best layout
    for (auto l = layouts.rbegin(); l != layouts.rend(); l++)
    {
        if (l->valid)
        {
            return *l;
        }
    }
    return column_layout(1);
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

    // add all the functions
    setup_catalog();
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
        const auto& fn_name = fn_name_by_id(n->fn_index);
        auto fn = fn_get_fn_ptr_by_name(fn_name);
        if (n->re_args.size())
        {
            lg::debug("executing function '{}({})'\n", fn_name, n->re_args);
            return fn->reop(*this, n->re_args);
        }
        else
        {
            size_t min_items = std::abs(fn->num_args());
            if (min_items > stack.size())
            {
                lg::error("{} requires at least {} items on the stack; only {} "
                          "items are currently present\n",
                          fn_name, min_items, stack.size());
                return false;
            }
            lg::debug("executing function '{}'\n", fn_name);
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
        if (auto n = std::get_if<number_parts>(&itm); n)
        {
            // put it on the stack
            e.value(make_numeric(*n), flags);
        }
        else if (auto n = std::get_if<compound_parts>(&itm); n)
        {
            // put it on the stack
            e.value(make_numeric(*n), flags);
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
        else if (auto n = std::get_if<symbolic_parts_ptr>(&itm); n)
        {
            e.value(symbolic(*n), flags);
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
    return true;
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
            auto& v = it->value();

            if (auto q = std::get_if<mpq>(&v); q)
            {
                // mpq gets special treatment to print a quotient or float
                if (config.mpq_mode == e_mpq_mode::quotient)
                {
                    ui->out("{}{:q}{}\n", row_idx, *q, it->unit());
                }
                else // floating
                {
                    ui->out("{0}{1:.{2}f}{3}\n", row_idx, *q, it->precision,
                            it->unit());
                }
            }
            else if (auto c = std::get_if<mpc>(&v); c)
            {
                // mpc gets special treatment with three print styles
                if (config.mpc_mode == e_mpc_mode::polar)
                {
                    ui->out("{0}{1:.{2}p}{3}\n", row_idx, *c, it->precision,
                            it->unit());
                }
                else if (config.mpc_mode == e_mpc_mode::rectangular)
                {
                    ui->out("{0}{1:.{2}r}{3}\n", row_idx, *c, it->precision,
                            it->unit());
                }
                else // ij mode
                {
                    ui->out("{0}{1:.{2}i}{3}\n", row_idx, *c, it->precision,
                            it->unit());
                }
            }
            else if (auto f = std::get_if<mpf>(&v); f)
            {
                ui->out("{0}{1:.{2}f}{3}\n", row_idx, *f, it->precision,
                        it->unit());
            }
            else if (auto z = std::get_if<mpz>(&v); z)
            {
                switch (it->base)
                {
                    case 2:
                        ui->out("{0}{1:#{2}b}{3}\n", row_idx, *z,
                                it->fixed_bits, it->unit());
                        break;
                    case 8:
                        ui->out("{0}{1:#{2}o}{3}\n", row_idx, *z,
                                it->fixed_bits, it->unit());
                        break;
                    case 10:
                        ui->out("{0}{1:{2}d}{3}\n", row_idx, *z, it->fixed_bits,
                                it->unit());
                        break;
                    case 16:
                        ui->out("{0}{1:#{2}x}{3}\n", row_idx, *z,
                                it->fixed_bits, it->unit());
                        break;
                }
            }
            else if (auto m = std::get_if<matrix>(&v); m)
            {
                constexpr int screen_width = 80;
                ui->out("{0}{1:{2}.{3}{:d}{:.5f}{:.5f}{:.5i}}{4}\n", row_idx,
                        *m, first_col, screen_width, it->unit());
            }
            else if (auto lst = std::get_if<list>(&v); lst)
            {
                ui->out("{}{:{:d}{:.5f}{:.5f}{:.5i}}{}\n", row_idx, *lst,
                        it->unit());
            }
            else if (auto sym = std::get_if<symbolic>(&v); sym)
            {
                ui->out("{}'{}'{}\n", row_idx, *sym, it->unit());
            }
            else
            {
                std::visit(
                    [it, ui, &row_idx](const auto& a) {
                        ui->out("{}{}{}\n", row_idx, a, it->unit());
                    },
                    v);
            }
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
