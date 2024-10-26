/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>

template <integer T>
struct std::numeric_limits<checked_int<T>> : public std::numeric_limits<T>
{
};

template <integer T>
struct checked_int
{
    using value_type = T;

    value_type value;

    constexpr checked_int(const T& v) : value(v)
    {
    }
    template <integer I>
    constexpr checked_int(const I& v) : value(static_cast<T>(v))
    {
    }
    constexpr checked_int() : value(0)
    {
    }
    explicit checked_int(std::string_view r, int base = 10)
    {
        std::from_chars(r.begin(), r.end(), value, base);
        lg::debug("checked_int({}): value: {}\n", r, value);
    }
    constexpr bool operator==(const checked_int<T>& r) const
    {
        return value == r.value;
    }
    constexpr std::strong_ordering
        operator<=>(const checked_int<T>& r) const = default;

    template <integer I>
    constexpr bool operator==(I r) const
    {
        return value == r;
    }
    template <integer I>
    constexpr std::strong_ordering operator<=>(I r) const
    {
        return value <=> r;
    }

    /* automatic conversion to mpf */
    template <floating F>
    explicit constexpr operator checked_float<F>() const
    {
        return checked_float<F>{value};
    }

    template <floating F>
    explicit constexpr operator std::complex<F>() const
    {
        return std::complex<F>{static_cast<F>(value), static_cast<F>(0)};
    }

    /* automatic conversion to mpz */
    template <integer I>
    explicit operator I() const
    {
        return static_cast<I>(value);
    }

