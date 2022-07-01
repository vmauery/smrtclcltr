/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{

struct bitwise_and : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"&"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y &\n"
            "\n"
            "    Returns the bitwise AND of the bottom two items on "
            "the stack (x & y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc, [](const auto& a, const auto& b) { return a & b; });
    }
};

struct bitwise_or : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"|"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y |\n"
            "\n"
            "    Returns the bitwise OR of the bottom "
            "two items on the stack (x & y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc, [](const auto& a, const auto& b) { return a | b; });
    }
};

struct bitwise_xor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"xor"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y xor\n"
            "\n"
            "    Returns the bitwise XOR of the bottom two items on "
            "the stack (x xor y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc, [](const auto& a, const auto& b) { return a % b; });
    }
};

struct bitwise_inv : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"~"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x ~\n"
            "\n"
            "    Returns the bitwise negation of the bottom "
            "item on the stack (~x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<mpz>(calc, [](const auto& a) { return ~a; });
    }
};

} // namespace function

register_calc_fn(bitwise_and);
register_calc_fn(bitwise_or);
register_calc_fn(bitwise_xor);
register_calc_fn(bitwise_inv);
