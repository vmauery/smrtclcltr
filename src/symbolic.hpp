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

struct symbolic_actual;
class symbolic
{
  public:
    symbolic();
    symbolic(const symbolic_parts_ptr& o);
    symbolic(const symbolic& o);
    symbolic& operator=(const symbolic& o);
    symbolic_actual& operator*() const;

    symbolic operator+(const symbolic& o) const;
    symbolic operator-(const symbolic& o) const;
    symbolic operator*(const symbolic& o) const;
    symbolic operator/(const symbolic& o) const;

  protected:
    std::shared_ptr<symbolic_actual> ptr;
};

using symbolic_operand =
    std::variant<std::monostate, std::string, mpx, symbolic>;

struct symbolic_actual
{
    explicit symbolic_actual(symbolic& creator);
    symbolic_actual(symbolic& creator, const symbolic_actual& o);
    symbolic_actual(symbolic& creator, const symbolic_parts& parts);

    symbolic operator+(const symbolic_actual& o) const;
    symbolic operator-(const symbolic_actual& o) const;
    symbolic operator*(const symbolic_actual& o) const;
    symbolic operator/(const symbolic_actual& o) const;

    std::reference_wrapper<symbolic> box;
    size_t fn_index;
    symbolic_op fn_style;
    symbolic_operand left;
    symbolic_operand right;
};

} // namespace smrty

template <>
struct std::formatter<smrty::symbolic_actual>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::symbolic_actual& sym,
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
        if (sym.fn_style == smrty::symbolic_op::infix)
        {
            out = std::format_to(out, "{}{}{}", sym.left,
                                 smrty::fn_name_by_id(sym.fn_index), sym.right);
        }
        else if (sym.fn_style == smrty::symbolic_op::prefix)
        {
            out = std::format_to(out, "{}{}",
                                 smrty::fn_name_by_id(sym.fn_index), sym.left);
        }
        else if (sym.fn_style == smrty::symbolic_op::postfix)
        {
            out = std::format_to(out, "{}{}", sym.left,
                                 smrty::fn_name_by_id(sym.fn_index));
        }
        else
        {
            out = std::format_to(out, "({}: {}, {})",
                                 smrty::fn_name_by_id(sym.fn_index), sym.left,
                                 sym.right);
        }
        return out;
    }
};

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
    auto format(const smrty::symbolic& b,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "{}", *b);
        return out;
    }
};
