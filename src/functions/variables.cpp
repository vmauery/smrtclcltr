/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct store : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sto"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x 'y' sto\n"
            "\n"
            "    Store the second to bottom item on the stack (x)\n"
            "    in a variable named y\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 2)
        {
            throw std::invalid_argument("Requires 2 arguments");
        }
        stack_entry& a = calc.stack[1];
        stack_entry& b = calc.stack[0];

        auto& val = a.value();
        auto var = std::get_if<symbolic>(&b.value());
        if (var)
        {
            if (auto n = std::get_if<std::string>(&(*(*var)).left); n)
            {
                calc.variables[*n] = val;
                calc.stack.pop_front();
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument("'y' must be a string for variable name");
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(store);
