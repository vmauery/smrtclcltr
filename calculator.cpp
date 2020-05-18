/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/algorithm/string.hpp>
#include <calculator.hpp>
#include <string>

calculator::calculator()
{
    // set up the grammar?
    // add all the functions
    _operations["debug"] = [this]() -> bool { return debug(); };
    _operations["base"] = [this]() -> bool { return base(); };
    _operations["fixed_bits"] = [this]() -> bool { return fixed_bits(); };
    _operations["precision"] = [this]() -> bool { return precision(); };
    _operations["unsigned"] = [this]() -> bool { return unsigned_mode(); };
    _operations["+"] = [this]() -> bool { return add(); };
    _operations["-"] = [this]() -> bool { return subtract(); };
    _operations["*"] = [this]() -> bool { return multiply(); };
    _operations["/"] = [this]() -> bool { return divide(); };
}

bool calculator::run_one(const std::string& expr)
{
    auto op = _operations.find(expr);
    if (op != _operations.end())
    {
        return op->second();
    }
    // not a function
    stack_entry e;
    if (expr.find("(") != std::string::npos)
    {
        std::cerr << "mpc(\"" << expr << "\")\n";
        e.value = mpc(expr);
    }
    else if (expr.find(".") != std::string::npos)
    {
        std::cerr << "mpf(\"" << expr << "\")\n";
        e.value = mpf(expr);
    }
    else if (expr.find("/") != std::string::npos)
    {
        std::cerr << "mpq(\"" << expr << "\")\n";
        e.value = mpq(expr);
    }
    else
    {
        if (_fixed_bits)
        {
            std::cerr << "make_fixed(mpz(\"" << expr << "\"))\n";
            e.value = make_fixed(mpz(expr), _fixed_bits, _is_signed);
        }
        else
        {
            std::cerr << "mpz(\"" << expr << "\")\n";
            e.value = mpz(expr);
        }
    }
    e.base = _base;
    e.precision = _precision;
    e.fixed_bits = _fixed_bits;
    e.is_signed = _is_signed;
    _stack.push_front(std::move(e));
    return true;
}

std::string calculator::get_next_token()
{
    static std::deque<std::string> current_line;
    if (current_line.size() == 0)
    {
        std::string input;
        std::cerr << "<waiting for input>\n";
        if (!std::getline(std::cin, input))
        {
            std::cerr << "end of input\n";
            _running = false;
            return "";
        }
        boost::split(current_line, input, boost::is_any_of(" \t\n\r"));
        current_line.push_back("\n");
    }
    std::string next = current_line.front();
    current_line.pop_front();
    std::cerr << "next token is :'" << next << "'\n";
    return next;
}

bool calculator::run()
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
            run_one(token);
        }
    }
    return true;
}

bool calculator::debug()
{
    _debug = !_debug;
    return true;
}

bool calculator::unsigned_mode()
{
    _is_signed = !_is_signed;
    return true;
}

bool calculator::base()
{
    stack_entry e = _stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
    if (v)
    {
        _stack.pop_front();
        auto iv = static_cast<int>(*v);
        switch (iv)
        {
            case 2:
            case 8:
            case 10:
            case 16:
                _base = iv;
                return true;
        }
    }
    return false;
}

bool calculator::fixed_bits()
{
    stack_entry e = _stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
    if (v)
    {
        _stack.pop_front();
        auto iv = static_cast<int>(*v);
        if (iv >= 0 && iv <= 8192)
        {
            _fixed_bits = iv;
            return true;
        }
    }
    return false;
}

bool calculator::precision()
{
    stack_entry e = _stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
    if (v)
    {
        _stack.pop_front();
        auto iv = static_cast<int>(*v);
        if (iv > 0 && iv <= 1000000)
        {
            _precision = iv;
            return true;
        }
    }
    return false;
}

void calculator::show_stack()
{
    // on higher lines; truncate?
    // only show n lines where n is the screen height
    size_t c = _stack.size();
    for (auto it = _stack.rbegin(); it != _stack.rend(); it++)
    {
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
        std::cout << it->value << "\n";
    }
}

bool calculator::add()
{
    return two_arg_op(
        [](const auto& a, const auto& b) -> numeric { return a + b; });
}

bool calculator::subtract()
{
    return two_arg_op(
        [](const auto& a, const auto& b) -> numeric { return a - b; });
}

bool calculator::multiply()
{
    return two_arg_op(
        [](const auto& a, const auto& b) -> numeric { return a * b; });
}

bool calculator::divide()
{
    return two_arg_op(
        [](const auto& a, const auto& b) -> numeric { return a / b; });
}
