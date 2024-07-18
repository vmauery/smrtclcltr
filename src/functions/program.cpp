/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct execute : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"execute"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: $(...) execute\n"
            "\n"
            "    executes the program on the bottom on the stack $(...)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // one arg using num_args
        stack_entry& e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            throw units_prohibited();
        }
        auto p = std::get_if<program>(&e.value());
        if (!p)
        {
            throw std::invalid_argument("argument is not a program");
        }
        program prog{*p};
        calc.stack.pop_front();

        return prog.execute(
            [&calc](const simple_instruction& itm, execution_flags& eflags) {
                bool retval = calc.run_one(itm);
                eflags = calc.flags;
                return retval;
            },
            calc.flags);
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return -1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(execute);
