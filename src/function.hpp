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
#include <exception.hpp>
#include <function_library.hpp>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <string>
#include <tuple>
#include <type_helpers.hpp>
#include <units.hpp>

namespace smrty
{

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
        throw std::invalid_argument("Requires 1 argument");
    }
    stack_entry& a = calc.stack.front();

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, a.value());
    calc.stack.pop_front();
    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                             calc.config.fixed_bits, a.precision,
                             calc.config.is_signed, calc.flags);
    return true;
}

template <typename It, typename Ot, typename Lt>
struct one_arg_conv
{
    template <typename Fn>
    static bool op(Calculator&, const Fn&)
    {
        return false;
    }
};
template <typename... Itypes, typename... Otypes, typename... Ltypes>
struct one_arg_conv<ITypes<Itypes...>, OTypes<Otypes...>, LTypes<Ltypes...>>
{
    template <typename Fn>
    static bool op(Calculator& calc, const Fn& fn)
    {
        if (calc.stack.size() < 1)
        {
            throw std::invalid_argument("Requires 1 argument");
        }
        stack_entry& a = calc.stack.front();
        numeric ca = a.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        std::variant<Ltypes...> lca;
        if (!reduce(ca, lca)())
        {
            throw std::runtime_error(
                "Argument failed to reduce after conversion");
        }

        auto [cv, nu] = std::visit(
            [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, lca);
        calc.stack.pop_front();

        calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                                 calc.config.fixed_bits, a.precision,
                                 calc.config.is_signed, calc.flags);
        return true;
    }
};

template <typename... AllowedTypes, typename Fn>
bool one_arg_limited_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 1)
    {
        throw std::invalid_argument("Requires 1 argument");
    }
    stack_entry& a = calc.stack.front();
    std::variant<AllowedTypes...> la;
    if (!variant_holds_type<AllowedTypes...>(a.value()))
    {
        throw std::invalid_argument("Invalid argument type");
    }
    if (!reduce(a.value(), la)())
    {
        throw std::runtime_error("Argument failed to reduce after conversion");
    }

    auto [cv, nu] = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, la);

    calc.stack.pop_front();
    calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                             calc.config.fixed_bits, a.precision,
                             calc.config.is_signed, calc.flags);
    return true;
}

template <typename... AllowedTypes, typename Fn>
bool one_arg_limited_multi_return_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 1)
    {
        throw std::invalid_argument("Requires 1 argument");
    }
    stack_entry& a = calc.stack.front();
    std::variant<AllowedTypes...> la;
    if (!variant_holds_type<AllowedTypes...>(a.value()))
    {
        throw std::invalid_argument("Invalid argument type");
    }
    if (!reduce(a.value(), la)())
    {
        throw std::runtime_error("Argument failed to reduce after conversion");
    }

    std::vector<std::tuple<numeric, units::unit>> values = std::visit(
        [&fn, ua{a.unit()}](const auto& a) { return fn(a, ua); }, la);

    calc.stack.pop_front();
    for (auto& [cv, nu] : values)
    {
        calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                                 calc.config.fixed_bits, a.precision,
                                 calc.config.is_signed, calc.flags);
    }
    return true;
}

template <typename Fn>
bool two_arg_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        throw std::invalid_argument("Requires 2 arguments");
    }
    stack_entry& a = calc.stack[1];
    stack_entry& b = calc.stack[0];

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()), calc.flags);
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
        std::min(a.precision, b.precision), calc.config.is_signed, calc.flags);
    return true;
}

template <typename Fn>
bool two_arg_uconv_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        throw std::invalid_argument("Requires 2 arguments");
    }
    stack_entry& a = calc.stack[1];
    stack_entry& b = calc.stack[0];

    if (a.unit() != b.unit())
    {
        if (a.unit().compat(b.unit()))
        {
            // convert b to a units
            b.value(units::convert(b.value(), b.unit(), a.unit()), calc.flags);
        }
        else if (units::are_temp_units(a.unit(), b.unit()))
        {
            b.value(std::visit(
                        [ub{b.unit()}, ua{a.unit()}](const auto& v) {
                            return units::scale_temp_units(v, ub, ua);
                        },
                        b.value()),
                    calc.flags);
            b.unit(a.unit());
        }
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
        std::min(a.precision, b.precision), calc.config.is_signed, calc.flags);
    return true;
}

template <typename It, typename Ot, typename Lt>
struct two_arg_conv
{
    template <typename Fn>
    bool op(Calculator&, const Fn&)
    {
        return false;
    }
};

