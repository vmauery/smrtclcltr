/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>
#include <functions/common.hpp>

namespace function
{
namespace util
{

mpz comb(const mpz& x, const mpz& y)
{
    return factorial(x) / (factorial(y) * factorial(x - y));
}

mpz perm(const mpz& x, const mpz& y)
{
    return factorial(x) / factorial(x - y);
}
} // namespace util

struct combination : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"comb"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y comb\n"
            "\n"
            "    Return the statistical combination of the bottom "
            "two items on the stack\n"
            "\n"
            "    Use when order doesn't matter in the choice.\n"
            "\n"
            "    No repetition, use: x y comb\n"
            "    / x \\       x!\n"
            "    |    | = --------\n"
            "    \\ y /    y!(x-y)!\n"
            "\n"
            "    With repetition, use: x y swap over + 1 - swap comb\n"
            "                  or use: x y 1 - over + swap 1 - comb\n"
            "\n"
            "    / x+y-1 \\     / x+y-1 \\     (x+y-1)!\n"
            "    |        | =  |        | =  --------\n"
            "    \\   y   /     \\  x-1  /     y!(x-y)!\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        mpz* x = std::get_if<mpz>(&e1.value());
        mpz* y = std::get_if<mpz>(&e0.value());
        if (!x || !y || (*y > *x))
        {
            return false;
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = util::comb(*x, *y);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
    }
};

struct permutation : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"perm"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "    Usage: x y perm\n"
            "\n"
            "    Return the statistical permutation of the bottom "
            "two items on the stack\n"
            "\n"
            "    Use when order matters in the choice.\n"
            "\n"
            "    No repetition, use: x y perm\n"
            "                                        x!\n"
            "    order y things from x available = ------\n"
            "                                      (x-y)!\n"
            "\n"
            "    With repetition, use: x y ^\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        mpz* x = std::get_if<mpz>(&e1.value());
        mpz* y = std::get_if<mpz>(&e0.value());
        if (!x || !y || (*y > *x))
        {
            return false;
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = util::perm(*x, *y);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
    }
};

} // namespace function

register_calc_fn(combination);
register_calc_fn(permutation);
