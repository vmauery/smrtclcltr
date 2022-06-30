/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{

struct uconv : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"uconv"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y uconv\n"
            "\n"
            "    Convert x (with units) to be in terms "
            "of units of y (value ignored)"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // require two items on the stack
        //
        if (calc.stack.size() < 2)
        {
            return false;
        }
        stack_entry a = calc.stack[1];
        stack_entry b = calc.stack[0];

        units::convert(a.value(), a.unit(), b.unit());
        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.push_front(a);
        return false;
    }
};
} // namespace function

register_calc_fn(uconv);