template <typename... Itypes, typename... Otypes, typename... Ltypes>
struct two_arg_conv<ITypes<Itypes...>, OTypes<Otypes...>, LTypes<Ltypes...>>
{
    template <typename Fn>
    static bool op(Calculator& calc, const Fn& fn)
    {
        if (calc.stack.size() < 2)
        {
            throw std::invalid_argument("Requires 2 arguments");
        }
        stack_entry& a = calc.stack[1];
        stack_entry& b = calc.stack[0];

        lg::debug("a: ({} (type {}))\n", a.value(), DEBUG_TYPE(a.value()));
        lg::debug("b: ({} (type {}))\n", b.value(), DEBUG_TYPE(b.value()));
        if (a.unit() != b.unit() && a.unit().compat(b.unit()))
        {
            // convert b to a units
            b.value(units::convert(b.value(), b.unit(), a.unit()), calc.flags);
        }
        lg::debug("a: ({} (type {}))\n", a.value(), DEBUG_TYPE(a.value()));
        lg::debug("b: ({} (type {}))\n", b.value(), DEBUG_TYPE(b.value()));
        numeric ca = a.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        numeric cb = b.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
        std::variant<Ltypes...> lca;
        std::variant<Ltypes...> lcb;
        if (!reduce(ca, lca)() || !reduce(cb, lcb)())
        {
            throw std::runtime_error(
                "Argument(s) failed to reduce after conversion");
        }
        lg::debug("a: ({} (type {}))\n", lca, DEBUG_TYPE(lca));
        lg::debug("b: ({} (type {}))\n", lcb, DEBUG_TYPE(lcb));
        auto [cv, nu] = std::visit(
            [&fn, ua{a.unit()}, ub{b.unit()}](const auto& a, const auto& b) {
                lg::debug("a: ({} (type {}))\n", a, DEBUG_TYPE(a));
                lg::debug("b: ({} (type {}))\n", b, DEBUG_TYPE(b));
                return fn(a, b, ua, ub);
            },
            lca, lcb);

        calc.stack.pop_front();
        calc.stack.pop_front();

        calc.stack.emplace_front(std::move(cv), nu, calc.config.base,
                                 calc.config.fixed_bits,
                                 std::min(a.precision, b.precision),
                                 calc.config.is_signed, calc.flags);
        return true;
    }
};

template <typename... AllowedTypes, typename Fn>
bool two_arg_limited_op(Calculator& calc, const Fn& fn)
{
    if (calc.stack.size() < 2)
    {
        throw std::invalid_argument("Requires 2 arguments");
    }

    stack_entry& a = calc.stack[1];
    stack_entry& b = calc.stack[0];

    if (!variant_holds_type<AllowedTypes...>(a.value()) ||
        !variant_holds_type<AllowedTypes...>(b.value()))
    {
        throw std::invalid_argument("Invalid argument type");
    }
    std::variant<AllowedTypes...> la;
    std::variant<AllowedTypes...> lb;
    if (!reduce(a.value(), la)() || !reduce(b.value(), lb)())
    {
        throw std::runtime_error(
            "Argument(s) failed to reduce after conversion");
    }

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()), calc.flags);
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
        std::min(a.precision, b.precision), calc.config.is_signed, calc.flags);
    return true;
}

template <typename... AllowedTypes, typename Fn>
bool three_arg_limited_op(Calculator& calc, const Fn& fn,
                          const std::tuple<AllowedTypes...>& /*allowed types*/)
{
    if (calc.stack.size() < 3)
    {
        throw std::invalid_argument("Requires 3 arguments");
    }

    stack_entry& a = calc.stack[2];
    stack_entry& b = calc.stack[1];
    stack_entry& c = calc.stack[0];

    if (a.unit().compat(b.unit()))
    {
        // convert b to a units
        b.value(units::convert(b.value(), b.unit(), a.unit()), calc.flags);
    }
    if (a.unit().compat(c.unit()))
    {
        // convert c to a units
        c.value(units::convert(c.value(), c.unit(), a.unit()), calc.flags);
    }
    if (!variant_holds_type<AllowedTypes...>(a.value()) ||
        !variant_holds_type<AllowedTypes...>(b.value()) ||
        !variant_holds_type<AllowedTypes...>(c.value()))
    {
        throw std::invalid_argument("Invalid argument type");
    }
    std::variant<AllowedTypes...> la;
    std::variant<AllowedTypes...> lb;
    std::variant<AllowedTypes...> lc;
    if (!reduce(a.value(), la)() || !reduce(b.value(), lb)() ||
        !reduce(c.value(), lc)())
    {
        throw std::runtime_error(
            "Argument(s) failed to reduce after conversion");
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
                             calc.config.is_signed, calc.flags);
    return true;
}

template <typename It, typename Ot, typename Lt>
struct three_arg_conv
{
    template <typename Fn>
    static bool op(Calculator&, const Fn&)
    {
        return false;
    }
};
template <typename... Itypes, typename... Otypes, typename... Ltypes>
struct three_arg_conv<ITypes<Itypes...>, OTypes<Otypes...>, LTypes<Ltypes...>>
{
    template <typename Fn>
    static bool op(Calculator& calc, const Fn& fn)
    {
        if (calc.stack.size() < 3)
        {
            throw std::invalid_argument("Requires 3 arguments");
        }
        stack_entry& a = calc.stack[2];
        stack_entry& b = calc.stack[1];
        stack_entry& c = calc.stack[0];

        if ((a.unit() != units::unit()) || (b.unit() != units::unit()) ||
            (c.unit() != units::unit()))
        {
            throw units_prohibited();
        }

        numeric ca = a.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        numeric cb = b.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
        numeric cc = c.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cc);
        std::variant<Ltypes...> lca;
        std::variant<Ltypes...> lcb;
        std::variant<Ltypes...> lcc;
        if (!reduce(ca, lca)() || !reduce(cb, lcb)() || !reduce(cc, lcc)())
        {
            throw std::runtime_error(
                "Argument(s) failed to reduce after conversion");
        }
        auto cv = std::visit([&fn](const auto& a, const auto& b,
                                   const auto& c) { return fn(a, b, c); },
                             lca, lcb, lcc);

        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.pop_front();

