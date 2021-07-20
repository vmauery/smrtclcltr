/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace drop
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    calc.stack.pop_front();
    return true;
}

auto constexpr help = "drops one item from the stack";

} // namespace drop

namespace dup
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    calc.stack.push_front(a);
    return true;
}

auto constexpr help = "duplicates first item on the stack";

} // namespace dup

namespace swap
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    calc.stack.pop_front();
    stack_entry b = calc.stack.front();
    calc.stack.pop_front();
    calc.stack.push_front(std::move(a));
    calc.stack.push_front(std::move(b));
    return true;
}

auto constexpr help = "swaps position of first two items on the stack";

} // namespace swap

namespace clear
{

bool impl(Calculator& calc)
{
    calc.stack.clear();
    return true;
}

auto constexpr help = "clear the stack and settings";

} // namespace clear

namespace depth
{

bool impl(Calculator& calc)
{
    stack_entry e(mpz(calc.stack.size()), calc.config.base,
                  calc.config.fixed_bits, calc.config.precision,
                  calc.config.is_signed);
    calc.stack.push_front(std::move(e));
    return true;
}

auto constexpr help = "returns the number of items on the stack";

} // namespace depth

} // namespace function

namespace functions
{

CalcFunction drop = {function::drop::help, function::drop::impl};
CalcFunction dup = {function::dup::help, function::dup::impl};
CalcFunction swap = {function::swap::help, function::swap::impl};
CalcFunction clear = {function::clear::help, function::clear::impl};
CalcFunction depth = {function::depth::help, function::depth::impl};

} // namespace functions