/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>

template <floating T>
struct checked_float;

template <floating T>
struct std::numeric_limits<checked_float<T>> : public std::numeric_limits<T>
{
};

template <floating T>
struct checked_float
{
    using value_type = T;

    value_type value;

    template <floating F>
    constexpr checked_float(const F& v) : value(static_cast<T>(v))
    {
    }
    constexpr checked_float() : value(0)
    {
    }
    explicit checked_float(std::string_view r)
    {
        std::from_chars(r.begin(), r.end(), value);
        lg::debug("checked_float({}): value: {}\n", r, value);
    }
    constexpr std::partial_ordering
        operator<=>(const checked_float<T>& r) const = default;

    template <floating I>
    constexpr bool operator==(I r) const
    {
        return value == r;
    }
    constexpr bool operator==(const checked_float<T>& r) const
    {
        return value == r.value;
    }
    template <floating I>
    constexpr std::partial_ordering operator<=>(I r) const
    {
        return value <=> static_cast<T>(r);
    }

    /* cast conversion to mpc */
    explicit constexpr operator mpc() const
    {
        return mpc(value);
    }
    /* cast conversion to mpz */
    explicit constexpr operator mpz() const
    {
        return mpz(value);
    }
    /* cast conversion to mpq */
    /*
    explicit constexpr operator mpq() const
    {
        return make_quotient(value);
    }
    */
    constexpr operator T() const
    {
        return value;
    }

    template <integer I>
    explicit constexpr operator checked_int<I>() const
    {
        return checked_int<I>{static_cast<I>(value)};
    }

    /* ops with other ints */
    constexpr checked_float<T> operator+(const checked_float<T>& r) const
    {
        return checked_float<T>{value + r.value};
    }
    /*
    template <floating F>
    constexpr checked_float<T> operator-(F r) const
    {
        return checked_float<T> {value - r};
    }
    */
    constexpr checked_float<T> operator-(const checked_float<T>& r) const
    {
        return checked_float<T>{value - r.value};
    }
    constexpr checked_float<T> operator-() const
    {
        return checked_float<T>{-value};
    }
    constexpr checked_float<T> operator*(const checked_float<T>& r) const
    {
        return checked_float<T>{value * r.value};
    }
    constexpr checked_float<T> operator/(const checked_float<T>& r) const
    {
        if (r.value == 0)
        {
            throw std::runtime_error("divide by zero");
        }
        return checked_float<T>{value / r.value};
    }
    constexpr checked_float<T> operator%(const checked_float<T>& r) const
    {
        if (r.value == 0)
        {
            throw std::runtime_error("modular divide by zero");
        }
        return checked_float<T>{fmod(value, r.value)};
    }
    checked_float<T>& operator++()
    {
        value++;
        return *this;
    }
    checked_float<T> operator++(int)
    {
        checked_float<T> result{value};
        value++;
        return result;
    }
    checked_float<T>& operator--()
    {
        value--;
        return *this;
    }
    checked_float<T> operator--(int)
    {
        checked_float<T> result{value};
        value--;
        return result;
    }
    checked_float<T>& operator+=(const checked_float<T>& r)
    {
        value += r.value;
        return *this;
    }
    checked_float<T>& operator-=(const checked_float<T>& r)
    {
        value -= r.value;
        return *this;
    }
    checked_float<T>& operator*=(const checked_float<T>& r)
    {
        value *= r.value;
        return *this;
    }
    checked_float<T>& operator/=(const checked_float<T>& r)
    {
        if (r.value == 0)
        {
            throw std::runtime_error("divide by zero");
        }
        value /= r.value;
        return *this;
    }

    // need to handle flags for output modes
    std::string str(std::streamsize width = 0,
                    [[maybe_unused]] std::ios_base::fmtflags f = {}) const
    {
        return std::format("{:.{}}", value, width);
    }
};
