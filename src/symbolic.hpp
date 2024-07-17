/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <regex>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

enum class symbolic_op
{
    none,
    paren,
    prefix,
    infix,
    postfix,
};

struct symbolic
{
    symbolic(const std::string& value) : value(value)
    {
    }
    symbolic(std::string&& value) : value(std::move(value))
    {
    }

    // ultimately, using a tree to represent this would be ideal
    // Then adding new operations and values is just adding nodes
    // printing is easy. evaluation is easy.
    std::string value;

    symbolic operator+(const symbolic& o) const
    {
        return value + "+" + o.value;
    }
    symbolic operator-(const symbolic& o) const
    {
        return value + "-" + o.value;
    }
    symbolic operator*(const symbolic& o) const
    {
        return value + "*" + o.value;
    }
    symbolic operator/(const symbolic& o) const
    {
        return value + "/" + o.value;
    }
};

} // namespace smrty

template <>
struct std::formatter<smrty::symbolic>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::symbolic& sym,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::copy(sym.value.begin(), sym.value.end(), out);
        return out;
    }
};
