/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <numeric.hpp>
#include <symbolic.hpp>

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
symbolic::symbolic() : ptr(std::make_shared<symbolic_actual>(*this))
{
}
symbolic::symbolic(const symbolic_parts_ptr& o) :
    ptr(std::make_shared<symbolic_actual>(*this, o()))
{
    lg::debug("here\n");
}
symbolic::symbolic(const symbolic& o) :
    ptr(std::make_shared<symbolic_actual>(*this, *o))
{
    lg::debug("here\n");
}
symbolic& symbolic::operator=(const symbolic& o)
{
    lg::debug("here\n");
    ptr = std::make_shared<symbolic_actual>(*o);
    return *this;
}
symbolic_actual& symbolic::operator*() const
{
    return *ptr;
}

symbolic symbolic::operator+(const symbolic& o) const
{
    return *ptr + *o;
}
symbolic symbolic::operator-(const symbolic& o) const
{
    return *ptr - *o;
}
symbolic symbolic::operator*(const symbolic& o) const
{
    return *ptr * (*o);
}
symbolic symbolic::operator/(const symbolic& o) const
{
    return *ptr / (*o);
}

symbolic_actual::symbolic_actual(symbolic& creator) :
    box(std::ref(creator)), fn_index(invalid_function),
    fn_style(symbolic_op::none)
{
}

symbolic_actual::symbolic_actual(symbolic& creator, const symbolic_actual& o) :
    box(std::ref(creator)), fn_index(o.fn_index), fn_style(o.fn_style)
{
    auto get_arg = [](const auto& a) -> symbolic_operand {
        if constexpr (same_type_v<decltype(a), std::string> ||
                      same_type_v<decltype(a), mpx>)
        {
            lg::debug("arg is variable or numeric\n");
            return a;
        }
        else if constexpr (same_type_v<decltype(a), symbolic>)
        {
            lg::debug("arg is symbolic\n");
            return symbolic{a};
        }
        else
        {
            lg::debug("arg is monostate\n");
            return std::monostate();
        }
    };
    left = std::visit(get_arg, o.left);
    right = std::visit(get_arg, o.right);
}

symbolic_actual::symbolic_actual(symbolic& creator,
                                 const symbolic_parts& parts) :
    box(std::ref(creator)), fn_index(parts.fn_index), fn_style(parts.fn_style)
{
    lg::debug("incoming parts: {}\n", parts);
    // std::monostate, std::string, number_parts, symbolic_parts_ptr
    auto get_arg = [](const auto& a) -> symbolic_operand {
        if constexpr (same_type_v<decltype(a), std::string>)
        {
            lg::debug("arg is variable\n");
            return a;
        }
        else if constexpr (same_type_v<decltype(a), number_parts>)
        {
            lg::debug("arg is numeric\n");
            return make_mpx(a);
        }
        else if constexpr (same_type_v<decltype(a), symbolic_parts_ptr>)
        {
            lg::debug("arg is symbolic\n");
            return symbolic(a);
        }
        else
        {
            lg::debug("arg is monostate\n");
            return std::monostate();
        }
    };
    left = std::visit(get_arg, parts.left);
    right = std::visit(get_arg, parts.right);
}

symbolic symbolic_actual::operator+(const symbolic_actual& o) const
{
    symbolic s{};
    symbolic_actual& sum = *s;
    sum.fn_index = fn_id_by_name("+");
    sum.fn_style = symbolic_op::infix;
    sum.left = box.get();
    sum.right = o.box.get();
    lg::debug("symbolic addition: {}\n", s);
    return s;
}

symbolic symbolic_actual::operator-(const symbolic_actual& o) const
{
    symbolic d{};
    symbolic_actual& diff = *d;
    diff.fn_index = fn_id_by_name("-");
    diff.fn_style = symbolic_op::infix;
    diff.left = box.get();
    diff.right = o.box.get();
    return d;
}

symbolic symbolic_actual::operator*(const symbolic_actual& o) const
{
    symbolic p{};
    symbolic_actual& prod = *p;
    prod.fn_index = fn_id_by_name("*");
    prod.fn_style = symbolic_op::infix;
    prod.left = box.get();
    prod.right = o.box.get();
    return p;
}

symbolic symbolic_actual::operator/(const symbolic_actual& o) const
{
    symbolic d{};
    symbolic_actual& div = *d;
    div.fn_index = fn_id_by_name("/");
    div.fn_style = symbolic_op::infix;
    div.left = box.get();
    div.right = o.box.get();
    return d;
}

} // namespace smrty
