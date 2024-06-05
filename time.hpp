/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <chrono>
#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>

template <typename T>
struct basic_time
{
    using value_type = T;

    basic_time() : value(0, 1), absolute(false)
    {
    }

#ifdef USE_BASIC_TYPES
    // Basic types can only handle microsecond resolution
    template <typename Rep, typename Period>
    explicit basic_time(const std::chrono::duration<Rep, Period>& d,
                        bool absolute = false) :
        value(std::chrono::duration_cast<std::chrono::microseconds>(d).count(),
              1'000'000ull),
        absolute(absolute)
    {
        lg::debug(
            "time({}ms, {})\n",
            std::chrono::duration_cast<std::chrono::microseconds>(d).count(),
            absolute);
    }
#else  // !USE_BASIC_TYPES
    // Boost types can handle nanosecond resolution
    template <typename Rep, typename Period>
    explicit basic_time(const std::chrono::duration<Rep, Period>& d,
                        bool absolute = false) :
        value(std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(),
              1'000'000'000ull),
        absolute(absolute)
    {
        lg::debug(
            "time({}ns, {})\n",
            std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(),
            absolute);
    }
#endif // USE_BASIC_TYPES

    template <typename Clock, typename Duration>
    explicit basic_time(const std::chrono::time_point<Clock, Duration>& tp) :
        basic_time(tp.time_since_epoch(), true)
    {
    }

    // parse basic_time from string
    explicit basic_time(std::string_view t) :
        value(make_quotient(t)), absolute(false)
    {
    }

    // numbers to basic_time (default to absolute times)
    template <integer I>
    basic_time(const I& i, bool absolute) :
        value(static_cast<T>(i)), absolute(absolute)
    {
    }
    basic_time(const T& t, bool absolute) : value(t), absolute(absolute)
    {
    }
    template <floating F>
    basic_time(const F& f, bool absolute) :
        value(static_cast<T>(f)), absolute(absolute)
    {
    }

    bool operator==(const basic_time<T>& t) const
    {
        return absolute == t.absolute && value == t.value;
    }
    bool operator!=(const basic_time<T>& t) const
    {
        return !(absolute == t.absolute && value == t.value);
    }
    bool operator<(const basic_time<T>& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value < t.value;
    }
    bool operator>(const basic_time<T>& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value > t.value;
    }
    bool operator<=(const basic_time<T>& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value <= t.value;
    }
    bool operator>=(const basic_time<T>& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value >= t.value;
    }
    /* ops with other times */
    basic_time<T> operator+(const basic_time<T>& t) const
    {
        if (absolute && t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform arithmetic with two absolute times");
        }
        return basic_time<T>(value + t.value, absolute | t.absolute);
    }
    basic_time<T> operator-(const basic_time<T>& t) const
    {
        return basic_time<T>(value - t.value, absolute ^ t.absolute);
    }
    basic_time<T> operator*(const basic_time<T>&) const
    {
        throw std::invalid_argument("cannot perform multiplication with times");
    }
    T operator/(const basic_time<T>& t) const
    {
        if (absolute || t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform division with absolute times");
        }
        return value / t.value;
    }
    basic_time<T>& operator+=(const basic_time<T>& t)
    {
        if (absolute && t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform arithmetic with two absolute times");
        }
        value += t.value;
        absolute |= t.absolute;
        return *this;
    }
    basic_time<T>& operator-=(const basic_time<T>& t)
    {
        value -= t.value;
        absolute ^= t.absolute;
        return *this;
    }
    basic_time<T>& operator*=(const basic_time<T>& t)
    {
        if (absolute)
        {
            throw std::invalid_argument(
                "cannot perform arithmetic with an absolute time");
        }
        value -= t.value;
        absolute ^= t.absolute;
        return *this;
    }
    basic_time<T>& operator/=(const basic_time<T>& t)
    {
        if (absolute)
        {
            throw std::invalid_argument(
                "cannot perform arithmetic with an absolute time");
        }
        value /= t.value;
        absolute ^= t.absolute;
        return *this;
    }

    /* ops with scalers */
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T> operator+(const I& t) const
    {
        return basic_time<T>(value + static_cast<T>(t), absolute);
    }
    basic_time<T> operator+(const mpf& t) const
    {
        return basic_time<T>(value + make_quotient(t), absolute);
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T> operator-(const I& t) const
    {
        return basic_time<T>(value - static_cast<T>(t), absolute);
    }
    basic_time<T> operator-(const mpf& t) const
    {
        return basic_time<T>(value - make_quotient(t), absolute);
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T> operator*(const I& t) const
    {
        // mult on absolute time makes it a duration
        return basic_time<T>(value * static_cast<T>(t), false);
    }
    basic_time<T> operator*(const mpf& t) const
    {
        return basic_time<T>(value * make_quotient(t), false);
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T> operator/(const I& t) const
    {
        // div on absolute time makes it a duration
        return basic_time<T>(value / static_cast<T>(t), false);
    }
    basic_time<T> operator/(const mpf& t) const
    {
        // div on absolute time makes it a duration
        return basic_time<T>(value / make_quotient(t), false);
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T>& operator+=(const I& t)
    {
        if constexpr (std::is_same_v<mpf, std::remove_cvref<I>>)
        {
            value += make_quotient(t);
        }
        else
        {
            value += static_cast<T>(t);
        }
        return *this;
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T>& operator-=(const I& t)
    {
        if constexpr (std::is_same_v<mpf, std::remove_cvref<I>>)
        {
            value -= make_quotient(t);
        }
        else
        {
            value -= static_cast<T>(t);
        }
        return *this;
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T>& operator*=(const I& t)
    {
        // mult on absolute time makes it a duration
        if constexpr (std::is_same_v<mpf, std::remove_cvref<I>>)
        {
            value *= make_quotient(t);
        }
        else
        {
            value *= static_cast<T>(t);
        }
        absolute = false;
        return *this;
    }
    template <typename I, std::enable_if_t<is_mathy_v<I>, bool> = true>
    basic_time<T>& operator/=(const I& t)
    {
        // div on absolute time makes it a duration
        if constexpr (std::is_same_v<mpf, std::remove_cvref<I>>)
        {
            value /= make_quotient(t);
        }
        else
        {
            value /= static_cast<T>(t);
        }
        absolute = false;
        return *this;
    }

    // used by std::format()
    std::string str() const
    {
        // lg::debug("str(): value={:q}\n", value);
        if (absolute)
        {
#ifdef USE_BASIC_TYPES
            long long nanos = static_cast<long long>(
                helper::numerator(value) *
                (one_million / helper::denominator(value)));
            std::chrono::duration d = std::chrono::microseconds(nanos);
#else  // !USE_BASIC_TYPES
            long long nanos = static_cast<long long>(
                helper::numerator(value) *
                (one_billion / helper::denominator(value)));
            std::chrono::duration d = std::chrono::nanoseconds(nanos);
#endif // USE_BASIC_TYPES
       // lg::debug("value={}, nanos={}\n", value, nanos);
            std::chrono::time_point<std::chrono::system_clock> tp(d);
            return std::format("{:%F %T}", tp);
            // const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
            // return std::strftime(std::localtime(&t_c), "%F %T");
        }

        // duration
        static const mpq one_day{86400, 1};
        static const mpq one_hour{3600, 1};
        static const mpq one_minute{60, 1};
        static const mpq one_second{1, 1};
        static const mpq one_ms{1, 1000};
        static const mpq one_us{1, 1000000};
        static const mpq one_ns{1, 1000000000};

        auto pval = abs_fn(value);
        if (pval >= one_day)
        {
            return std::format("{:f}d", value / one_day);
        }
        if (pval >= one_hour)
        {
            return std::format("{:f}h", value / one_hour);
        }
        if (pval >= one_minute)
        {
            return std::format("{:f}m", value / one_minute);
        }
        if (pval >= one_second)
        {
            return std::format("{:f}s", value / one_second);
        }
        if (pval >= one_ms)
        {
            return std::format("{:f}ms", value / one_ms);
        }
        if (pval >= one_us)
        {
            return std::format("{:f}us", value / one_us);
        }
        return std::format("{:f}ns", value / one_ns);
    }

    T value;
    bool absolute;
};

template <typename T>
struct std::formatter<basic_time<T>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const basic_time<T>& t,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto s = t.str();
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};
