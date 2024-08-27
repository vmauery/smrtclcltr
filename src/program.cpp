/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <numeric.hpp>
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
//     [ statements ; continue ; break ]
// loop
//
// for <var> in {list|x y range|instructions that emit a list} do
//     [ statements ; continue ; break ]
// next
//

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
        // control statements
        else if (auto s = std::get_if<statement::ptr>(&(*next)); s)
        {
            lg::debug("ctrl-statement: {}\n", *s);
            if (const auto& itm = (*s)->next_item(flags); !is_noop(itm))
            {
                return itm;
            }
            next++;
            // recurse
            return next_item(flags);
        }
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

simple_program::simple_program(const simple_instructions& si) : body(si)
{
    next = body.begin();
}

simple_program::simple_program(const simple_program& o) :
    body(o.body), next(body.begin())
{
}

simple_program& simple_program::operator=(const simple_program& o)
{
    body = o.body;
    next = body.begin();
    return *this;
}

simple_program& simple_program::operator=(simple_program&& o)
{
    body = std::move(o.body);
    next = body.begin();
    return *this;
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

} // namespace smrty
