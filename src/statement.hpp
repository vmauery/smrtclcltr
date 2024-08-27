/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <parser_parts.hpp>
#include <string>
#include <variant>
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

struct keyword
{
    keyword() : word()
    {
    }
    keyword(const std::string& v) : word(v)
    {
    }
    std::string word;
};

struct time_parts;
struct function_parts;
struct program;

using simple_instruction =
    std::variant<std::monostate, bool, keyword, mpx, list, matrix, time_parts,
                 function_parts, symbolic, program>;
using simple_instructions = std::vector<simple_instruction>;

struct statement
{
    using ptr = std::shared_ptr<statement>;

    virtual ~statement() = default;
    virtual const simple_instruction& next_item(execution_flags&) = 0;
};

// TODO: add control statement types to instruction variant
using instruction = std::variant<simple_instruction, statement::ptr>;
using instructions = std::vector<instruction>;

} // namespace smrty

template <>
struct std::formatter<smrty::keyword>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::keyword& k,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}", k.word);
        return out;
    }
};
