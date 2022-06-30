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
    // stack_entry a = calc.stack.front();
    calc.stack.pop_front();
    return true;
}

auto constexpr help = "\n"
                      "    Usage: drop\n"
                      "\n"
                      "    Removes the bottom item on the stack\n";

} // namespace drop

namespace drop2
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    calc.stack.pop_front();
    calc.stack.pop_front();
    return true;
}

auto constexpr help = "\n"
                      "    Usage: drop2\n"
                      "\n"
                      "    Removes the bottom two items on the stack\n";

} // namespace drop2

namespace dropn
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry& n = calc.stack.front();
    size_t count = static_cast<size_t>(to_mpz(n.value()));
    if (calc.stack.size() < (count + 1))
    {
        return false;
    }
    calc.stack.pop_front();
    for (size_t i = 0; i < count; i++)
    {
        calc.stack.pop_front();
    }
    return true;
}

auto constexpr help = "\n"
                      "    Usage: x dropn\n"
                      "\n"
                      "    Removes the x bottom items on the stack\n";

} // namespace dropn

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

namespace dup2
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry x = calc.stack[1];
    stack_entry y = calc.stack[0];
    calc.stack.push_front(x);
    calc.stack.push_front(y);
    return true;
}

auto constexpr help = "\n"
                      "    Usage: x y dup2\n"
                      "\n"
                      "    Duplicates the bottom item on the stack (x y x y)\n";

} // namespace dup2

namespace dupn
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry& n = calc.stack.front();
    size_t count = static_cast<size_t>(to_mpz(n.value()));
    if (calc.stack.size() < (count + 1))
    {
        return false;
    }
    // remove N
    calc.stack.pop_front();
    for (size_t i = 0; i < count; i++)
    {
        calc.stack.push_front(calc.stack[count - 1]);
    }
    return true;
}

auto constexpr help = "\n"
                      "    Usage: x0 x1..xn n dupn\n"
                      "\n"
                      "    Duplicates the bottom n items on the stack\n";

} // namespace dupn

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

namespace roll
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack.back();
    calc.stack.pop_back();
    calc.stack.push_front(a);
    return true;
}

auto constexpr help = "\n"
                      "    Usage: roll\n"
                      "\n"
                      "    Rolls the stack up (top item becomes new bottom)\n";

} // namespace roll

namespace rolln
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry& n = calc.stack.front();
    size_t count = static_cast<size_t>(to_mpz(n.value()));
    if (calc.stack.size() < (count + 1))
    {
        return false;
    }
    // remove N
    calc.stack.pop_front();
    for (size_t i = 0; i < count; i++)
    {
        stack_entry a = calc.stack.back();
        calc.stack.pop_back();
        calc.stack.push_front(a);
    }
    return true;
}

auto constexpr help = "\n"
                      "    Usage: x rolln\n"
                      "\n"
                      "    Rolls the stack up x times\n";

} // namespace rolln

namespace rolld
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    calc.stack.pop_front();
    calc.stack.push_back(a);
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: rolld\n"
    "\n"
    "    Rolls the stack down (bottom item becomes new top)\n";

} // namespace rolld

namespace rolldn
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry& n = calc.stack.front();
    size_t count = static_cast<size_t>(to_mpz(n.value()));
    if (calc.stack.size() < (count + 1))
    {
        return false;
    }
    // remove N
    calc.stack.pop_front();
    for (size_t i = 0; i < count; i++)
    {
        stack_entry a = calc.stack.front();
        calc.stack.pop_front();
        calc.stack.push_back(a);
    }
    return true;
}

auto constexpr help = "\n"
                      "    Usage: rolldn\n"
                      "\n"
                      "    Rolls the stack down x times\n";

} // namespace rolldn

namespace pick
{

bool impl(Calculator& calc)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry& n = calc.stack.front();
    size_t count = static_cast<size_t>(to_mpz(n.value()));
    if (calc.stack.size() < (count + 1))
    {
        return false;
    }
    // remove N
    calc.stack.pop_front();
    calc.stack.push_front(calc.stack[count - 1]);
    return true;
}

auto constexpr help = "\n"
                      "    Usage: x pick\n"
                      "\n"
                      "    Returns the item x entries up the stack\n";

} // namespace pick

} // namespace function

namespace functions
{

CalcFunction drop = {function::drop::help, function::drop::impl};
CalcFunction drop2 = {function::drop2::help, function::drop2::impl};
CalcFunction dropn = {function::dropn::help, function::dropn::impl};
CalcFunction dup = {function::dup::help, function::dup::impl};
CalcFunction dup2 = {function::dup2::help, function::dup2::impl};
CalcFunction dupn = {function::dupn::help, function::dupn::impl};
CalcFunction over = {function::over::help, function::over::impl};
CalcFunction swap = {function::swap::help, function::swap::impl};
CalcFunction clear = {function::clear::help, function::clear::impl};
CalcFunction depth = {function::depth::help, function::depth::impl};
CalcFunction roll = {function::roll::help, function::roll::impl};
CalcFunction rolln = {function::rolln::help, function::rolln::impl};
CalcFunction rolld = {function::rolld::help, function::rolld::impl};
CalcFunction rolldn = {function::rolldn::help, function::rolldn::impl};
CalcFunction pick = {function::pick::help, function::pick::impl};

} // namespace functions
