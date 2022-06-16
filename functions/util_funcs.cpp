/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace split
{

bool impl(Calculator& calc)
{
    stack_entry a = calc.stack.front();
    auto c = std::get_if<mpc>(&a.value());
    if (c)
    {
        calc.stack.pop_front();
        stack_entry c1{a};
        c1.value(c->real());
        calc.stack.push_front(c1);
        stack_entry c2{a};
        c2.value(c->imag());
        calc.stack.push_front(c2);
        return true;
    }
    auto q = std::get_if<mpq>(&a.value());
    if (q)
    {
        calc.stack.pop_front();
        stack_entry q1{a};
        q1.value(helper::numerator(*q));
        calc.stack.push_front(q1);
        stack_entry q2{a};
        q2.value(helper::denominator(*q));
        calc.stack.push_front(q2);
        return true;
    }
    /*
    auto v = std::get_if<vector>(&a.value());
    if (v)
    {
        calc.stack.pop_front();
        for (const auto& vv : v)
        {
            stack_entry v1{a};
            v1.value ( vv.num);
            calc.stack.push_front(v1);
        }
        return true;
    }
    */
    return false;
}

auto constexpr help =
    "\n"
    "    Usage: x split\n"
    "\n"
    "    split composite item and place parts on the stack\n"
    "    No action taken on non-composite items (int, float, etc.)\n"
    "    complex -> real imag; vector -> X Y Z ...; matrix -> vector(s)";

} // namespace split
} // namespace function

namespace functions
{

CalcFunction split = {function::split::help, function::split::impl};

}
