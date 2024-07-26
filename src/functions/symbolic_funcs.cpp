/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct Eval : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"eval"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x eval\n"
            "\n"
            "    Evaluates the sybolic item at the bottom on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 1)
        {
            throw std::invalid_argument("Requires 1 argument");
        }
        stack_entry& a = calc.stack.front();

        if (auto s = std::get_if<symbolic>(&a.value()); s)
        {
            auto v = *s;
            calc.stack.pop_front();
            v.eval(calc);
        }
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

register_calc_fn(Eval);
