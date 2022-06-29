/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#if HAVE_READLINE == 1
#include <readline/history.h>
#include <readline/readline.h>
#endif // HAVE_READLINE

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <function.hpp>
#include <functions.hpp>
#include <iostream>
#include <string>

Calculator::Calculator()
{
    config.interactive = isatty(STDIN_FILENO);
#if HAVE_READLINE == 1
    // don't forget to disable tab completion for now
    rl_bind_key('\t', rl_insert);
#endif // HAVE_READLINE

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

bool Calculator::run_one(const std::string& expr)
{
    if (expr == "help")
    {
        // if no arguments follow,
        auto fn = get_next_token();
        if (fn == "" || fn == "\n")
        {
            size_t mxlen = _op_names_max_strlen + 4;
            size_t cols = 80 / mxlen;
            for (size_t idx = 0; idx < _op_names.size(); idx++)
            {
                std::cout << std::left << std::setw(mxlen) << _op_names[idx];
                if (idx % cols == (cols - 1) || idx == (_op_names.size() - 1))
                {
                    std::cout << "\n";
                }
            }
            return true;
        }
        auto help_op = _operations.find(fn);
        if (help_op != _operations.end())
        {
            std::cout << help_op->first << "\n\t"
                      << std::get<0>(help_op->second) << "\n";
        }
        return true;
    }
    auto op = _operations.find(expr);
    if (op != _operations.end())
    {
        return std::get<1>(op->second)(*this);
    }
    // not a function
    stack_entry e;
    e.base = config.base;
    e.precision = config.precision;
    e.fixed_bits = config.fixed_bits;
    e.is_signed = config.is_signed;
    try
    {
        // time literals ns, us, ms, s, m, h, d
        if (expr.ends_with("ns") || expr.ends_with("us") ||
            expr.ends_with("ms") || expr.ends_with("s") ||
            expr.ends_with("m") || expr.ends_with("h") || expr.ends_with("d"))
        {
            e.value(parse_time(expr));
        }
        else if (expr.starts_with("(") || expr.ends_with("i"))
        {
            // std::cerr << "mpc(\"" << expr << "\")\n";
            e.value(parse_mpc(expr));
        }
        else if (expr.find(".") != std::string::npos)
        {
            // std::cerr << "mpf(\"" << expr << "\")\n";
            e.value(parse_mpf(expr));
        }
        else if (expr.find("/") != std::string::npos)
        {
            // std::cerr << "mpq(\"" << expr << "\")\n";
            e.value(mpq(expr));
        }
        else
        {
            if (config.fixed_bits)
            {
                // std::cerr << "make_fixed(mpz(\"" << expr << "\"))\n";
                auto v = parse_mpz(expr);
                e.value(make_fixed(v, config.fixed_bits, config.is_signed));
            }
            else
            {
                // std::cerr << "mpz(\"" << expr << "\")\n";
                std::string num;
                if (expr[0] == '0')
                {
                    // check for base prefix
                    if (expr.starts_with("0x"))
                    {
                        e.base = 16;
                        num = expr;
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
        std::cerr << "bad expression '" << expr << "': " << e.what() << "\n";
        return false;
    }
    stack.push_front(std::move(e));
    return true;
}

std::optional<std::string> Calculator::get_input()
{
    if (config.interactive)
    {
        // for interactive, use readline
#if HAVE_READLINE == 1
        char* buf = readline("> ");
        std::string nextline;
        if (buf)
        {
            nextline = buf;
            free(buf);
            if (nextline.size())
            {
                add_history(nextline.c_str());
                return nextline;
            }
            return "\n";
        }
#else
        // for non-interactive, std::getline() works fine
        // std::cerr << "<waiting for input>\n";
        std::cout << "> ";
        std::cout.flush();
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            return nextline;
        }
#endif // HAVE_READLINE
        return std::nullopt;
    }
    else
    {
        // for non-interactive, std::getline() works fine
        // std::cerr << "<waiting for input>\n";
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            return nextline;
        }
        return std::nullopt;
    }
}

std::string Calculator::get_next_token()
{
    static std::deque<std::string> current_line;
    if (current_line.size() == 0)
    {
        // std::cerr << "<waiting for input>\n";
        std::optional<std::string> nextline = get_input();
        if (!nextline)
        {
            // std::cerr << "end of input\n";
            _running = false;
            return "";
        }
        std::string& input = *nextline;
        boost::algorithm::to_lower(input);
        boost::split(current_line, input, boost::is_any_of(" \t\n\r"));
        current_line.push_back("\n");
    }
    std::string next = current_line.front();
    current_line.pop_front();
    // std::cerr << "next token is :'" << next << "'\n";
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
            show_stack();
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
                std::cerr << "Exception: " << e.what() << "\n";
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
    std::cout << "using " << MATH_BACKEND << " for numeric backend\n";
    config.debug = !config.debug;
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
    mpz* v = std::get_if<mpz>(&e.value());
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
    mpz* v = std::get_if<mpz>(&e.value());
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
    mpz* v = std::get_if<mpz>(&e.value());
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

struct binary_wrapper
{
    explicit binary_wrapper(const mpz& v) : v(v)
    {
    }
    const mpz& v;
};

std::ostream& operator<<(std::ostream& os, const binary_wrapper& bw)
{
    const mpz& v = bw.v;
    // limit binary prints to 64k bits?
    size_t bits = 0;
    mpz mask(v);
    if (mask < 0)
    {
        mask = -mask;
    }
    while (mask > 0)
    {
        mask >>= 1;
        bits++;
    }
    if (bits >= 64 * 1024)
    {
        std::stringstream ss;
        ss << v;
        os << ss.str();
        return os;
    }
    mask = 1;
    mask <<= bits;
    std::string out;
    out.reserve(bits + 3);
    if (os.flags() & std::ios_base::showbase)
    {
        out.push_back('0');
        out.push_back(os.flags() & std::ios_base::uppercase ? 'B' : 'b');
    }
    while (mask && !(mask & v))
    {
        mask >>= 1;
    }
    if (mask)
    {
        for (; mask; mask >>= 1)
        {
            out.push_back(v & mask ? '1' : '0');
        }
    }
    else
    {
        out.push_back('0');
    }
    os << out;
    return os;
}

void Calculator::show_stack()
{
    // on higher lines; truncate?
    // only show n lines where n is the screen height
    size_t c = stack.size();
    for (auto it = stack.rbegin(); it != stack.rend(); it++)
    {
        if (config.debug)
        {
            std::cout << numeric_types[it->value().index()] << " | ";
        }
        if (config.interactive)
        {
            std::cout << std::dec << c << ": ";
            c--;
        }
        auto& v = it->value();

        if (auto q = std::get_if<mpq>(&v); q)
        {
            // mpq gets special treatment to print a quotient or float
            if (config.mpq_mode == e_mpq_mode::f)
            {
                auto f = to_mpf(*q);
                std::cout << std::setprecision(it->precision) << f << "\n";
            }
            else
            {
                std::cout << std::setprecision(it->precision) << *q << "\n";
            }
        }
        else
        {
            switch (it->base)
            {
                case 2:
                    std::cout << std::showbase;
                    // only ints, only base 2, special case
                    if (auto z = std::get_if<mpz>(&v); z)
                    {
                        std::cout << std::setprecision(it->precision)
                                  << binary_wrapper(*z) << "\n";
                        continue;
                    }
                    break;
                case 8:
                    std::cout << std::showbase << std::oct;
                    break;
                case 10:
                    std::cout << std::showbase << std::dec;
                    break;
                case 16:
                    std::cout << std::showbase << std::hex;
                    break;
            }
            std::cout << std::setprecision(it->precision) << v << "\n";
        }
    }
}

void Calculator::make_grammar()
{
}

void Calculator::make_functions()
{
    _operations["debug"] = {
        "enable debug", [](Calculator& calc) -> bool { return calc.debug(); }};
    _operations["undo"] = {
        "undo last operation or command line",
        [](Calculator& calc) -> bool { return calc.undo(); }};
    _operations["base"] = {
        "sets the numeric base",
        [](Calculator& calc) -> bool { return calc.base(); }};
    _operations["cbase"] = {
        "changes the numeric base of the bottom item to be the current base",
        [](Calculator& calc) -> bool { return calc.cbase(); }};
    _operations["fixed_bits"] = {
        "sets the number of fixed bits",
        [](Calculator& calc) -> bool { return calc.fixed_bits(); }};
    _operations["precision"] = {
        "sets the precision",
        [](Calculator& calc) -> bool { return calc.precision(); }};
    _operations["q"] = {
        "prints quotients as quotients",
        [](Calculator& calc) -> bool { return calc.mpq_mode(e_mpq_mode::q); }};
    _operations["f"] = {
        "prints quotients as floats",
        [](Calculator& calc) -> bool { return calc.mpq_mode(e_mpq_mode::f); }};
    _operations["unsigned"] = {
        "sets unsigned mode",
        [](Calculator& calc) -> bool { return calc.unsigned_mode(); }};
    _operations["rad"] = {"sets radians angle mode",
                          [](Calculator& calc) -> bool {
                              return calc.angle_mode(e_angle_mode::rad);
                          }};
    _operations["deg"] = {"sets degrees angle mode",
                          [](Calculator& calc) -> bool {
                              return calc.angle_mode(e_angle_mode::deg);
                          }};
    _operations["grad"] = {"sets gradians angle mode",
                           [](Calculator& calc) -> bool {
                               return calc.angle_mode(e_angle_mode::grad);
                           }};
    _operations["clear"] = functions::clear;
    _operations["depth"] = functions::depth;
    _operations["drop"] = functions::drop;
    _operations["dup"] = functions::dup;
    _operations["over"] = functions::over;
    _operations["swap"] = functions::swap;
    _operations["+"] = functions::add;
    _operations["-"] = functions::subtract;
    _operations["*"] = functions::multiply;
    _operations["/"] = functions::divide;
    _operations[">>"] = functions::rshift;
    _operations["<<"] = functions::lshift;
    _operations["floor"] = functions::floor;
    _operations["ceil"] = functions::ceil;
    _operations["round"] = functions::round;
    _operations["range"] = functions::range;
    _operations["sum"] = functions::sum;
    _operations["prod"] = functions::product;
    _operations["sqrt"] = functions::sqrt;
    _operations["sin"] = functions::sin;
    _operations["cos"] = functions::cos;
    _operations["tan"] = functions::tan;
    _operations["asin"] = functions::asin;
    _operations["acos"] = functions::acos;
    _operations["atan"] = functions::atan;
    _operations["sinn"] = functions::sinh;
    _operations["cosh"] = functions::cosh;
    _operations["tanh"] = functions::tanh;
    _operations["asinh"] = functions::asinh;
    _operations["acosh"] = functions::acosh;
    _operations["atanh"] = functions::atanh;
    _operations["log"] = functions::log;
    _operations["ln"] = functions::ln;
    _operations["!"] = functions::factorial;
    _operations["e"] = constants::e;
    _operations["pi"] = constants::pi;
    _operations["i"] = constants::i;
    _operations["neg"] = functions::negate;
    _operations["inv"] = functions::inverse;
    _operations["%"] = functions::divmod;
    _operations["&"] = functions::bitwise_and;
    _operations["|"] = functions::bitwise_or;
    _operations["xor"] = functions::bitwise_xor;
    _operations["~"] = functions::bitwise_inv;
    _operations["^"] = functions::power;
    _operations["modexp"] = functions::modexp;
    _operations["modinv"] = functions::modinv;
    _operations["factor"] = functions::factor;
    _operations["gcd"] = functions::gcd;
    _operations["lcm"] = functions::lcm;
    _operations["comb"] = functions::combination;
    _operations["perm"] = functions::permutation;
    _operations["split"] = functions::split;
    _operations["now"] = functions::unix_ts;

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
