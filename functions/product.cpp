/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>
#include <functions.hpp>

namespace function
{
namespace product
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
    auto mult = std::get<1>(functions::multiply);
    for (; count > 0; count--)
    {
        if (!mult(calc))
        {
            return false;
        }
    }
    return true;
}

auto constexpr help = "returns the product of the first X items on the stack";

} // namespace product
} // namespace function

namespace functions
{

CalcFunction product = {function::product::help, function::product::impl};

}
