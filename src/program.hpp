/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <parser_parts.hpp>
#include <regex>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

struct execution_flags
{
    bool zero;
    bool carry;
    bool overflow;
    bool sign;
};

struct if_elif_statement;
struct program;

using simple_instruction = std::variant<bool, number_parts, compound_parts,
                                        time_parts, function_parts, program>;
using simple_instructions = std::vector<simple_instruction>;

// TODO: add control statement types to instruction variant
using instruction = std::variant<simple_instruction, if_elif_statement>;
using instructions = std::vector<instruction>;

struct statement
{
    virtual ~statement() = default;
    virtual const simple_instruction& next_item(execution_flags&) = 0;
};

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
    virtual ~simple_program();

    virtual const simple_instruction& next_item(execution_flags&) final;

    simple_instructions body;
    simple_instructions::iterator next;
};

struct if_elif_statement : public statement
{
    if_elif_statement();
    if_elif_statement(const if_elif_statement& o);
    virtual ~if_elif_statement();

    const simple_instruction& branch_next_item(execution_flags&);
    virtual const simple_instruction& next_item(execution_flags&) final;

    // possible parser helpers
    // if_elif_statement(condition)
    // set_statement
    // add_else

    using condition = std::tuple<bool, simple_program, program>;
    std::vector<condition> branches;
    std::vector<condition>::iterator current_branch;
};

static constexpr bool noop_value{false};
static constexpr simple_instruction noop{noop_value};
static inline bool is_noop(const simple_instruction& itm)
{
    if (auto v = std::get_if<bool>(&itm); v)
    {
        return *v == noop_value;
    }
    return false;
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

template <>
struct std::formatter<smrty::if_elif_statement>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::if_elif_statement& ifelif,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        for (size_t i = 0; i < ifelif.branches.size(); i++)
        {
            const auto& br = ifelif.branches[i];
            auto& cond = std::get<smrty::simple_program>(br);
            auto& body = std::get<smrty::program>(br);
            if (cond.body.size())
            {
                constexpr std::string_view ifstr{"if "};
                constexpr std::string_view elifstr{" elif "};
                std::string_view cond_type = (i == 0) ? ifstr : elifstr;
                out = std::format_to(out, "{} {} then ", cond_type, cond);
            }
            else
            {
                constexpr std::string_view elsestr{" else "};
                out = std::copy(elsestr.begin(), elsestr.end(), out);
            }
            out = std::format_to(out, "{}", body);
            if (i == (ifelif.branches.size() - 1))
            {
                constexpr std::string_view endifstr{" endif"};
                out = std::copy(endifstr.begin(), endifstr.end(), out);
            }
        }
        return out;
    }
};
