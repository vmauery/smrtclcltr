/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <parser.hpp>
#include <program.hpp>

namespace smrty
{

// break programs up into segments of conditional and non-conditional
// statements that can be executed 'in-order' with the conditional
// stuff having some logic to make sure stuff is skipped or looped
//
// [ non-condition statement ]
// [ if / elif / else statement ]
//   [ conditional statement ]
//     [ non-conditional body statement ]
//   [ conditional statement ]
//     [ non-conditional body statement ]
//   [ conditional statement (empty for else) ]
//     [ non-conditional body statement ]
// [ for loop statement ]
//   [ conditional statement ]
//     [ non-conditional body statement ]

// program syntax:
//
// if [ statements ] then
//     [ statements ]
// [elif [ statements ] then
//     [ statements ] ]
// [ else
//     [ statements ] ]
// endif
//
// while [ statements ] do
//     [ statements ]
// loop
//
// foreach <var> in (list|<start> <stop> range) do
//     [ statements ]
// next
//
// for <var> do
//     [ statements ]
// next

program::program() : body(), next(body.begin()), standalone(false)
{
}

program::program(const instructions& i) :
    body(i), next(body.begin()), standalone(false)
{
}

program::program(const program& o) :
    body(o.body), next(body.begin()), standalone(o.standalone)
{
}

program::program(program&& o) :
    body(std::move(o.body)), next(body.begin()), standalone(o.standalone)
{
}

program& program::operator=(const program& o)
{
    body = o.body;
    next = body.begin();
    standalone = o.standalone;
    return *this;
}

program& program::operator=(program&& o)
{
    body = std::move(o.body);
    next = body.begin();
    standalone = o.standalone;
    return *this;
}

program::~program()
{
}

const simple_instruction& program::next_item(execution_flags& flags)
{
    if (next != body.end())
    {
        if (auto s = std::get_if<simple_instruction>(&(*next)); s)
        {
            next++;
            return *s;
        }
        else if (auto s = std::get_if<if_elif_statement>(&(*next)); s)
        {
            if (const auto& itm = s->next_item(flags); !is_noop(itm))
            {
                return itm;
            }
            next++;
            // recurse
            return next_item(flags);
        }
        //
        // TODO: add other control statements here (loops, etc.)
        //
    }
    next = body.begin();
    return noop;
}

bool program::execute(const Executor& executor, execution_flags& flags)
{
    // reset program counter to beginning
    next = body.begin();

    lg::debug("program::execute()\n");
    while (1)
    {
        const auto& itm = next_item(flags);
        lg::debug("program: next_item = {}\n", itm);
        if (is_noop(itm))
        {
            lg::debug("program::execute() - end of program\n");
            break;
        }
        if (!executor(itm, flags))
        {
            lg::info("execution halted at '{}'\n", itm);
            return false;
        };
    }
    return true;
}

void program::quit()
{
    next = body.end();
}

bool program::done()
{
    return next != body.end();
}
simple_program::simple_program() : body(), next(body.begin())
{
}

simple_program::simple_program(const simple_instructions& si) :
    body(si), next(body.begin())
{
}

simple_program::simple_program(const simple_program& o) :
    body(o.body), next(body.begin())
{
}

simple_program::~simple_program()
{
}

const simple_instruction& simple_program::next_item(execution_flags&)
{
    if (next != body.end())
    {
        return *next++;
    }
    next = body.begin();
    return noop;
}

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

} // namespace smrty
