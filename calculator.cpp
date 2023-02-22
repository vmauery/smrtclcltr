/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <debug.hpp>
#include <function.hpp>
#include <input.hpp>
#include <iostream>
#include <numeric>
#include <string>
#include <ui.hpp>
#include <version.hpp>

// internal calculator functions
namespace function
{
struct version : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"version"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: version\n"
            "\n"
            "    Display version info\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator&) const final
    {
        ui::get()->out("Version: {}\n", Version::full());
        return true;
    }
};

struct debug : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"debug"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: debug\n"
            "\n"
            "    Toggle debug mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.debug();
    }
};

struct verbose : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"verbose"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: n verbose\n"
            "\n"
            "    Set verbosity to level n (0-7)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v)
        {
            auto lvl = static_cast<lg::level>(static_cast<int>(*v));
            if (lvl >= lg::level::emergency && lvl <= lg::level::debug)
            {
                lg::debug_level = lvl;
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument("Invalid verbosity: must be 0..7");
    }
};

struct undo : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"undo"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: undo\n"
            "\n"
            "    Undo last operation or command line\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.undo();
    }
};

struct base : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"base"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x base\n"
            "\n"
            "    Sets the numeric base to the bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.base();
    }
};

struct cbase : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cbase"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: cbase\n"
            "\n"
            "    Changes the numeric base of the bottom item to be the current base\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.cbase();
    }
};

struct fixed_bits : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"fixed_bits"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x fixed_bits\n"
            "\n"
            "    Sets the number of fixed bits to the "
            "bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.fixed_bits();
    }
};

struct precision : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"precision"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x precision\n"
            "\n"
            "    Sets the precision to the bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.precision();
    }
};

struct quotient : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"q"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: q\n"
            "\n"
            "    Print quotients as quotients instead of floats\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpq_mode(Calculator::e_mpq_mode::q);
    }
};

struct floats : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"f"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x f\n"
            "\n"
            "    Print quotients as floats instead of quotients\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpq_mode(Calculator::e_mpq_mode::f);
    }
};

struct unsigned_mode : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"unsigned"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: unsigned\n"
            "\n"
            "    Sets unsigned mode for integers\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.unsigned_mode();
    }
};

struct radians : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rad"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rad\n"
            "\n"
            "    Sets radians angle mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::rad);
    }
};

struct degrees : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"deg"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: deg\n"
            "\n"
            "    Sets degrees angle mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::deg);
    }
};

struct gradiens : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"grad"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: grad\n"
            "\n"
            "    Sets gradiens degree mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::grad);
    }
};

} // namespace function
register_calc_fn(version);
register_calc_fn(debug);
register_calc_fn(verbose);
register_calc_fn(undo);
register_calc_fn(base);
register_calc_fn(cbase);
register_calc_fn(fixed_bits);
register_calc_fn(precision);
register_calc_fn(quotient);
register_calc_fn(floats);
register_calc_fn(unsigned_mode);
register_calc_fn(radians);
register_calc_fn(degrees);
register_calc_fn(gradiens);

struct column_layout
{
    explicit column_layout(size_t n) : cols(n), len(0), valid(true)
    {
    }
    std::vector<size_t> cols;
    size_t len;
    bool valid;
};

