/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace sum
{

bool impl(Calculator& calc)
{
    stack_entry e = calc.stack.front();
    mpz* v = std::get_if<mpz>(&e.value);
    if (!v || (*v > 1000000000) || (*v >= calc.stack.size()))
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

auto constexpr help = "returns the sum of the first X items on the stack";

} // namespace sum
} // namespace function

namespace functions
{

CalcFunction sum = {function::sum::help, function::sum::impl};

}
