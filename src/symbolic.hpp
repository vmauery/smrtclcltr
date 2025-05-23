/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <format>
#include <function_library.hpp>
#include <regex>
#include <std_container_format.hpp>
#include <string>
#include <vector>

namespace smrty
{

constexpr auto invalid_function = nullptr;

enum class symbolic_op
{
    none,
    paren,
    prefix,
    infix,
    postfix,
};

/*
 6/7+3*8*-x^(y+2)*sin(5*z)
           sym('+')
          /        \
   sym('/')      sym('*')
     /   \        /     \
  num(6) num(7) num(3) sym('*')
                       /    \
                    num(8)  sym('*')
                             /    \
                        num(-1)  sym('*')
                                 /     \
                           sym(exp)   sym(sin)(style=paren)
                            /     \         \
                       var(x)  sym()(paren) sym('*')
                                /           /     \
                           sym('+')      num(5)  var(z)
                            /     \
                         var(y)  num(2)
*/

class Calculator;
struct symbolic_actual;
class symbolic
{
  public:
    symbolic();
    symbolic(const symbolic& o);
    symbolic(symbolic&& o);
    symbolic(const std::string& o);
    explicit symbolic(const mpx& o);
    template <typename T>
        requires is_one_of_v<T, mpx>
    explicit symbolic(const T& o) : symbolic(mpx{o})
    {
    }
    ~symbolic();

    symbolic& operator=(const symbolic& o);
    symbolic& operator=(symbolic&& o);
    symbolic_actual& operator*() const;

    symbolic operator+(const symbolic& o) const;
    symbolic operator-(const symbolic& o) const;
    symbolic operator*(const symbolic& o) const;
    symbolic operator/(const symbolic& o) const;
    symbolic operator%(const symbolic& o) const;

    void eval(Calculator&) const;

  protected:
    std::shared_ptr<symbolic_actual> ptr;
};

using symbolic_operand =
    std::variant<std::monostate, std::string, mpx, symbolic>;

enum class fn_prio
{
    addsub,
    multdiv,
    negate,
    exponent,
    factorial,
    atomic
};

struct symbolic_actual
{
    explicit symbolic_actual(symbolic& creator);
    symbolic_actual(symbolic& creator, const symbolic_actual& o);
    symbolic_actual(symbolic& creator, const mpx& o);
    symbolic_actual(symbolic& creator, const std::string& o);
    ~symbolic_actual();

    symbolic operator+(const symbolic_actual& o) const;
    symbolic operator-(const symbolic_actual& o) const;
    symbolic operator*(const symbolic_actual& o) const;
    symbolic operator/(const symbolic_actual& o) const;
    symbolic operator%(const symbolic_actual& o) const;

    void eval(Calculator&) const;
    fn_prio prio() const;

    std::reference_wrapper<symbolic> box;
    CalcFunction::ptr fn_ptr;
    symbolic_op fn_style;
    symbolic_operand left;
    symbolic_operand right;
};

symbolic floor(const symbolic& v);
symbolic ceil(const symbolic& v);
symbolic round(const symbolic& v);
symbolic lcm(const symbolic& a, const symbolic& b);
symbolic lcm(const symbolic& a, const mpz& b);
symbolic lcm(const mpz& a, const symbolic& b);
symbolic gcd(const symbolic& a, const symbolic& b);
symbolic gcd(const symbolic& a, const mpz& b);
symbolic gcd(const mpz& a, const symbolic& b);
symbolic pow(const symbolic& a, const symbolic& b);
symbolic pow(const mpx& a, const symbolic& b);
symbolic pow(const symbolic& a, const mpx& b);
symbolic exp(const symbolic& v);
symbolic factorial(const symbolic& v);
symbolic tgamma(const symbolic& v);
symbolic zeta(const symbolic& v);
symbolic abs(const symbolic& v);
symbolic log(const symbolic& v);
symbolic ln(const symbolic& v);
symbolic sqrt(const symbolic& v);
symbolic sin(const symbolic& v);
symbolic cos(const symbolic& v);
symbolic tan(const symbolic& v);
symbolic asin(const symbolic& v);
symbolic acos(const symbolic& v);
symbolic atan(const symbolic& v);
symbolic sinh(const symbolic& v);
symbolic cosh(const symbolic& v);
symbolic tanh(const symbolic& v);
symbolic asinh(const symbolic& v);
symbolic acosh(const symbolic& v);
symbolic atanh(const symbolic& v);

} // namespace smrty

