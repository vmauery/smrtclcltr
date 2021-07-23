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

auto constexpr help = "\n"
                      "    Usage: x drop\n"
                      "\n"
                      "    Removes the bottom item on the stack (x)\n";

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

auto constexpr help = "\n"
                      "    Usage: x dup\n"
                      "\n"
                      "    Duplicates the bottom item on the stack (x x)\n";

} // namespace dup

namespace over
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack[1];
    calc.stack.push_front(a);
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x over\n"
    "\n"
    "    Pushes the second to bottom item onto the stack as the bottom\n";

} // namespace over

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

auto constexpr help = "\n"
                      "    Usage: x y swap\n"
                      "\n"
                      "    Swaps the bottom two items on the stack (y x)\n";

} // namespace swap

namespace clear
{

bool impl(Calculator& calc)
{
    calc.stack.clear();
    return true;
}

auto constexpr help = "\n"
                      "    Usage: clear\n"
                      "\n"
                      "    Removes all items from the stack\n";

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

auto constexpr help = "\n"
                      "    Usage: depth\n"
                      "\n"
                      "    Returns the number of items on the stack\n";

} // namespace depth

} // namespace function

namespace functions
{

CalcFunction drop = {function::drop::help, function::drop::impl};
CalcFunction dup = {function::dup::help, function::dup::impl};
CalcFunction over = {function::over::help, function::over::impl};
CalcFunction swap = {function::swap::help, function::swap::impl};
CalcFunction clear = {function::clear::help, function::clear::impl};
CalcFunction depth = {function::depth::help, function::depth::impl};

} // namespace functions
