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
#include <string>
#include <ui.hpp>

extern const struct smrty::CalcFunction* __start_calc_functions;
extern const struct smrty::CalcFunction* __stop_calc_functions;

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

column_layout find_best_layout(const std::vector<std::string_view>& words,
                               size_t width)
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
    make_functions();

    // set up the grammar
    parser = std::make_unique<Parser>(10, _op_names, _reops);
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

bool Calculator::run_help()
{
    auto ui = ui::get();
    // get the next token, but drain the input so stack doesn't get printed
    auto next = get_next_token(true);
    lg::debug("help token is: {}\n", *next);
    if (next->type == Parser::eol_type || next->type == Parser::null_type)
    {
        constexpr size_t WIDTH = 80; // FIXME: get this from the window
        column_layout l = find_best_layout(_op_names, WIDTH);
        size_t col_count = l.cols.size();
        size_t row_count =
            std::ceil(static_cast<double>(_op_names.size()) / col_count);
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
    auto help_op = _operations.find(next->value);
    if (help_op != _operations.end())
    {
        ui->out("{}\n\t{}\n", help_op->second->name(), help_op->second->help());
    }
    return true;
}

bool Calculator::run_one(std::shared_ptr<Token>& token)
{
    if (token->type == Parser::cmd_type)
    {
        // operation should always be present,
        // but find is the same speed as []
        auto opiter = _operations.find(token->value);
        if (opiter == _operations.end())
        {
            lg::error("function not found: '{}'\n", token->value);
            return false;
        }
        if (token->matches.size())
        {
            lg::debug("executing function '{}({})'\n", token->value,
                      token->matches[0]);
            return opiter->second->reop(*this, token->matches);
        }
        else
        {
            lg::debug("executing function '{}'\n", token->value);
            return opiter->second->op(*this);
        }
    }
    // not a function
    stack_entry e;
    e.base = config.base;
    e.precision = config.precision;
    e.fixed_bits = config.fixed_bits;
    e.is_signed = config.is_signed;
    std::string expr = token->value;
    try
    {
        if (auto ubar = expr.find("_"); ubar != std::string::npos)
        {
            // parse units off the end and then let the numeric get parsed below
            std::string unitstr = expr.substr(ubar + 1);
            expr = expr.substr(0, ubar);
            e.unit(unitstr);
        }
        if (token->type == Parser::mpz_type)
        {
            lg::debug("mpz(\"{}\")\n", expr);
            std::string_view num;
            int base{config.base};
            if (expr[0] == '0' && expr.size() > 1)
            {
                // check for base prefix
                if (expr.starts_with("0x"))
                {
                    base = e.base = 16;
                    num = expr;
                }
                else if (expr.starts_with("0d"))
                {
                    base = e.base = 10;
                    num = expr.substr(2);
                }
                else if (expr.starts_with("0b"))
                {
                    base = e.base = 2;
                    num = binary_to_hex(expr);
                }
                else
                {
                    base = e.base = 8;
                    num = expr;
                }
            }
            else
            {
                num = expr;
            }
            e.value(parse_mpz(num, base));
        }
        else if (token->type == Parser::mpf_type)
        {
            lg::debug("mpf(\"{}\")\n", expr);
            e.value(parse_mpf(expr));
        }
        else if (token->type == Parser::mpc_type)
        {
            lg::debug("mpc(\"{}\")\n", expr);
            e.value(parse_mpc(expr));
        }
        else if (token->type == Parser::mpq_type)
        {
            lg::debug("mpq(\"{}\")\n", expr);
            e.value(mpq(expr));
        }
        else if (token->type == Parser::matrix_type)
        {
            e.value(parse_matrix(expr));
        }
        else if (token->type == Parser::list_type)
        {
        }
        else if (token->type == Parser::time_type)
        {
            // time literals ns, us, ms, s, m, h, d, or
            // absolute times of the ISO8601 format: yyyy-mm-dd[Thh:mm:ss]
            if (std::optional<time_> t = parse_time(expr); t)
            {
                e.value(*t);
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

std::shared_ptr<Token> Calculator::get_next_token(bool drain)
{
    static std::deque<std::shared_ptr<Token>> current_line{};
    if (current_line.size() == 0)
    {
        std::optional<std::string> nextline = input->readline();
        if (!nextline)
        {
            _running = false;
            return std::make_shared<Token>();
        }
        std::string_view next = *nextline;
        do
        {
            const auto& [parse_ok, t, more] = parser->parse_next(next);
            if (parse_ok)
            {
                lg::debug("parsed: {}\n", *t);
            }
            next = more;
            if (!parse_ok)
            {
                lg::debug("\n");
                auto ui = ui::get();
                ui->out("Failed to parse at '{}'\n", more);
                current_line.clear();
                return std::make_shared<Token>();
            }
            current_line.push_back(t);
        } while (next.size());
        current_line.push_back(std::make_shared<Token>(Parser::eol_type));
    }
    auto next_token = current_line.front();
    if (drain)
    {
        current_line.clear();
    }
    else
    {
        current_line.pop_front();
    }
    lg::debug("next token is: {}\n", *next_token);
    return next_token;
}

bool Calculator::run()
{
    while (_running)
    {
        // get the next token
        std::shared_ptr<Token> token = get_next_token(false);
        if (token->type == Parser::null_type)
        {
        }
        else if (token->type == Parser::eol_type)
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
            parser->rebuild(b);
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
                ui->out("{0}{1:{2}}{3}\n", row_idx, *m, first_col, it->unit());
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

    for (const auto& [k, v] : _operations)
    {
        auto re = v->regex();
        if (re)
        {
            auto first = _op_names.begin();
            auto iter = std::find(first, _op_names.end(), k);
            _reops.emplace(std::distance(first, iter), re);
        }
    }
}

std::optional<std::string_view> Calculator::auto_complete(std::string_view in,
                                                          int state)
{
    static size_t last_idx = 0;
    static std::vector<std::string_view> matches;
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

} // namespace smrty
