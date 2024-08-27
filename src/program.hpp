/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <numeric>
#include <statement.hpp>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

using Executor =
    std::function<bool(const simple_instruction&, execution_flags&)>;

/* a list of possibly conditional/compound items to execute */
struct program : public statement
{
    program();
    program(const program&);
    program(const instructions&);
    program(program&&);
    program& operator=(const program&);
    program& operator=(program&&);
    virtual ~program();

    virtual const simple_instruction& next_item(execution_flags&) final;
    bool execute(const Executor&, execution_flags&);
    void quit();
    bool done();

    instructions body;
    instructions::iterator next;
    bool standalone;
};

/* a list of non-conditional items to execute */
struct simple_program : public statement
{
    simple_program();
    simple_program(const simple_instructions&);
    simple_program(const simple_program&);
    simple_program& operator=(const simple_program&);
    simple_program& operator=(simple_program&&);
    virtual ~simple_program();

    virtual const simple_instruction& next_item(execution_flags&) final;

    simple_instructions body;
    simple_instructions::iterator next;
};

static constexpr std::monostate noop_value{};
static constexpr simple_instruction noop{noop_value};

static inline bool is_noop(const simple_instruction& itm)
{
    return (std::get_if<std::monostate>(&itm) != nullptr);
}

} // namespace smrty

template <>
struct std::formatter<smrty::program>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::program& pgm,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        if (pgm.standalone)
        {
            *out++ = '$';
            *out++ = '(';
        }
        std::format_to(out, "{: }", pgm.body);
        if (pgm.standalone)
        {
            *out++ = ')';
        }
        return out;
    }
};

template <>
struct std::formatter<smrty::simple_program>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::simple_program& pgm,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{: }", pgm.body);
        return out;
    }
};
