/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <function_library.hpp>
#include <parser_parts.hpp>
#include <regex>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

// 6/7+3*8*-x^(y+2)*sin(5*z)
//           sym('+')
//          /        \
//   sym('/')      sym('*')
//     /   \        /     \
//  num(6) num(7) num(3) sym('*')
//                       /    \
//                    num(8)  sym('*')
//                             /    \
//                        num(-1)  sym('*')
//                                 /     \
//                           sym(exp)   sym(sin)(style=paren)
//                            /     \         \
//                       var(x)  sym()(paren) sym('*')
//                                /           /     \
//                           sym('+')      num(5)  var(z)
//                            /     \
//                         var(y)  num(2)
//

// defined in numeric.cpp; declared here for ease
mpx make_mpx(const smrty::number_parts&);

struct symbolic_base
{
    symbolic_base()
    {
    }
    virtual ~symbolic_base()
    {
    }
    virtual void make_me_polymorphic()
    {
    }
};
using symbolic_operand =
    std::variant<std::monostate, std::string, mpx, symbolic_base>;

struct symbolic : public symbolic_base
{
    symbolic() : fn_index(invalid_function), fn_style(symbolic_op::none)
    {
    }

    symbolic(const symbolic_parts& parts) :
        fn_index(parts.fn_index), fn_style(parts.fn_style)
    {
        auto get_arg = [](const auto& a) -> symbolic_operand {
            if constexpr (same_type_v<decltype(a), std::string>)
            {
                return a;
            }
            else if constexpr (same_type_v<decltype(a), number_parts>)
            {
                return make_mpx(a);
            }
            else if constexpr (same_type_v<decltype(a), symbolic_parts>)
            {
                return symbolic(a);
            }
            else
            {
                return std::monostate();
            }
        };
        left = get_arg(parts.left);
        right = get_arg(parts.right);
    }

    symbolic operator+(const symbolic& o) const
    {
        symbolic sum{};
        sum.fn_index = fn_id_by_name("+");
        sum.fn_style = symbolic_op::infix;
        sum.left = *this;
        sum.right = o;
        return sum;
    }

    symbolic operator-(const symbolic& o) const
    {
        symbolic diff{};
        diff.fn_index = fn_id_by_name("-");
        diff.fn_style = symbolic_op::infix;
        diff.left = *this;
        diff.right = o;
        return diff;
    }

    symbolic operator*(const symbolic& o) const
    {
        symbolic prod{};
        prod.fn_index = fn_id_by_name("*");
        prod.fn_style = symbolic_op::infix;
        prod.left = *this;
        prod.right = o;
        return prod;
    }

    symbolic operator/(const symbolic& o) const
    {
        symbolic div{};
        div.fn_index = fn_id_by_name("/");
        div.fn_style = symbolic_op::infix;
        div.left = *this;
        div.right = o;
        return div;
    }

    size_t fn_index;
    symbolic_op fn_style;
    symbolic_operand left;
    symbolic_operand right;
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
        // number_parts or symbolic_parts single operand
        if (sym.fn_index == smrty::invalid_function)
        {
            if (sym.fn_style == smrty::symbolic_op::paren)
            {
                // just lefty
                out = std::format_to(out, "{}", sym.left);
            }
            else if (auto v = std::get_if<std::string>(&sym.left); v)
            {
                // variable
                out = std::format_to(out, "{}", sym.left);
            }
            else if (auto v = std::get_if<mpx>(&sym.left); v)
            {
                out = std::format_to(out, "{}", sym.left);
            }
            return out;
        }
        // multi-part op

        out = std::format_to(out, "{}",
                             smrty::parser::fn_name_by_id(sym.fn_index));
        return out;
    }
};

template <>
struct std::formatter<smrty::symbolic_base>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::symbolic_base& b,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        // assume it's actually a symbolic
        try
        {
            auto sym = dynamic_cast<const smrty::symbolic&>(b);
            out = std::format_to(out, "{}", sym);
        }
        catch (const std::bad_cast&)
        {
            static constexpr std::string_view empty{"{}"};
            return std::copy(empty.begin(), empty.end(), out);
        }
        return out;
    }
};
