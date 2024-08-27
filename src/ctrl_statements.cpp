/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <ctrl_statements.hpp>
#include <numeric.hpp>
#include <parser.hpp>
#include <parser_parts.hpp>

namespace smrty
{

if_elif_statement::if_elif_statement() :
    branches(), current_branch(branches.begin())
{
}

if_elif_statement::if_elif_statement(const if_elif_statement& o) :
    branches(o.branches), current_branch(branches.begin())
{
}

if_elif_statement::~if_elif_statement()
{
}

void if_elif_statement::set_cond(const std::vector<simple_instruction>& s)
{
    branches.emplace_back(std::make_tuple(true, simple_program{s}, program{}));
    current_branch = branches.begin();
}

void if_elif_statement::set_body(const std::vector<instruction>& s)
{
    std::get<program>(branches.back()).body = s;
    current_branch = branches.begin();
}

void if_elif_statement::set_else(const std::vector<instruction>& s)
{
    branches.emplace_back(std::make_tuple(true, simple_program{}, program{s}));
    current_branch = branches.begin();
}

const simple_instruction&
    if_elif_statement::branch_next_item(execution_flags& flags)
{
    auto& [test_phase, condition, body] = *current_branch;
    /*
    if (lg::debug_level == lg::level::debug)
    {
        // are we on the first branch
        if (condition.next == condition.body.begin())
        {
            if (current_branch == branches.begin())
            {
                if (condition.next == condition.body.begin())
                {
                    lg::debug("Starting [IF] condition execution\n");
                }
            }
            else if (condition.size() != 0)
            {
                lg::debug("Starting [ELIF] condition execution\n");
            }
            else
            {
                lg::debug("Starting [ELSE] body execution\n");
            }
        }
    }
    */
    if (condition.body.size() && test_phase)
    {
        lg::debug("Executing next item in condition\n");
        if (const auto& itm = condition.next_item(flags); !is_noop(itm))
        {
            return itm;
        }
        lg::debug("End of condition; flags: z({}) c({}) o({}) s({})\n",
                  flags.zero, flags.carry, flags.overflow, flags.sign);
        // drop the item used for test
        auto& calc = Calculator::get();
        if (calc.stack.size())
        {
            calc.stack.pop_front();
        }
        if (flags.zero)
        {
            return noop;
        }
        test_phase = false;
    }
    if (auto& itm = body.next_item(flags); !is_noop(itm))
    {
        return itm;
    }
    test_phase = true;
    return noop;
}

const simple_instruction& if_elif_statement::next_item(execution_flags& flags)
{
    while (current_branch != branches.end())
    {
        bool continue_next = std::get<bool>(*current_branch);
        if (const auto& itm = branch_next_item(flags); !is_noop(itm))
        {
            return itm;
        }
        // the condition was true and we are finished
        // executing the body. Reset conditional.
        if (!continue_next)
        {
            current_branch = branches.begin();
            return noop;
        }
        current_branch++;
    }
    current_branch = branches.begin();
    return noop;
}

// user provides while loop conditional
while_statement::while_statement() : cond(), body(), in_cond(true)
{
}

while_statement::while_statement(const while_statement& o) :
    cond(o.cond), body(o.body), in_cond(true)
{
}
while_statement& while_statement::operator=(const while_statement& o)
{
    cond = o.cond;
    body = o.body;
    in_cond = true;
    return *this;
}

void while_statement::set_cond(const std::vector<simple_instruction>& s)
{
    cond = simple_program{s};
    in_cond = true;
}

void while_statement::set_body(const std::vector<instruction>& s)
{
    body = program{s};
}

const simple_instruction& while_statement::next_item(execution_flags& flags)
{
    do
    {
        if (in_cond)
        {
            // keep processing setup instructions until last instruction
            if (auto& itm = cond.next_item(flags); !is_noop(itm))
            {
                return itm;
            }
            // pop the final item that we just measured
            auto& calc = Calculator::get();
            if (calc.stack.size())
            {
                calc.stack.pop_front();
            }
            if (flags.zero)
            {
                break;
            }
            in_cond = false;
        }

        // loop body
        if (auto& itm = body.next_item(flags); !is_noop(itm))
        {
            // check for break/continue
            if (auto i = std::get_if<keyword>(&itm); i)
            {
                if (i->word == "break")
                {
                    body.quit();
                    break;
                }
                if (i->word == "continue")
                {
                    body.quit();
                    in_cond = true;
                    continue;
                }
            }
            return itm;
        }
        in_cond = true;
    } while (1);
    return noop;
}

// generate loop conditional based on var name and list of items to iterate
for_statement::for_statement() :
    setup(), body(), var_name(), values(), val_iter(), in_setup(true)
{
}

for_statement::for_statement(const for_statement& o) :
    setup(o.setup), body(o.body), var_name(o.var_name), values(), val_iter(),
    in_setup(true)
{
}

for_statement& for_statement::operator=(const for_statement& o)
{
    setup = o.setup;
    body = o.body;
    var_name = o.var_name;
    values = o.values;
    val_iter = values.begin();
    in_setup = true;
    return *this;
}

void for_statement::set_var(const symbolic& name)
{
    var_name = std::get<std::string>((*name).left);
}

void for_statement::set_setup(const std::vector<simple_instruction>& s)
{
    setup = simple_program{s};
    in_setup = true;
}

void for_statement::set_body(const std::vector<instruction>& s)
{
    body = program{s};
}

const simple_instruction& for_statement::next_item(execution_flags& flags)
{
    auto& calc = Calculator::get();
    if (in_setup)
    {
        lg::debug("setup: {}; next: {}\n", setup, *(setup.next));
        // keep processing setup instructions until last instruction
        // test stack item to ensure it is a list
        if (auto& itm = setup.next_item(flags); !is_noop(itm))
        {
            return itm;
        }
        if (calc.stack.size() < 1)
        {
            throw std::invalid_argument(
                "FOR loop setup resulted in an empty stack");
        }
        auto& e = calc.stack.front();
        auto& v = e.value();
        auto l = std::get_if<list>(&v);
        if (!l)
        {
            throw std::invalid_argument(
                "FOR loop setup did not evaluate to a list");
        }
        values = *l;
        val_iter = values.begin();
        calc.stack.pop_front();
        if (val_iter == values.end())
        {
            // end of loop
            return noop;
        }
        lg::debug("values is {}; next {} is {}\n", values, var_name, *val_iter);
        calc.set_var(var_name, variant_cast(*val_iter));
        in_setup = false;
    }

    auto for_loop_advance = [&calc, this]() {
        val_iter++;
        if (val_iter == values.end())
        {
            // end of loop
            in_setup = true;
            return false;
        }
        calc.set_var(var_name, variant_cast(*val_iter));
        return true;
    };
    do
    {
        // loop body
        if (auto& itm = body.next_item(flags); !is_noop(itm))
        {
            // check for break/continue
            if (auto i = std::get_if<keyword>(&itm); i)
            {
                if (i->word == "break")
                {
                    body.quit();
                    break;
                }
                if (i->word == "continue")
                {
                    body.quit();
                    in_setup = true;
                    continue;
                }
            }
            return itm;
        }
    } while (for_loop_advance());
    return noop;
}

} // namespace smrty
