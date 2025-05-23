/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct TEMPLATE : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"TEMPLATE"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x TEMPLATE\n"
            "\n"
            "    Returns the TEMPLATE of the bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        (void)calc.stack.front();
        // return false to inhibit stack printing upon return
        return true;
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(TEMPLATE);
