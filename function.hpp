/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <deque>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <string>
#include <tuple>
#include <type_traits>

// TODO: add the other 80 combinations? maybe as needed?
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpz& bb, const mpz& cc)
{
    return fn(aa, bb, cc);
}

template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpz& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpq& bb)
{
    return fn(to_mpq(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpf& bb)
{
    return fn(to_mpf(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const time_& bb)
{
    return fn(aa, bb);
}

// mpq
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpz& bb)
{
    return fn(aa, to_mpq(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpq& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpf& bb)
{
    return fn(to_mpf(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const time_& bb)
{
    return fn(to_mpf(aa), bb);
}

// mpf
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpz& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpq& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpf& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const time_& bb)
{
    return fn(aa, bb);
}

// mpc
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpz& bb)
{
    return fn(aa, to_mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpq& bb)
{
    return fn(aa, to_mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpf& bb)
{
    return fn(aa, to_mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpc& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const time_& bb)
{
    return fn(to_mpf(aa), bb);
}

// time_
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa, const mpz& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa, const mpq& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa, const mpf& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa, const mpc& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa, const time_& bb)
{
    return fn(aa, bb);
}

template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const time_& aa)
{
    return fn(aa);
}

constexpr bool const_or(bool b0)
{
    return b0;
}

template <typename B0, typename... BN>
constexpr bool const_or(B0 b0, BN... bN)
{
    return b0 | const_or(bN...);
}

template <typename... Ttypes, typename... Vtypes>
bool variant_holds_type(const std::variant<Vtypes...>& v)
{
    return const_or(std::holds_alternative<Ttypes>(v)...);
}

template <typename T, typename Vtype>
struct variant_has_member;

template <typename T, typename... Vtypes>
struct variant_has_member<T, std::variant<Vtypes...>>
    : public std::disjunction<std::is_same<T, Vtypes>...>
{
};

template <typename Vin, typename Vout>
struct reduce
{
    constexpr static size_t A_size = std::variant_size<Vin>::value;

    Vin& _vin;
    Vout& _vout;

    reduce(Vin& vin, Vout& vout) : _vin(vin), _vout(vout)
    {
    }

    bool extract_I(std::integral_constant<size_t, A_size>)
    {
        return false;
    }

    template <size_t I>
    bool extract_I(
        std::integral_constant<size_t, I> = std::integral_constant<size_t, 0>())
    {
        if constexpr (variant_has_member<std::variant_alternative_t<I, Vin>,
                                         Vout>::value)
        {
            auto p = std::get_if<I>(&_vin);
            if (p)
            {
                _vout = *p;
                return true;
            }
        }
        return extract_I(std::integral_constant<size_t, I + 1>());
    }

    bool operator()()
    {
        return extract_I<0>();
    }
};

template <typename TypeOut, typename TypeIn>
TypeOut coerce_variant(const TypeIn& in)
{
    return TypeOut(in);
}

template <std::size_t N, class... Args>
using list_type_t = std::tuple_element_t<N, std::tuple<Args...>>;

template <typename... I>
struct conversion
{
    static void op(numeric&)
    {
    }
};

template <typename... Ti, typename... To>
struct conversion<std::tuple<Ti...>, std::tuple<To...>>
{
    constexpr static size_t I_size = sizeof...(Ti);

    static void op(numeric&, std::integral_constant<size_t, I_size>)
    {
    }
    template <size_t I>
    static void op(numeric& v, std::integral_constant<size_t, I>)
    {
        if (std::holds_alternative<list_type_t<I, Ti...>>(v))
        {
            v = coerce_variant<list_type_t<I, To...>>(
                std::get<list_type_t<I, Ti...>>(v));
        }
        op(v, std::integral_constant<size_t, I + 1>());
    }
    static void op(numeric& v)
    {
        static_assert(I_size >= 1);
        static_assert(sizeof...(Ti) == sizeof...(To));
        op(v, std::integral_constant<size_t, 0>());
    }
};

template <typename Fn>
bool one_arg_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit([&fn](const auto& a) { return operate(fn, a); },
                        a.value());
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        throw;
    }
    calc.stack.emplace_front(std::move(cv), calc.config.base,
                             calc.config.fixed_bits, a.precision,
                             calc.config.is_signed);
    return true;
}

template <typename Fn, typename... Itypes, typename... Otypes,
          typename... Ltypes>
bool one_arg_conv_op(Calculator& calc, const Fn& fn,
                     const std::tuple<Itypes...>& /* in */,
                     const std::tuple<Otypes...>& /* out */,
                     const std::tuple<Ltypes...>& /*limit*/)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    numeric& ca = a.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
    std::variant<Ltypes...> lca;
    if (!reduce(ca, lca)())
    {
        std::cerr << "wtf, we converted and now it doesn't reduce? Eat "
                     "shit c++.\n";
        return false;
    }
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit([&fn](const auto& a) { return operate(fn, a); }, lca);
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        throw;
    }
    calc.stack.emplace_front(std::move(cv), calc.config.base,
                             calc.config.fixed_bits, a.precision,
                             calc.config.is_signed);
    return true;
}

template <typename... AllowedTypes, typename Fn>
bool one_arg_limited_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 1)
    {
        return false;
    }
    stack_entry a = calc.stack.front();
    std::variant<AllowedTypes...> la;
    if (!variant_holds_type<AllowedTypes...>(a.value()))
    {
        return false;
    }
    if (!reduce(a.value(), la)())
    {
        return false;
    }
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit([&fn](const auto& a) { return operate(fn, a); }, la);
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        throw;
    }
    calc.stack.emplace_front(std::move(cv), calc.config.base,
                             calc.config.fixed_bits, a.precision,
                             calc.config.is_signed);
    return true;
}