column_layout find_best_layout(const std::vector<std::string>& words,
                               size_t width)
{
    constexpr size_t PADDING_SIZE = 2; // 2 spaces
    // max columns should be less than the ideal if there is minimal padding
    size_t total_chars = std::accumulate(
        words.begin(), words.end(), 0, [](size_t sum, const std::string& w) {
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
                ceil(static_cast<double>(words.size()) / col_count);
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
                               [this](const std::string& in, int state) {
                                   return auto_complete(in, state);
                               });

    config.precision = builtin_default_precision;
    set_default_precision(builtin_default_precision);

    // add all the functions
    make_functions();

    // set up the grammar
    make_grammar();
}

std::string binary_to_hex(const std::string& v)
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

bool Calculator::run_help()
{
    auto ui = ui::get();
    // if no arguments follow,
    auto fn = get_next_token();
    if (fn == "" || fn == "\n")
    {
        constexpr size_t WIDTH = 80; // FIXME: get this from the window
        column_layout l = find_best_layout(_op_names, WIDTH);
        size_t col_count = l.cols.size();
        size_t row_count =
            ceil(static_cast<double>(_op_names.size()) / col_count);
        for (size_t i = 0; i < row_count; i++)
        {
            for (size_t j = 0; j < col_count; j++)
            {
                size_t idx = row_count * j + i;
                if (idx >= _op_names.size())
                {
                    break;
                }
                ui->out("{0: <{1}}", _op_names[idx], l.cols[j]);
            }
            ui->out("\n");
        }
        return true;
    }
    auto help_op = _operations.find(fn);
    if (help_op != _operations.end())
    {
        ui->out("{}\n\t{}\n", help_op->second->name(), help_op->second->help());
    }
    // drain the input so stack doesn't get printed
    std::string nt;
    do
    {
        nt = get_next_token();
    } while (nt != "\n");
    return true;
}

bool Calculator::run_one(std::string expr)
{
    if (expr == "help")
    {
        return run_help();
    }
    auto calc_fn = _operations.find(expr);
    if (calc_fn != _operations.end())
    {
        lg::debug("executing function '{}'\n", expr);
        return calc_fn->second->op(*this);
    }
    // not a function
    stack_entry e;
    e.base = config.base;
    e.precision = config.precision;
    e.fixed_bits = config.fixed_bits;
    e.is_signed = config.is_signed;
    try
    {
        if (auto ubar = expr.find("_"); ubar != std::string::npos)
        {
            // parse units off the end and then let the numeric get parsed below
            std::string unitstr = expr.substr(ubar + 1);
            expr = expr.substr(0, ubar);
            e.unit(unitstr);
        }
        // time literals ns, us, ms, s, m, h, d, or
        // absolute times of the ISO8601 format: yyyy-mm-dd[Thh:mm:ss]
        if (std::optional<time_> t = parse_time(expr); t)
        {
            e.value(*t);
        }
        else if (expr.starts_with("(") || expr.ends_with("i") ||
                 expr.ends_with("j"))
        {
            lg::debug("mpc(\"{}\")\n", expr);
            e.value(parse_mpc(expr));
        }
        // FIXME: how to use locale-based decimal radix separator
        else if (expr.find(".") != std::string::npos)
        {
            lg::debug("mpf(\"{}\")\n", expr);
            e.value(parse_mpf(expr));
        }
        else if (expr.find("/") != std::string::npos)
        {
            lg::debug("mpq(\"{}\")\n", expr);
            e.value(mpq(expr));
        }
        else
        {
            if (config.fixed_bits)
            {
                lg::debug("mpz(\"{}\") {fixed}\n", expr);
                auto v = parse_mpz(expr);
                e.value(make_fixed(v, config.fixed_bits, config.is_signed));
            }
            else
            {
                lg::debug("mpz(\"{}\")\n", expr);
                std::string num;
                if (expr[0] == '0' && expr.size() > 1)
                {
                    // check for base prefix
                    if (expr.starts_with("0x"))
                    {
                        e.base = 16;
                        num = expr;
                    }
                    else if (expr.starts_with("0d"))
                    {
                        e.base = 10;
                        num = expr.substr(2);
                    }
                    else if (expr.starts_with("0b"))
                    {
                        e.base = 2;
                        num = binary_to_hex(expr);
                    }
                    else
                    {
                        e.base = 8;
                        num = expr;
                    }
                }
                else
                {
                    num = expr;
                }
                e.value(parse_mpz(num));
            }
        }
    }
    catch (const std::exception& e)
    {
        lg::error("bad expression '{}': {}\n", expr, e.what());
        return false;
    }
    stack.push_front(std::move(e));
    return true;
}

std::string Calculator::get_next_token()
{
    static std::deque<std::string> current_line;
    if (current_line.size() == 0)
    {
        std::optional<std::string> nextline = input->readline();
        if (!nextline)
        {
            _running = false;
            return "";
        }
        std::string& input = *nextline;
        boost::split(current_line, input, boost::is_any_of(" \t\n\r"));
        current_line.push_back("\n");
    }
    std::string next = current_line.front();
    current_line.pop_front();
    lg::debug("next token is :'{}'\n", next);
    return next;
}

bool Calculator::run()
{
    while (_running)
    {
        // get the next token
        std::string token = get_next_token();
        if (token == "")
        {
        }
        else if (token == "\n")
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
        else
        {
            try
            {
                // before executing the next token, save the stack
                saved_stacks.push_front(stack);
                run_one(token);
            }
            catch (const std::exception& e)
            {
                lg::error("Exception: {}\n", e.what());
            }
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
    }
    return true;
}

bool Calculator::unsigned_mode()
{
    config.is_signed = !config.is_signed;
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

bool Calculator::base()
{
    stack_entry e = stack.front();
    const mpz* v = std::get_if<mpz>(&e.value());
    if (v)
    {
        auto iv = static_cast<int>(*v);
        switch (iv)
        {
            case 2:
            case 8:
            case 10:
            case 16:
                stack.pop_front();
                config.base = iv;
                return true;
        }
    }
    return false;
}

bool Calculator::cbase()
{
    stack_entry& e = stack.front();
    e.base = config.base;
    return true;
}

bool Calculator::fixed_bits()
{
    stack_entry e = stack.front();
    const mpz* v = std::get_if<mpz>(&e.value());
    if (v)
    {
        stack.pop_front();
        auto iv = static_cast<int>(*v);
        if (iv >= 0 && iv <= 8192)
        {
            config.fixed_bits = iv;
            return true;
        }
    }
    return false;
}

bool Calculator::precision()
{
    stack_entry e = stack.front();
    const mpz* v = std::get_if<mpz>(&e.value());
    if (v)
    {
        stack.pop_front();
        auto iv = static_cast<int>(*v);
        if (iv > 0 && iv <= max_precision)
        {
            config.precision = iv;
            set_default_precision(iv);
            return true;
        }
    }
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
        try
        {
            if (config.debug)
            {
                ui->out("{} | ", numeric_types[it->value().index()]);
            }
            if (config.interactive)
            {
                ui->out("{:d}: ", c);
            }
            auto& v = it->value();

            if (auto q = std::get_if<mpq>(&v); q)
            {
                // mpq gets special treatment to print a quotient or float
                if (config.mpq_mode == e_mpq_mode::f)
                {
                    ui->out("{0:.{1}f}{2}\n", *q, it->precision, it->unit());
                }
                else
                {
                    ui->out("{:g}{}\n", *q, it->unit());
                }
            }
            else if (auto f = std::get_if<mpf>(&v); f)
            {
                ui->out("{0:.{1}f}{2}\n", *f, it->precision, it->unit());
            }
            else if (auto z = std::get_if<mpz>(&v); z)
            {
                switch (it->base)
                {
                    case 2:
                        ui->out("{:b}{}\n", *z, it->unit());
                        break;
                    case 8:
                        ui->out("{:o}{}\n", *z, it->unit());
                        break;
                    case 10:
                        ui->out("{:d}{}\n", *z, it->unit());
                        break;
                    case 16:
                        ui->out("{:x}{}\n", *z, it->unit());
                        break;
                }
            }
            else
            {
                std::visit(
                    [it, ui](const auto& a) {
                        ui->out("{}{}\n", a, it->unit());
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

extern const struct CalcFunction* __start_calc_functions;
extern const struct CalcFunction* __stop_calc_functions;

void Calculator::make_grammar()
{
}

void Calculator::make_functions()
{

    // add the functions in the __functions__ section
    for (auto iter = &__start_calc_functions; iter < &__stop_calc_functions;
         iter++)
    {
        _operations[(*iter)->name()] = *iter;
    }
    _op_names_max_strlen = 1;
    std::transform(_operations.begin(), _operations.end(),
                   std::back_inserter(_op_names), [this](const auto& kv) {
                       size_t sz = kv.first.size();
                       if (sz > _op_names_max_strlen)
                       {
                           _op_names_max_strlen = sz;
                       }
                       return kv.first;
                   });
    std::sort(_op_names.begin(), _op_names.end());
}

std::optional<std::string> Calculator::auto_complete(const std::string& in,
                                                     int state)
{
    static size_t last_idx = 0;
    static std::vector<std::string> matches;
    if (state == 0)
    {
        last_idx = 0;
        matches.clear();
        // find all the new matches
        for (auto kv : _operations)
        {
            if (kv.first.starts_with(in))
            {
                matches.emplace_back(kv.first);
            }
        }
    }
    if (last_idx < matches.size())
    {
        return matches[last_idx++];
    }
    return std::nullopt;
}
