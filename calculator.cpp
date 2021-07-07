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
        return std::get<1>(op->second)();
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
        auto iv = static_cast<int>(*v);
        switch (iv)
        {
            case 2:
            case 8:
            case 10:
            case 16:
                _stack.pop_front();
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
    _operations["debug"] = {"enable debug",
                            [this]() -> bool { return debug(); }};
    _operations["clear"] = {"clear the stack and settings", [this]() -> bool {
                                _stack.clear();
                                return true;
                            }};
    _operations["depth"] = {
        "returns the number of items on the stack", [this]() -> bool {
            stack_entry e(mpz(_stack.size()), _base, _fixed_bits, _precision,
                          _is_signed);
            _stack.push_front(std::move(e));
            return true;
        }};
    _operations["base"] = {"sets the numeric base",
                           [this]() -> bool { return base(); }};
    _operations["fixed_bits"] = {"sets the number of fixed bits",
                                 [this]() -> bool { return fixed_bits(); }};
    _operations["precision"] = {"sets the precision",
                                [this]() -> bool { return precision(); }};
    _operations["unsigned"] = {"sets unsigned mode",
                               [this]() -> bool { return unsigned_mode(); }};
    _operations["rad"] = {"sets radians angle mode", [this]() -> bool {
                              return angle_mode(e_angle_mode::rad);
                          }};
    _operations["deg"] = {"sets degrees angle mode", [this]() -> bool {
                              return angle_mode(e_angle_mode::deg);
                          }};
    _operations["grad"] = {"sets gradians angle mode", [this]() -> bool {
                               return angle_mode(e_angle_mode::grad);
                           }};
    _operations["drop"] = {"drops one item from the stack", [this]() -> bool {
                               if (_stack.size() < 1)
                               {
                                   return false;
                               }
                               stack_entry a = _stack.front();
                               _stack.pop_front();
                               return true;
                           }};
    _operations["dup"] = {"duplicates first item on the stack",
                          [this]() -> bool {
                              if (_stack.size() < 1)
                              {
                                  return false;
                              }
                              stack_entry a = _stack.front();
                              _stack.push_front(a);
                              return true;
                          }};
    _operations["swap"] = {"swaps position of first two items on the stack",
                           [this]() -> bool {
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
                           }};
    _operations["+"] = {"addition for first two items on stack",
                        [this]() -> bool {
                            return two_arg_op([](const auto& a, const auto& b) {
                                return a + b;
                            });
                        }};
    _operations["-"] = {"subtraction for first two items on stack",
                        [this]() -> bool {
                            return two_arg_op([](const auto& a, const auto& b) {
                                return a - b;
                            });
                        }};
    _operations["*"] = {"multiplication for first two items on stack",
                        [this]() -> bool {
                            return two_arg_op([](const auto& a, const auto& b) {
                                return a * b;
                            });
                        }};
    _operations["/"] = {
        "division for first two items on stack", [this]() -> bool {
            return two_arg_conv_op(
                [](const auto& a, const auto& b) { return a / b; },
                std::tuple<mpz>{}, std::tuple<mpq>{},
                std::tuple<mpq, mpf, mpc>{});
        }};
    _operations["range"] = {
        "adds to the stack a range of numbers [Y..X)", [this]() -> bool {
            stack_entry xe = _stack.front();
            _stack.pop_front();
            stack_entry ye = _stack.front();
            mpz* x = std::get_if<mpz>(&xe.value);
            mpz* y = std::get_if<mpz>(&ye.value);
            if (!x || !y)
            {
                _stack.push_front(xe);
                return false;
            }
            _stack.pop_front();
            mpz step, count;
            if (*x > *y)
            {
                count = *x - *y;
                step = 1;
            }
            else
            {
                count = *y - *x;
                step = -1;
            }
            mpz v = *y;
            for (; count > 0; count--)
            {
                stack_entry ve;
                ve.value = v;
                _stack.emplace_front(v, _base, _fixed_bits, _precision,
                                     _is_signed);
                v += step;
            }
            return true;
        }};
    _operations["sum"] = {
        "returns the sum of the first X items on the stack", [this]() -> bool {
            stack_entry e = _stack.front();
            mpz* v = std::get_if<mpz>(&e.value);
            if (!v || (*v > 1000000000) || (*v >= _stack.size()))
            {
                return false;
            }
            _stack.pop_front();
            size_t count = static_cast<size_t>(*v - 1);
            auto add = std::get<1>(_operations["+"]);
            for (; count > 0; count--)
            {
                if (!add())
                {
                    return false;
                }
            }
            return true;
        }};
    _operations["prod"] = {
        "returns the product of the first X items on the stack",
        [this]() -> bool {
            stack_entry e = _stack.front();
            mpz* v = std::get_if<mpz>(&e.value);
            if (!v || (*v > 1000000000) || (*v >= _stack.size()))
            {
                return false;
            }
            _stack.pop_front();
            size_t count = static_cast<size_t>(*v - 1);
            auto mult = std::get<1>(_operations["*"]);
            for (; count > 0; count--)
            {
                if (!mult())
                {
                    return false;
                }
            }
            return true;
        }};
    _operations["sqrt"] = {
        "square root of the first item on the stack", [this]() -> bool {
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
        }};
    _operations["sin"] = {
        "returns Sine of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op(a,
                                          [](const auto& a) { return sin(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["cos"] = {
        "returns Cosine of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op(a,
                                          [](const auto& a) { return cos(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["tan"] = {
        "returns Tangent of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op(a,
                                          [](const auto& a) { return tan(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["asin"] = {
        "returns Arcsine of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op_inv(
                        a, [](const auto& a) { return asin(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["acos"] = {
        "returns Arccosine of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op_inv(
                        a, [](const auto& a) { return acos(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["atan"] = {
        "returns Arctangent of the first item on the stack", [this]() -> bool {
            return one_arg_conv_op(
                [this](const auto& a) -> numeric {
                    return scaled_trig_op_inv(
                        a, [](const auto& a) { return atan(a); });
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["log"] = {
        "base 10 logarithm of X", [this]() {
            return one_arg_conv_op(
                [](const auto& a) -> numeric {
                    mpf log10 = log(mpf{10});
                    if constexpr (std::is_same<decltype(a), const mpc&>::value)
                    {
                        return log(a) / log10;
                    }
                    else
                    {
                        if (a > decltype(a)(0))
                        {
                            return log(mpf{a}) / log10;
                        }
                        else
                        {
                            return log(mpc{a}) / log10;
                        }
                    }
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["ln"] = {
        "base e logarithm of X", [this]() {
            return one_arg_conv_op(
                [](const auto& a) -> numeric {
                    if constexpr (std::is_same<decltype(a), const mpc&>::value)
                    {
                        return log(a);
                    }
                    else
                    {
                        if (a > decltype(a)(0))
                        {
                            return log(mpf{a});
                        }
                        else
                        {
                            return log(mpc{a});
                        }
                    }
                },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};
    _operations["!"] = {
        "factorial of first item on the stack", [this]() -> bool {
            stack_entry e = _stack.front();
            mpz* v = std::get_if<mpz>(&e.value);
            if (!v || (*v > 100000))
            {
                return false;
            }
            _stack.pop_front();
            mpz f = *v;
            mpz n = *v;
            while (--n > 1)
            {
                f *= n;
            }
            _stack.emplace_front(f, _base, _fixed_bits, _precision, _is_signed);
            return true;
        }};
    _operations["e"] = {"pushes constant e onto the stack", [this]() -> bool {
                            stack_entry e(boost::math::constants::e<mpf>(),
                                          _base, _fixed_bits, _precision,
                                          _is_signed);
                            _stack.push_front(std::move(e));
                            return true;
                        }};
    _operations["pi"] = {"pushes constant Pi onto the stack", [this]() -> bool {
                             stack_entry e(boost::math::constants::pi<mpf>(),
                                           _base, _fixed_bits, _precision,
                                           _is_signed);
                             _stack.push_front(std::move(e));
                             return true;
                         }};
    _operations["neg"] = {
        "negates the first item on the stack", [this]() -> bool {
            return one_arg_op([](const auto& a) { return -a; });
        }};
    _operations["%"] = {
        "modular division for the first two items on the stack",
        [this]() -> bool {
            return two_arg_limited_op<mpz>(
                [](const auto& a, const auto& b) { return a % b; });
        }};
    _operations["&"] = {
        "bitwise AND of the first two items on the stack", [this]() -> bool {
            return two_arg_limited_op<mpz>(
                [](const auto& a, const auto& b) { return a & b; });
        }};
    _operations["|"] = {
        "bitwise OR of the first two items on the stack", [this]() -> bool {
            return two_arg_limited_op<mpz>(
                [](const auto& a, const auto& b) { return a | b; });
        }};
    _operations["xor"] = {
        "bitwise xor of the first two items on the stack", [this]() -> bool {
            return two_arg_limited_op<mpz>(
                [](const auto& a, const auto& b) { return a ^ b; });
        }};
    _operations["~"] = {
        "bitwise inversion of the first item on the stack", [this]() -> bool {
            return one_arg_limited_op<mpz>([](const auto& a) { return ~a; });
        }};
    _operations["^"] = {
        "exponentiation for the first two items on the stack",
        [this]() -> bool {
            return two_arg_conv_op(
                [](const auto& a, const auto& b) { return pow(a, b); },
                std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
                std::tuple<mpf, mpc>{});
        }};

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
