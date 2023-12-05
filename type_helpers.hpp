/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <type_traits>

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
