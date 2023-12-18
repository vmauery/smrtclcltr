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
