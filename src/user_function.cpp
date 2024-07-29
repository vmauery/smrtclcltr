/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <user_function.hpp>

namespace smrty
{

UserFunction::UserFunction(const std::string& name, program&& prog) :
    _name(name), function(std::move(prog))
{
    _help = std::format("\n"
                        "    User defined function: '{}'\n"
                        "\n"
                        "    {}\n",
                        name, function);
}
const std::string& UserFunction::name() const
{
    return _name;
}
const std::string& UserFunction::help() const
{
    return _help;
}
bool UserFunction::op(Calculator& calc) const
{
    program fn{function};
    return fn.execute(
        [&calc](const simple_instruction& itm, execution_flags& eflags) {
            bool retval = calc.run_one(itm);
            eflags = calc.flags;
            return retval;
        },
        calc.flags);
}
// if number of args is known, each function should set that
// if number of args is < 0, it is variable, with |n| as the min
int UserFunction::num_args() const
{
    return 0;
}
int UserFunction::num_resp() const
{
    return 0;
}
symbolic_op UserFunction::symbolic_usage() const
{
    return symbolic_op::none;
}

CalcFunction::ptr UserFunction::create(const std::string& name, program&& prog)
{
    return std::make_shared<UserFunction>(name, std::move(prog));
}

} // namespace smrty