        calc.stack.emplace_front(std::move(cv), units::unit(), calc.config.base,
                                 calc.config.fixed_bits,
                                 std::min(a.precision, b.precision),
                                 calc.config.is_signed, calc.flags);
        return true;
    }
};

template <typename It, typename Ot, typename Lt>
struct four_arg_conv
{
    template <typename Fn>
    static bool op(Calculator&, const Fn&)
    {
        return false;
    }
};
template <typename... Itypes, typename... Otypes, typename... Ltypes>
struct four_arg_conv<ITypes<Itypes...>, OTypes<Otypes...>, LTypes<Ltypes...>>
{
    template <typename Fn>
    static bool op(Calculator& calc, const Fn& fn)
    {
        if (calc.stack.size() < 4)
        {
            throw std::invalid_argument("Requires 4 arguments");
        }
        stack_entry& a = calc.stack[3];
        stack_entry& b = calc.stack[2];
        stack_entry& c = calc.stack[1];
        stack_entry& d = calc.stack[0];

        if ((a.unit() != units::unit()) || (b.unit() != units::unit()) ||
            (c.unit() != units::unit()) || (d.unit() != units::unit()))
        {
            throw units_prohibited();
        }

        numeric ca = a.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        numeric cb = b.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
        numeric cc = c.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cc);
        numeric cd = d.value();
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cd);
        std::variant<Ltypes...> lca;
        std::variant<Ltypes...> lcb;
        std::variant<Ltypes...> lcc;
        std::variant<Ltypes...> lcd;
        if (!reduce(ca, lca)() || !reduce(cb, lcb)() || !reduce(cc, lcc)() ||
            !reduce(cd, lcd)())
        {
            throw std::runtime_error(
                "Argument(s) failed to reduce after conversion");
        }
        auto cv = std::visit([&fn](const auto& a, const auto& b, const auto& c,
                                   const auto& d) { return fn(a, b, c, d); },
                             lca, lcb, lcc, lcd);

        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.pop_front();

        calc.stack.emplace_front(std::move(cv), units::unit(), calc.config.base,
                                 calc.config.fixed_bits,
                                 std::min(a.precision, b.precision),
                                 calc.config.is_signed, calc.flags);
        return true;
    }
};

static const mpf one_eighty_f{180.0l};
static const mpf two_hundred_f{200.0l};

template <typename Fn>
std::tuple<numeric, units::unit> scaled_trig_op(Calculator& calc, auto a,
                                                const Fn& fn)
{
    if constexpr (same_type_v<decltype(a), symbolic>)
    {
        return {fn(a), units::unit()};
    }
    else
    {
        if (calc.config.angle_mode == Calculator::e_angle_mode::degrees)
        {
            a *= boost::math::constants::pi<mpf>() / one_eighty_f;
        }
        else if (calc.config.angle_mode == Calculator::e_angle_mode::gradians)
        {
            a *= boost::math::constants::pi<mpf>() / two_hundred_f;
        }
        return {fn(a), units::unit()};
    }
}

template <typename Fn>
std::tuple<numeric, units::unit> scaled_trig_op_inv(Calculator& calc,
                                                    const auto& a, const Fn& fn)
{
    auto b = fn(a);
    if constexpr (same_type_v<decltype(a), symbolic>)
    {
        return {b, units::unit()};
    }
    else
    {
        if (calc.config.angle_mode == Calculator::e_angle_mode::degrees)
        {
            b *= one_eighty_f / boost::math::constants::pi<mpf>();
        }
        else if (calc.config.angle_mode == Calculator::e_angle_mode::gradians)
        {
            b *= two_hundred_f / boost::math::constants::pi<mpf>();
        }
        return {b, units::unit()};
    }
}

template <typename Fn>
std::tuple<numeric, units::unit>
    scaled_trig_two_arg_op_inv(Calculator& calc, const auto& a, const auto& b,
                               const Fn& fn)
{
    auto c = fn(a, b);
    if (calc.config.angle_mode == Calculator::e_angle_mode::degrees)
    {
        c *= one_eighty_f / boost::math::constants::pi<mpf>();
    }
    else if (calc.config.angle_mode == Calculator::e_angle_mode::gradians)
    {
        c *= two_hundred_f / boost::math::constants::pi<mpf>();
    }
    return {c, units::unit()};
}
} // namespace smrty
