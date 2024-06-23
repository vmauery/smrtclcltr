/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
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
        // require two items on the stack; provided by num_args
        stack_entry a = calc.stack[1];
        stack_entry b = calc.stack[0];

        units::convert(a.value(), a.unit(), b.unit());
        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.push_front(a);
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};
} // namespace function
} // namespace smrty

register_calc_fn(uconv);
