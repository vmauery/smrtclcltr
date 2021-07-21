/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace factorial
{

bool impl(Calculator& calc)
{

    stack_entry e = calc.stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
    if (!v || (*v > 100000))
    {
        return false;
    }
    calc.stack.pop_front();
    mpz f = *v;
    mpz n = *v;
    while (--n > 1)
    {
        f *= n;
    }
    calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                             calc.config.precision, calc.config.is_signed);
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x !\n"
    "\n"
    "    Returns the factorial of the bottom item on the stack (x!)\n";

} // namespace factorial
} // namespace function

namespace functions
{

CalcFunction factorial = {function::factorial::help, function::factorial::impl};

}
