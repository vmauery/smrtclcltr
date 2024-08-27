/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <numeric.hpp>
#include <program.hpp>
#include <statement.hpp>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

struct if_elif_statement : public statement
{
    if_elif_statement();
    if_elif_statement(const if_elif_statement& o);
    virtual ~if_elif_statement();

    void set_cond(const std::vector<simple_instruction>&);
    void set_body(const std::vector<instruction>&);
    void set_else(const std::vector<instruction>&);

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

struct while_statement : public statement
{
    while_statement();
    while_statement(const while_statement& o);
    while_statement& operator=(const while_statement& o);

    void set_cond(const std::vector<simple_instruction>&);
    void set_body(const std::vector<instruction>&);

    virtual const simple_instruction& next_item(execution_flags&);

    simple_program cond;
    program body;
    bool in_cond;
};

struct for_statement : public statement
{
    for_statement();
    for_statement(const for_statement& o);
    for_statement& operator=(const for_statement& o);

    void set_var(const symbolic&);
    void set_setup(const std::vector<simple_instruction>&);
    void set_body(const std::vector<instruction>&);

    virtual const simple_instruction& next_item(execution_flags&);

    // setup is user-provided mechanism for creating the list of items
    simple_program setup;
    program body;
    std::string var_name;
    list values;
    list::iterator val_iter;
    bool in_setup;
};

} // namespace smrty

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

template <>
struct std::formatter<smrty::while_statement>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::while_statement& s,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "while {} do {} done", s.cond, s.body);
        return out;
    }
};

template <>
struct std::formatter<smrty::for_statement>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::for_statement& s,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "for {} in {} do {} done", s.var_name,
                             s.setup, s.body);
        return out;
    }
};

template <>
struct std::formatter<smrty::statement::ptr>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::statement::ptr& s,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        // all known subclasses of statement
        if (auto while_loop =
                std::dynamic_pointer_cast<smrty::while_statement>(s);
            while_loop)
        {
            out = std::format_to(out, "{}", *while_loop);
        }
        else if (auto for_loop =
                     std::dynamic_pointer_cast<smrty::for_statement>(s);
                 for_loop)
        {
            out = std::format_to(out, "{}", *for_loop);
        }
        else if (auto if_elif =
                     std::dynamic_pointer_cast<smrty::if_elif_statement>(s);
                 if_elif)
        {
            out = std::format_to(out, "{}", *if_elif);
        }
        else
        {
            constexpr std::string_view unknown_subclass{
                "unknown statement sub-class"};
            out = std::copy(unknown_subclass.begin(), unknown_subclass.end(),
                            out);
        }
        return out;
    }
};