#if (USE_BOOST_CPP_BACKEND || USE_GMP_BACKEND || USE_MPFR_BACKEND)
namespace boost
{
namespace multiprecision
{
using ::smrty::ceil;
using ::smrty::exp;
using ::smrty::floor;
using ::smrty::pow;
using ::smrty::round;
} // namespace multiprecision
namespace math
{
using ::smrty::gcd;
using ::smrty::lcm;
using ::smrty::tgamma;
using ::smrty::zeta;
} // namespace math
} // namespace boost

using ::smrty::abs;
using ::smrty::acos;
using ::smrty::acosh;
using ::smrty::asin;
using ::smrty::asinh;
using ::smrty::atan;
using ::smrty::atanh;
using ::smrty::cos;
using ::smrty::cosh;
using ::smrty::log;
using ::smrty::sin;
using ::smrty::sinh;
using ::smrty::sqrt;
using ::smrty::tan;
using ::smrty::tanh;
#endif

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
    auto format(const smrty::symbolic_actual& sym, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        // mpx or symbolic single operand
        if (sym.fn_ptr == smrty::invalid_function)
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
            // check for adding explicit parenthesis
            // left side
            if (auto l = std::get_if<smrty::symbolic>(&(sym.left));
                l && (*(*l)).fn_ptr != smrty::invalid_function &&
                (*(*l)).prio() < sym.prio())
            {
                out = std::format_to(out, "({})", *l);
            }
            else
            {
                out = std::format_to(out, "{}", sym.left);
            }
            // op
            out = std::format_to(out, "{}", fn_get_name(sym.fn_ptr));
            // right side
            if (auto r = std::get_if<smrty::symbolic>(&(sym.right));
                r && (*(*r)).fn_ptr != smrty::invalid_function &&
                (*(*r)).prio() < sym.prio())
            {
                out = std::format_to(out, "({})", *r);
            }
            else
            {
                out = std::format_to(out, "{}", sym.right);
            }
        }
        else if (sym.fn_style == smrty::symbolic_op::prefix)
        {
            out =
                std::format_to(out, "{}{}", fn_get_name(sym.fn_ptr), sym.left);
        }
        else if (sym.fn_style == smrty::symbolic_op::postfix)
        {
            out =
                std::format_to(out, "{}{}", sym.left, fn_get_name(sym.fn_ptr));
        }
        else if (sym.fn_style == smrty::symbolic_op::paren)
        {
            out =
                std::format_to(out, "{}({}", fn_get_name(sym.fn_ptr), sym.left);
            if (!std::get_if<std::monostate>(&sym.right))
            {
                out = std::format_to(out, ", {}", sym.right);
            }
            *out++ = ')';
        }
        else /* none */
        {
            out = std::format_to(out, "({}: {}, {})", fn_get_name(sym.fn_ptr),
                                 sym.left, sym.right);
        }
        /*
    none,
    paren,
    prefix,
    infix,
    postfix,
    */
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
    auto format(const smrty::symbolic& b, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        // only quote the outer-most symbolic
        static bool quotes = true;
        auto out = ctx.out();
        if (quotes)
        {
            quotes = false;
            out = std::format_to(out, "'{}'", *b);
            quotes = true;
        }
        else
        {
            out = std::format_to(out, "{}", *b);
        }
        return out;
    }
};
