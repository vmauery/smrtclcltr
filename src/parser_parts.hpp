/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <function_library.hpp>
#include <parser.hpp>
#include <regex>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

constexpr size_t invalid_function = static_cast<size_t>(-1);

enum class symbolic_op
{
    none,
    paren,
    prefix,
    infix,
    postfix,
};

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

struct symbolic_parts;
struct symbolic_parts_ptr
{
    symbolic_parts_ptr() : ptr(std::make_shared<symbolic_parts>())
    {
    }
    symbolic_parts& operator()()
    {
        return *ptr;
    }
    const symbolic_parts& operator()() const
    {
        return *ptr;
    }
    std::shared_ptr<symbolic_parts> ptr;
};

struct symbolic_parts
{
    using operand = std::variant<std::monostate, std::string, number_parts,
                                 symbolic_parts_ptr>;

    symbolic_parts() :
        count(counter++), fn_index(invalid_function),
        fn_style(symbolic_op::none), left(std::monostate()),
        right(std::monostate())
    {
    }
    size_t fn_index;
    symbolic_op fn_style;

    // operand is a number, or a variable, or other symbolic
    operand left;
    operand right;

    size_t count;
    static size_t counter;
};

struct function_parts
{
    function_parts() :
        fn_index(invalid_function), fn_style(symbolic_op::none), re_args()
    {
    }
    explicit function_parts(size_t id) :
        fn_index(id), fn_style(symbolic_op::none), re_args()
    {
    }
    size_t fn_index;
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

struct compound_parts
{
    compound_parts() : cols(0), items()
    {
    }
    size_t cols;
    std::vector<number_parts> items;
    // if cols is 0, it's a list
};

} // namespace smrty

/*
template <>
struct std::formatter<smrty::symbolic_parts>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin());

    template <typename FormatContext>
    auto format(const smrty::function_parts& fn,
                FormatContext& ctx) const -> decltype(ctx.out());
};
*/

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
    auto format(const smrty::function_parts& fn,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}", smrty::fn_name_by_id(fn.fn_index));
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
    auto format(const smrty::single_number_parts& n,
                FormatContext& ctx) const -> decltype(ctx.out())
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
    auto format(const smrty::two_number_parts& n,
                FormatContext& ctx) const -> decltype(ctx.out())
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
struct std::formatter<smrty::compound_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::compound_parts& n,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        if (n.cols)
        {
            *out++ = '[';
            size_t c = 0;
            auto iter = n.items.begin();
            while (iter != n.items.end())
            {
                if ((c % n.cols) == 0)
                {
                    *out++ = '[';
                }
                out = std::format_to(out, "{}", *iter++);
                if ((c % n.cols) == (n.cols - 1))
                {
                    *out++ = ']';
                }
                else
                {
                    *out++ = ' ';
                }
                c++;
            }
            *out++ = ']';
        }
        else
        {
            out = std::format_to(out, "{}", n.items);
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
    auto format(const smrty::time_parts& t,
                FormatContext& ctx) const -> decltype(ctx.out())
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

template <>
struct std::formatter<smrty::symbolic_parts>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::symbolic_parts& sym,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out =
            std::format_to(out, "[#{}: id:{}, style:{}, left:{}, right:{}]",
                           sym.count, smrty::fn_name_by_id(sym.fn_index),
                           static_cast<int>(sym.fn_style), sym.left, sym.right);
        return out;
        // number_parts or symbolic_parts single operand
        if (sym.fn_index == smrty::invalid_function)
        {
            out = std::format_to(out, "[#{}: {}]", sym.count, sym.left);
            return out;
        }
        // multi-part op
        if (sym.fn_style == smrty::symbolic_op::infix)
        {
            out = std::format_to(out, "[#{}: {} {} {}]", sym.count, sym.left,
                                 smrty::fn_name_by_id(sym.fn_index), sym.right);
        }
        else
        {
            out = std::format_to(out, "[#{}: {}({})]", sym.count,
                                 smrty::fn_name_by_id(sym.fn_index), sym.left);
        }
        return out;
    }
};

template <>
struct std::formatter<smrty::symbolic_parts_ptr>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::symbolic_parts_ptr& ptr,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}", *(ptr.ptr));
        return out;
    }
};
