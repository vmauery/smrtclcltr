/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>
#include <functions.hpp>

namespace function
{
namespace sum
{

bool impl(Calculator& calc)
{
    stack_entry e = calc.stack.front();
    mpz* v = std::get_if<mpz>(&e.value());
    if (!v || (*v > 1000000000) || (*v >= static_cast<mpz>(calc.stack.size())))
    {
        return false;
    }
    calc.stack.pop_front();
    size_t count = static_cast<size_t>(*v - 1);
    auto add_fn = std::get<1>(functions::add);
    for (; count > 0; count--)
    {
        if (!add_fn(calc))
        {
            return false;
        }
    }
    return true;
}

auto constexpr help = "\n"
                      "    Usage: ... x sum\n"
                      "\n"
                      "    Returns the sum of the "
                      "bottom x items on the stack: Nx * Nx-1 * ... * N0\n";

} // namespace sum
} // namespace function

namespace functions
{

CalcFunction sum = {function::sum::help, function::sum::impl};

}