template <typename Fn>
bool two_arg_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack[1];
    stack_entry b = calc.stack[0];
    calc.stack.pop_front();
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            a.value(), b.value());
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        calc.stack.push_front(b);
        throw;
    }
    calc.stack.emplace_front(
        std::move(cv), calc.config.base, calc.config.fixed_bits,
        std::min(a.precision, b.precision), calc.config.is_signed);
    return true;
}

template <typename Fn, typename... Itypes, typename... Otypes,
          typename... Ltypes>
bool two_arg_conv_op(Calculator& calc, const Fn& fn,
                     const std::tuple<Itypes...>& /* in */,
                     const std::tuple<Otypes...>& /* out */,
                     const std::tuple<Ltypes...>& /*limit*/)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack[1];
    stack_entry b = calc.stack[0];

    numeric& ca = a.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
    numeric& cb = b.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
    std::variant<Ltypes...> lca;
    std::variant<Ltypes...> lcb;
    if (!reduce(ca, lca)() || !reduce(cb, lcb)())
    {
        std::cerr << "wtf, we converted and now it doesn't reduce? Eat "
                     "shit c++.\n";
        return false;
    }
    calc.stack.pop_front();
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            lca, lcb);
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        calc.stack.push_front(b);
        throw;
    }
    calc.stack.emplace_front(
        std::move(cv), calc.config.base, calc.config.fixed_bits,
        std::min(a.precision, b.precision), calc.config.is_signed);
    return true;
}

template <typename... AllowedTypes, typename Fn>
bool two_arg_limited_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }

    stack_entry a = calc.stack[1];
    stack_entry b = calc.stack[0];

    if (!variant_holds_type<AllowedTypes...>(a.value()) ||
        !variant_holds_type<AllowedTypes...>(b.value()))
    {
        return false;
    }
    std::variant<AllowedTypes...> la;
    std::variant<AllowedTypes...> lb;
    if (!reduce(a.value(), la)() || !reduce(b.value(), lb)())
    {
        return false;
    }
    calc.stack.pop_front();
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            la, lb);
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        calc.stack.push_front(b);
        throw;
    }
    calc.stack.emplace_front(
        std::move(cv), calc.config.base, calc.config.fixed_bits,
        std::min(a.precision, b.precision), calc.config.is_signed);
    return true;
}

template <typename... AllowedTypes, typename Fn>
bool three_arg_limited_op(Calculator& calc, const Fn& fn,
                          const std::tuple<AllowedTypes...>& /*allowed types*/)
{
    if (calc.stack.size() < 3)
    {
        return false;
    }

    stack_entry a = calc.stack[2];
    stack_entry b = calc.stack[1];
    stack_entry c = calc.stack[0];

    if (!variant_holds_type<AllowedTypes...>(a.value()) ||
        !variant_holds_type<AllowedTypes...>(b.value()) ||
        !variant_holds_type<AllowedTypes...>(c.value()))
    {
        return false;
    }
    std::variant<AllowedTypes...> la;
    std::variant<AllowedTypes...> lb;
    std::variant<AllowedTypes...> lc;
    if (!reduce(a.value(), la)() || !reduce(b.value(), lb)() ||
        !reduce(c.value(), lc)())
    {
        return false;
    }
    calc.stack.pop_front();
    calc.stack.pop_front();
    calc.stack.pop_front();

    numeric cv;
    try
    {
        cv = std::visit([&fn](const auto& a, const auto& b,
                              const auto& c) { return operate(fn, a, b, c); },
                        la, lb, lc);
    }
    catch (const std::exception& e)
    {
        calc.stack.push_front(a);
        calc.stack.push_front(b);
        calc.stack.push_front(c);
        throw;
    }
    calc.stack.emplace_front(std::move(cv), calc.config.base,
                             calc.config.fixed_bits,
                             std::min({a.precision, b.precision, c.precision}),
                             calc.config.is_signed);
    return true;
}

template <typename Fn>
auto scaled_trig_op(Calculator& calc, auto a, const Fn& fn)
{
    if (calc.config.angle_mode == Calculator::e_angle_mode::deg)
    {
        a *= boost::math::constants::pi<mpf>() / 180;
    }
    else if (calc.config.angle_mode == Calculator::e_angle_mode::grad)
    {
        a *= boost::math::constants::pi<mpf>() / 50;
    }
    return fn(a);
}

template <typename Fn>
auto scaled_trig_op_inv(Calculator& calc, const auto& a, const Fn& fn)
{
    auto b = fn(a);
    if (calc.config.angle_mode == Calculator::e_angle_mode::deg)
    {
        b *= 180 / boost::math::constants::pi<mpf>();
    }
    else if (calc.config.angle_mode == Calculator::e_angle_mode::grad)
    {
        b *= 50 / boost::math::constants::pi<mpf>();
    }
    return b;
}
