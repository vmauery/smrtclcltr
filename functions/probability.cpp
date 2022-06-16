/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
// extern reference
namespace factorial
{
extern mpz factorial(const mpz&);
}

namespace combination
{
auto& factorial = ::function::factorial::factorial;
mpz comb(const mpz& x, const mpz& y)
{
    return factorial(x) / (factorial(y) * factorial(x - y));
}

bool impl(Calculator& calc)
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
    mpz f = comb(*x, *y);
    calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                             calc.config.precision, calc.config.is_signed);
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x y comb\n"
    "\n"
    "    Return the statistical combination of the bottom two items on the "
    "stack\n"
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
    "    \\   y   /     \\  x-1  /     y!(x-y)!\n";

} // namespace combination

namespace permutation
{
auto& factorial = ::function::factorial::factorial;
mpz perm(const mpz& x, const mpz& y)
{
    return factorial(x) / factorial(x - y);
}

bool impl(Calculator& calc)
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
    mpz f = perm(*x, *y);
    calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                             calc.config.precision, calc.config.is_signed);
    return true;
}

auto constexpr help = "    Usage: x y perm\n"
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
                      "    With repetition, use: x y ^\n";

} // namespace permutation
} // namespace function

namespace functions
{

CalcFunction combination = {function::combination::help,
                            function::combination::impl};
CalcFunction permutation = {function::permutation::help,
                            function::permutation::impl};

} // namespace functions
