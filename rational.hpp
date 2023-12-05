/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>

namespace smrty
{
constexpr mpz gcd(const mpz& l, const mpz& r);
constexpr mpz lcm(const mpz& l, const mpz& r);
} // namespace smrty

template <typename T>
struct rational
{
    T num = 0;
    T den = 1;

    /*
    rational(const rational&) = default;
    rational(rational&&) = default;
    rational& operator=(const rational&) = default;
    rational& operator=(rational&&) = default;
    */

    constexpr rational() : num(0), den(1)
    {
    }

    constexpr rational(const T& n) : num(n), den(1)
    {
    }

    template <integer I>
    constexpr rational(const I& n) : num(static_cast<T>(n)), den(1)
    {
    }

    constexpr rational(const T& n, const T& d) : num(n), den(d)
    {
        reduce();
    }

    explicit rational(std::string_view r)
    {
        auto d = r.find("/");
        if (d == std::string::npos)
        {
            num = T{r};
            den = 1;
        }
        else
        {
            auto numstr = r.substr(0, d);
            auto denstr = r.substr(d + 1);
            num = T{numstr};
            den = T{denstr};
        }
        reduce();
    }

    constexpr void reduce()
    {
        if (den < 0)
        {
            num *= -1;
            den *= -1;
        }
        T cf = smrty::gcd(num, den);
        if (cf > 1)
        {
            num /= cf;
            den /= cf;
        }
    }

    const T numerator() const
    {
        return num;
    }
    const T denominator() const
    {
        return den;
    }

    constexpr operator mpf() const
    {
        return mpf{static_cast<mpf::value_type>(num) /
                   static_cast<mpf::value_type>(den)};
    }

    auto operator<=>(const rational<T>& r) const
    {
        T lcm = smrty::lcm(den, r.den);
        return num * (lcm / den) <=> r.num * (lcm / r.den);
    }
    bool operator==(const rational<T>& r) const
    {
        return num == r.num && den == r.den;
    }

    /* ops with other ratios */
    rational<T> operator+(const rational<T>& r) const
    {
        T lcm = smrty::lcm(den, r.den);
        T nnum = num * (lcm / den) + r.num * (lcm / r.den);
        return rational<T>(nnum, lcm);
    }
    rational<T> operator-(const rational<T>& r) const
    {
        T lcm = smrty::lcm(den, r.den);
        T nnum = num * (lcm / den) - r.num * (lcm / r.den);
        return rational<T>(nnum, lcm);
    }
    rational<T> operator*(const rational<T>& r) const
    {
        T nnum = num * r.num;
        T lcm = den * r.den;
        return rational<T>(nnum, lcm);
    }
    rational<T> operator/(const rational<T>& r) const
    {
        T nnum = num * r.den;
        T lcm = den * r.num;
        return rational<T>(nnum, lcm);
    }
    rational<T> operator%(const rational<T>& b) const
    {
        // q = a/b -> c/d
        // r = a - b * (c - (c % d)) / d
        // equivalent to:
        // a b dup2 / split dup2 rolld swap % - roll / * -
        rational<T> q{num * b.den, den * b.num};
        return *this - b * rational<T>{q.num - q.num % q.den, q.den};
    }
    rational<T>& operator+=(const rational<T>& r)
    {
        T lcm = smrty::lcm(den, r.den);
        num = num * (lcm / den) + r.num * (lcm / r.den);
        den = lcm;
        reduce();
        return *this;
    }
    rational<T>& operator-=(const rational<T>& r)
    {
        T lcm = smrty::lcm(den, r.den);
        num = num * (lcm / den) - r.num * (lcm / r.den);
        den = lcm;
        reduce();
        return *this;
    }
    rational<T>& operator*=(const rational<T>& r)
    {
        num *= r.num;
        den *= r.den;
        reduce();
        return *this;
    }
    rational<T>& operator/=(const rational<T>& r)
    {
        num *= r.den;
        den *= r.num;
        reduce();
        return *this;
    }
    std::string str() const
    {
        return std::format("{}/{}", num, den);
    }
};

namespace helper
{
static inline mpz numerator(const mpq& q)
{
    return q.numerator();
}
static inline mpz denominator(const mpq& q)
{
    return q.denominator();
}
} // namespace helper
