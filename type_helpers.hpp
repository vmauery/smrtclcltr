/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <type_traits>
#include <variant>

template <class T, class O>
struct same_type
    : std::integral_constant<bool, std::is_same<std::remove_cvref_t<T>,
                                                std::remove_cvref_t<O>>::value>
{
};
template <class T, class O>
inline constexpr bool same_type_v = same_type<T, O>::value;

template <class T>
concept integer = std::is_integral<T>::value;

template <class T>
concept floating = std::is_floating_point<T>::value;

template <typename T>
struct variant0_or_single
{
    using type = T;
};
template <typename T, typename... Types>
struct variant0_or_single<std::variant<T, Types...>>
{
    using type = T;
};

template <typename T>
struct is_variant : std::false_type
{
};
template <typename... Args>
struct is_variant<std::variant<Args...>> : std::true_type
{
};
template <typename... Args>
inline constexpr bool is_variant_v = is_variant<Args...>::value;

template <class T, class U>
struct is_one_of;
template <class T, class... Ts>
struct is_one_of<T, std::variant<Ts...>>
    : public std::disjunction<same_type<T, Ts>...>
{
};
template <typename T, typename... Args>
inline constexpr bool is_one_of_v = is_one_of<T, Args...>::value;

template <class... Types>
struct variant_cast_helper
{
    std::variant<Types...> v;

    template <class... OTypes>
    operator std::variant<OTypes...>() const
    {
        return std::visit(
            [](auto&& arg) -> std::variant<OTypes...> { return arg; }, v);
    }
};
template <class... Types>
auto variant_cast(const std::variant<Types...>& v)
    -> variant_cast_helper<Types...>
{
    return {v};
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

    const Vin& _vin;
    Vout& _vout;

    reduce(const Vin& vin, Vout& vout) : _vin(vin), _vout(vout)
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

template <typename...>
struct ITypes
{
};

template <typename...>
struct OTypes
{
};

template <typename...>
struct LTypes
{
};

template <std::size_t N, class... Args>
using list_type_t = std::tuple_element_t<N, std::tuple<Args...>>;
