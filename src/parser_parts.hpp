/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <function_library.hpp>
#include <numeric.hpp>
#include <parser.hpp>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

struct single_number_parts
{
    single_number_parts() :
        base(10), mantissa_sign(1), mantissa(), exponent_sign(1), exponent(),
        full()
    {
    }
    single_number_parts(int sign, const std::string& value) :
        base(10), mantissa_sign(sign), mantissa(value), exponent_sign(1),
        exponent(), full(std::format("{}{}", (sign < 0 ? "-" : ""), value))
    {
    }

    int base;
    int mantissa_sign;
    std::string mantissa;
    int exponent_sign;
    std::string exponent;
    std::string full; // full string for number?
};

struct two_number_parts
{
    enum class type
    {
        cmplx,
        rtnl,
    };
    two_number_parts() :
        number_type(type::rtnl), polar(false), first(), second()
    {
    }
    type number_type = type::rtnl;
    bool polar = false;
    single_number_parts first;
    single_number_parts second;
};

using number_parts = std::variant<single_number_parts, two_number_parts>;

struct function_parts
{
    function_parts() : fn_ptr(nullptr), fn_style(symbolic_op::none), re_args()
    {
    }
    explicit function_parts(CalcFunction::ptr fn) :
        fn_ptr(fn), fn_style(symbolic_op::none), re_args()
    {
    }
    CalcFunction::ptr fn_ptr;
    symbolic_op fn_style;

    // re_args is used directly to execute re_op functions
    std::vector<std::string> re_args;
};

struct time_parts
{
    time_parts() :
        absolute(false), year(), month(), day(), h(), m(), s(), sub(), tz(),
        duration(), suffix(), full()
    {
    }
    // amended ISO8601 format: // yyyy-mm-dd[Thh:mm:ss[.sub]][Z|([+-]hh:mm)]
    bool absolute;
    std::string year;
    std::string month;
    std::string day;
    std::string h;
    std::string m;
    std::string s;
    std::string sub;
    std::string tz;
    single_number_parts duration;
    std::string suffix;
    std::string full;
};

} // namespace smrty

template <>
struct std::formatter<smrty::function_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::function_parts& fn, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}", fn_get_name(fn.fn_ptr));
        if (fn.re_args.size())
        {
            out = std::format_to(out, "({:, })", fn.re_args);
        }
        return out;
    }
};

template <>
struct std::formatter<smrty::single_number_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::single_number_parts& n, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}{}", (n.mantissa_sign < 0 ? "-" : ""),
                             n.mantissa);
        if (n.exponent.size())
        {
            out = std::format_to(out, "e{}{}", (n.exponent_sign < 0 ? "-" : ""),
                                 n.exponent);
        }
        return out;
    }
};

template <>
struct std::formatter<smrty::two_number_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::two_number_parts& n, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        if (n.number_type == smrty::two_number_parts::type::cmplx)
        {
            out = std::format_to(out, "({}{}{})", n.first,
                                 (n.polar ? ",<" : ","), n.second);
        }
        else if (n.number_type == smrty::two_number_parts::type::rtnl)
        {
            out = std::format_to(out, "{}/{}", n.first, n.second);
        }
        return out;
    }
};

template <>
struct std::formatter<smrty::time_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::time_parts& t, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        if (t.absolute)
        {
            std::format_to(out, "{}-{}-{}", t.year, t.month, t.day);
            if (t.h.size())
            {
                std::format_to(out, "T{}:{}:{}", t.h, t.m, t.s);
            }
            if (t.sub.size())
            {
                out = std::format_to(out, "{}", t.sub);
            }
            if (t.tz.size())
            {
                out = std::format_to(out, "{}", t.tz);
            }
        }
        else
        {
            out = std::format_to(out, "{}{}", t.duration, t.suffix);
        }
        return out;
    }
};
