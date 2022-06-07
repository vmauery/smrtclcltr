/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <function.hpp>
#include <functions.hpp>
#include <string>

Calculator::Calculator()
{
    // set up the grammar?
    make_grammar();
    // add all the functions
    make_functions();
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
    try
    {
        if (expr.find("(") != std::string::npos)
        {
            // std::cerr << "mpc(\"" << expr << "\")\n";
#ifndef TEST_BASIC_TYPES
            e.value = mpc(expr);
#endif
        }
        else if (expr.find(".") != std::string::npos)
        {
            // std::cerr << "mpf(\"" << expr << "\")\n";
#ifndef TEST_BASIC_TYPES
            e.value = mpf(expr);
#else
            e.value = std::stod(expr);
#endif
        }
        else if (expr.find("/") != std::string::npos)
        {
            // std::cerr << "mpq(\"" << expr << "\")\n";
            e.value = mpq(expr);
        }
        else
        {
            if (config.fixed_bits)
            {
                // std::cerr << "make_fixed(mpz(\"" << expr << "\"))\n";
#ifndef TEST_BASIC_TYPES
                e.value =
                    make_fixed(mpz(expr), config.fixed_bits, config.is_signed);
#endif
            }
            else
            {
                // std::cerr << "mpz(\"" << expr << "\")\n";
#ifndef TEST_BASIC_TYPES
                e.value = mpz(expr);
#else
                e.value = std::stoi(expr);
#endif
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "bad expression '" << expr << "'\n";
        return false;
    }
    e.base = config.base;
    e.precision = config.precision;
    e.fixed_bits = config.fixed_bits;
    e.is_signed = config.is_signed;
    stack.push_front(std::move(e));
    return true;
}

std::string Calculator::get_next_token()
{
    static std::deque<std::string> current_line;
    if (current_line.size() == 0)
    {
        std::string input;
        // std::cerr << "<waiting for input>\n";
        if (!std::getline(std::cin, input))
        {
            // std::cerr << "end of input\n";
            _running = false;
            return "";
        }
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

bool Calculator::base()
{
    stack_entry e = stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
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

bool Calculator::fixed_bits()
{
    stack_entry e = stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
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
    mpz* v = std::get_if<mpz>(&e.value);
    if (v)
    {
        stack.pop_front();
        auto iv = static_cast<int>(*v);
        if (iv > 0 && iv <= 1000000)
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
    size_t c = stack.size();
    for (auto it = stack.rbegin(); it != stack.rend(); it++)
    {
        if (config.debug)
        {
            std::cout << numeric_types[it->value.index()] << " | ";
        }
        std::cout << std::dec << c << ": ";
        c--;
        switch (it->base)
        {
            case 2:
                break;
            case 8:
                std::cout << std::oct;
                break;
            case 10:
                std::cout << std::dec;
                break;
            case 16:
                std::cout << std::hex;
                break;
        }
        std::cout << std::setprecision(it->precision) << it->value << "\n";
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
    _operations["fixed_bits"] = {
        "sets the number of fixed bits",
        [](Calculator& calc) -> bool { return calc.fixed_bits(); }};
    _operations["precision"] = {
        "sets the precision",
        [](Calculator& calc) -> bool { return calc.precision(); }};
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
    _operations["factor"] = functions::factor;
    _operations["gcd"] = functions::gcd;
    _operations["lcm"] = functions::lcm;
    _operations["comb"] = functions::combination;
    _operations["perm"] = functions::permutation;
    _operations["split"] = functions::split;

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
