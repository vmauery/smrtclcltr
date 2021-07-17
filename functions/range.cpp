/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace range
{

bool impl(Calculator& calc)
{
    stack_entry xe = calc.stack.front();
    calc.stack.pop_front();
    stack_entry ye = calc.stack.front();
    mpz* x = std::get_if<mpz>(&xe.value);
    mpz* y = std::get_if<mpz>(&ye.value);
    if (!x || !y)
    {
        calc.stack.push_front(xe);
        return false;
    }
    calc.stack.pop_front();
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
        calc.stack.emplace_front(v, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        v += step;
    }
    return true;
}

auto constexpr help = "adds to the stack a range of numbers [Y..X)";

} // namespace range
} // namespace function

namespace functions
{

CalcFunction range = {function::range::help, function::range::impl};

}
