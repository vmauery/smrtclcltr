/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#include <boost/multiprecision/number.hpp>
#include <calculator.hpp>
#include <cmath>
#include <debug.hpp>
#include <deque>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <string>
#include <tuple>
#include <type_traits>
#include <units.hpp>

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

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, a.value());
    calc.stack.pop_front();
    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
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
    numeric ca = a.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
    std::variant<Ltypes...> lca;
    if (!reduce(ca, lca)())
    {
        lg::error("wtf, we converted and now it doesn't reduce? Eat "
                  "shit c++.\n");
        return false;
    }

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, lca);
    calc.stack.pop_front();

    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
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

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, la);

    calc.stack.pop_front();
    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
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

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()));
    }
    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}, ub{b.unit()}](const auto& a, const auto& b) {
            return fn(a, b, ua, ub);
        },
        a.value(), b.value());

    calc.stack.pop_front();
    calc.stack.pop_front();

    calc.stack.emplace_front(
        std::move(cv), nu, calc.config.base, calc.config.fixed_bits,
        std::min(a.precision, b.precision), calc.config.is_signed);
    return true;
}

template <typename Fn>
bool two_arg_uconv_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        return false;
    }
    stack_entry a = calc.stack[1];
    stack_entry b = calc.stack[0];

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()));
    }
    else if (units::are_temp_units(a.unit(), b.unit()))
    {
        b.value(std::visit(
            [ub{b.unit()}, ua{a.unit()}](const auto& v) {
                return units::scale_temp_units(v, ub, ua);
            },
            b.value()));
        b.unit(a.unit());
    }

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}, ub{b.unit()}](const auto& a, const auto& b) {
            return fn(a, b, ua, ub);
        },
        a.value(), b.value());

    calc.stack.pop_front();
    calc.stack.pop_front();

    calc.stack.emplace_front(
        std::move(cv), nu, calc.config.base, calc.config.fixed_bits,
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

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()));
    }
    numeric ca = a.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
    numeric cb = b.value();
    conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
    std::variant<Ltypes...> lca;
    std::variant<Ltypes...> lcb;
    if (!reduce(ca, lca)() || !reduce(cb, lcb)())
    {
        lg::error("wtf, we converted and now it doesn't reduce? Eat "
                  "shit c++.\n");
        return false;
    }
    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}, ub{b.unit()}](const auto& a, const auto& b) {
            return fn(a, b, ua, ub);
        },
        lca, lcb);

    calc.stack.pop_front();
    calc.stack.pop_front();

    calc.stack.emplace_front(
        std::move(cv), nu, calc.config.base, calc.config.fixed_bits,
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

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()));
    }
    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}, ub{b.unit()}](const auto& a, const auto& b) {
            return fn(a, b, ua, ub);
        },
        la, lb);
    calc.stack.pop_front();
    calc.stack.pop_front();

    calc.stack.emplace_front(
        std::move(cv), nu, calc.config.base, calc.config.fixed_bits,
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

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()));
    }
    if (a.unit().compat(c.unit()))
    {
        // convert c to a units
        c.value(units::convert(c.value(), c.unit(), a.unit()));
    }
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

    auto [cv, nu] =
        std::visit([&fn, ua{a.unit()}, ub{b.unit()}, uc{c.unit()}](
                       const auto& a, const auto& b,
                       const auto& c) { return fn(a, b, c, ua, ub, uc); },
                   la, lb, lc);

    calc.stack.pop_front();
    calc.stack.pop_front();
    calc.stack.pop_front();

    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                             calc.config.fixed_bits,
                             std::min({a.precision, b.precision, c.precision}),
                             calc.config.is_signed);
    return true;
}

template <typename Fn>
std::tuple<numeric, units::unit> scaled_trig_op(Calculator& calc, auto a,
                                                const Fn& fn)
{
    if (calc.config.angle_mode == Calculator::e_angle_mode::deg)
    {
        a *= boost::math::constants::pi<mpf>() / 180;
    }
    else if (calc.config.angle_mode == Calculator::e_angle_mode::grad)
    {
        a *= boost::math::constants::pi<mpf>() / 50;
    }
    return {fn(a), units::unit()};
}

template <typename Fn>
std::tuple<numeric, units::unit> scaled_trig_op_inv(Calculator& calc,
                                                    const auto& a, const Fn& fn)
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
    return {b, units::unit()};
}