    /* ops with other ints */
    constexpr checked_int<T> operator+(const checked_int<T>& r) const
    {
        checked_int<T> result{};
        if (__builtin_add_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(
                std::format("overflow when adding {} and {}", value, r.value));
        }
        return result;
    }
    constexpr checked_int<T> operator-(T r) const
    {
        checked_int<T> result{};
        if (__builtin_sub_overflow(value, r, &result.value))
        {
            throw std::runtime_error(std::format(
                "overflow when subtracting {} and {}", value, r.value));
        }
        return result;
    }
    constexpr checked_int<T> operator-(const checked_int<T>& r) const
    {
        checked_int<T> result{};
        if (__builtin_sub_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(std::format(
                "overflow when subtracting {} and {}", value, r.value));
        }
        return result;
    }
    constexpr checked_int<T> operator-() const
    {
        checked_int<T> result{};
        if (__builtin_sub_overflow(0, value, &result.value))
        {
            throw std::runtime_error(
                std::format("overflow when negating {}", value));
        }
        return result;
    }
    constexpr checked_int<T> operator~() const
    {
        checked_int<T> result{~value};
        return result;
    }
    explicit constexpr operator bool() const
    {
        return value != 0;
    }
    constexpr checked_int<T> operator*(const checked_int<T>& r) const
    {
        checked_int<T> result{};
        if (__builtin_mul_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(std::format(
                "overflow when multiplying {} and {}", value, r.value));
        }
        return result;
    }
    constexpr checked_int<T> operator/(const checked_int<T>& r) const
    {
        if (r.value == 0)
        {
            throw std::runtime_error("divide by zero");
        }
        return checked_int<T>{value / r.value};
    }
    constexpr checked_int<T> operator%(const checked_int<T>& r) const
    {
        if (r.value == 0)
        {
            throw std::runtime_error("modular divide by zero");
        }
        return checked_int<T>{value % r.value};
    }
    template <integer I>
    constexpr checked_int<T> operator<<(I r) const
    {
        checked_int<T> result{value};
        while (r--)
        {
            result.value <<= 1;
            if (result.value < value)
            {
                throw std::runtime_error("overflow when left-shifting");
            }
        }
        return result;
    }
    constexpr checked_int<T> operator<<(const checked_int<T>& r) const
    {
        checked_int<T> result{value};
        T idx{r.value};
        while (idx--)
        {
            result.value <<= 1;
            if (result.value < value)
            {
                throw std::runtime_error("overflow when left-shifting");
            }
        }
        return result;
    }
    template <integer I>
    constexpr checked_int<T> operator>>(I r) const
    {
        return checked_int<T>{value >> r};
    }
    constexpr checked_int<T> operator>>(const checked_int<T>& r) const
    {
        return checked_int<T>{value >> r.value};
    }
    constexpr checked_int<T> operator|(const checked_int<T>& r) const
    {
        checked_int<T> result{value};
        result.value |= r.value;
        return result;
    }
    constexpr checked_int<T> operator&(const checked_int<T>& r) const
    {
        checked_int<T> result{value};
        result.value &= r.value;
        return result;
    }
    constexpr checked_int<T> operator^(const checked_int<T>& r) const
    {
        checked_int<T> result{value};
        result.value ^= r.value;
        return result;
    }
    checked_int<T>& operator++()
    {
        checked_int<T> result{};
        if (__builtin_add_overflow(value, 1, &result.value))
        {
            throw std::runtime_error("overflow when incrementing");
        }
        value = result.value;
        return *this;
    }
    checked_int<T> operator++(int)
    {
        checked_int<T> current{value};
        checked_int<T> result{};
        if (__builtin_add_overflow(value, 1, &result.value))
        {
            throw std::runtime_error("overflow when incrementing");
        }
        return current;
    }
    checked_int<T>& operator--()
    {
        checked_int<T> result{};
        if (__builtin_sub_overflow(value, 1, &result.value))
        {
            throw std::runtime_error("overflow when decrementing");
        }
        value = result.value;
        return *this;
    }
    checked_int<T> operator--(int)
    {
        checked_int<T> current{value};
        checked_int<T> result{};
        if (__builtin_sub_overflow(value, 1, &result.value))
        {
            throw std::runtime_error("overflow when decrementing");
        }
        return current;
    }
    checked_int<T>& operator+=(const checked_int<T>& r)
    {
        checked_int<T> result{};
        if (__builtin_add_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(
                std::format("overflow when adding {} and {}", value, r.value));
        }
        value = result.value;
        return *this;
    }
    checked_int<T>& operator-=(const checked_int<T>& r)
    {
        checked_int<T> result{};
        if (__builtin_sub_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(std::format(
                "overflow when subtracting {} and {}", value, r.value));
        }
        value = result.value;
        return *this;
    }
    checked_int<T>& operator*=(const checked_int<T>& r)
    {
        checked_int<T> result{};
        if (__builtin_mul_overflow(value, r.value, &result.value))
        {
            throw std::runtime_error(std::format(
                "overflow when multiplying {} and {}", value, r.value));
        }
        value = result.value;
        return *this;
    }
    checked_int<T>& operator/=(const checked_int<T>& r)
    {
        if (r.value == 0)
        {
            throw std::runtime_error("divide by zero");
        }
        value /= r.value;
        return *this;
    }
    checked_int<T>& operator%=(const checked_int<T>& r)
    {
        if (r.value == 0)
        {
            throw std::runtime_error("modular divide by zero");
        }
        value %= r.value;
        return *this;
    }
    checked_int<T>& operator<<=(unsigned int r)
    {
        T result{value};
        while (r--)
        {
            result <<= 1;
            if (result < value)
            {
                throw std::runtime_error("overflow when left-shifting");
            }
        }
        value = result;
        return *this;
    }
    checked_int<T>& operator>>=(unsigned int r)
    {
        value >>= r;
        return *this;
    }
    checked_int<T>& operator|=(const checked_int<T>& r)
    {
        value |= r.value;
        return *this;
    }
    checked_int<T>& operator&=(const checked_int<T>& r)
    {
        value &= r.value;
        return *this;
    }
    checked_int<T>& operator^=(const checked_int<T>& r)
    {
        value ^= r.value;
        return *this;
    }

    std::string str(std::streamsize width = 0,
                    std::ios_base::fmtflags f = {}) const
    {
        if (f & std::ios::oct)
        {
            width = static_cast<unsigned int>(
                std::ceil(static_cast<double>(width) / 3));
            if (f & std::ios::showbase)
            {
                return std::format("0{:0{}o}", value, width);
            }
            return std::format("{:0{}o}", value, width);
        }
        else if (f & std::ios::hex)
        {
            width = static_cast<unsigned int>(
                std::ceil(static_cast<double>(width) / 4));
            if (f & std::ios::showbase)
            {
                return std::format("0x{:0{}x}", value, width);
            }
            return std::format("{:0{}x}", value, width);
        }
        else // if (f & std::ios::dec)
        {
            static constexpr double log2_10 = 3.321928094887362347870319429;
            width = static_cast<unsigned int>(
                std::ceil(static_cast<double>(width) / log2_10));
            if (f & std::ios::showbase)
            {
                return std::format("0d{:0{}d}", value, width);
            }
            return std::format("{:0{}d}", value, width);
        }
    }
};
