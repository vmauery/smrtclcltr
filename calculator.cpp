/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <boost/algorithm/string.hpp>
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <string>

calculator::calculator()
{
    // set up the grammar?
    make_grammar();
    // add all the functions
    make_functions();
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
            if (_fixed_bits)
            {
                // std::cerr << "make_fixed(mpz(\"" << expr << "\"))\n";
#ifndef TEST_BASIC_TYPES
                e.value = make_fixed(mpz(expr), _fixed_bits, _is_signed);
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

bool calculator::angle_mode(e_angle_mode mode)
{
    _angle_mode = mode;
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

void calculator::make_grammar()
{
}

void calculator::make_functions()
{
    _operations["debug"] = [this]() -> bool { return debug(); };
    _operations["base"] = [this]() -> bool { return base(); };
    _operations["fixed_bits"] = [this]() -> bool { return fixed_bits(); };
    _operations["precision"] = [this]() -> bool { return precision(); };
    _operations["unsigned"] = [this]() -> bool { return unsigned_mode(); };
    _operations["rad"] = [this]() -> bool {
        return angle_mode(e_angle_mode::rad);
    };
    _operations["deg"] = [this]() -> bool {
        return angle_mode(e_angle_mode::deg);
    };
    _operations["grad"] = [this]() -> bool {
        return angle_mode(e_angle_mode::grad);
    };
    _operations["drop"] = [this]() -> bool {
        if (_stack.size() < 1)
        {
            return false;
        }
        stack_entry a = _stack.front();
        _stack.pop_front();
        return true;
    };
    _operations["swap"] = [this]() -> bool {
        if (_stack.size() < 2)
        {
            return false;
        }
        stack_entry a = _stack.front();
        _stack.pop_front();
        stack_entry b = _stack.front();
        _stack.pop_front();
        _stack.push_front(std::move(a));
        _stack.push_front(std::move(b));
        return true;
    };
    _operations["+"] = [this]() -> bool {
        return two_arg_op([](const auto& a, const auto& b) { return a + b; });
    };
    _operations["-"] = [this]() -> bool {
        return two_arg_op([](const auto& a, const auto& b) { return a - b; });
    };
    _operations["*"] = [this]() -> bool {
        return two_arg_op([](const auto& a, const auto& b) { return a * b; });
    };
    _operations["/"] = [this]() -> bool {
        return two_arg_conv_op(
            [](const auto& a, const auto& b) { return a / b; },
            std::tuple<mpz>{}, std::tuple<mpq>{}, std::tuple<mpq, mpf, mpc>{});
    };
    _operations["sqrt"] = [this]() -> bool {
        return one_arg_conv_op(
            [](const auto& a) -> numeric {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    return sqrt(a);
                }
                else
                {
                    if (a >= decltype(a)(0))
                    {
                        return sqrt(mpf{a});
                    }
                    else
                    {
                        return sqrt(mpc{a});
                    }
                }
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["sin"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op(a, [](const auto& a) { return sin(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["cos"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op(a, [](const auto& a) { return cos(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["tan"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op(a, [](const auto& a) { return tan(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["asin"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    a, [](const auto& a) { return asin(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["acos"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    a, [](const auto& a) { return acos(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["atan"] = [this]() -> bool {
        return one_arg_conv_op(
            [this](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    a, [](const auto& a) { return atan(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
    _operations["e"] = [this]() -> bool {
        stack_entry e(boost::math::constants::e<mpf>(), _base, _fixed_bits,
                      _precision, _is_signed);
        _stack.push_front(std::move(e));
        return true;
    };
    _operations["pi"] = [this]() -> bool {
        stack_entry e(boost::math::constants::pi<mpf>(), _base, _fixed_bits,
                      _precision, _is_signed);
        _stack.push_front(std::move(e));
        return true;
    };
    _operations["neg"] = [this]() -> bool {
        return one_arg_op([](const auto& a) { return -a; });
    };
    _operations["%"] = [this]() -> bool {
        return two_arg_limited_op<mpz>(
            [](const auto& a, const auto& b) { return a % b; });
    };
    _operations["&"] = [this]() -> bool {
        return two_arg_limited_op<mpz>(
            [](const auto& a, const auto& b) { return a & b; });
    };
    _operations["|"] = [this]() -> bool {
        return two_arg_limited_op<mpz>(
            [](const auto& a, const auto& b) { return a | b; });
    };
    _operations["xor"] = [this]() -> bool {
        return two_arg_limited_op<mpz>(
            [](const auto& a, const auto& b) { return a ^ b; });
    };
    _operations["~"] = [this]() -> bool {
        return one_arg_limited_op<mpz>([](const auto& a) { return ~a; });
    };
    _operations["^"] = [this]() -> bool {
        return two_arg_conv_op(
            [](const auto& a, const auto& b) { return pow(a, b); },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    };
}
