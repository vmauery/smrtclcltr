/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <debug.hpp>
#include <format>
#include <matrix.hpp>
#include <type_helpers.hpp>
#include <variant>

#ifdef USE_BASIC_TYPES
#include <numeric_basic_types.hpp>
#else
#include <numeric_boost_types.hpp>
#endif

template <class T>
struct is_integer
    : std::integral_constant<
          bool, std::is_integral<T>::value ||
                    std::is_same<mpz, std::remove_cvref_t<T>>::value>
{
};

template <class T>
struct is_float
    : std::integral_constant<
          bool, std::is_floating_point<T>::value ||
                    std::is_same<mpf, std::remove_cvref_t<T>>::value>
{
};

template <class T>
struct is_rational
    : std::integral_constant<bool,
                             std::is_same<mpq, std::remove_cvref_t<T>>::value>
{
};

template <class T>
struct is_mathy
    : std::integral_constant<bool, is_integer<T>::value || is_float<T>::value ||
                                       is_rational<T>::value>
{
};
template <class T>
inline constexpr bool is_mathy_v = is_mathy<T>::value;

// exact things lose no precision on conversions
template <class T>
struct is_exact
    : std::integral_constant<
          bool, is_integer<T>::value ||
                    std::is_same<mpf, std::remove_cvref_t<T>>::value>
{
};
template <class T>
inline constexpr bool is_exact_v = is_exact<T>::value;

#include <time.hpp>

using time_ = basic_time<mpq>;
using matrix = basic_matrix<mpq>;

using numeric = std::variant<mpz, mpf, mpc, mpq, matrix, time_>;

static constexpr auto numeric_types = std::to_array<const char*>({
    "mpz",
    "mpf",
    "mpc",
    "mpq",
    "matrix",
    "time",
});

numeric reduce_numeric(const numeric& n, int precision = 2);
mpz make_fixed(const mpz& v, int bits, bool is_signed);
mpq parse_mpf(std::string_view s);
std::optional<time_> parse_time(std::string_view s);
matrix parse_matrix(std::string_view s);

static inline bool operator<(const numeric& a, const numeric& b)
{
    return std::visit(
        [](const auto& a, const auto& b) {
            if constexpr (std::is_same_v<decltype(a), decltype(b)>)
            {
                return a < b;
            }
            else if constexpr (std::is_same_v<decltype(a), mpc> ||
                               std::is_same_v<decltype(b), mpc>)
            {
                return static_cast<mpc>(a) < static_cast<mpc>(b);
            }
            else if constexpr (std::is_same_v<decltype(a), mpf> ||
                               std::is_same_v<decltype(b), mpf>)
            {
                return static_cast<mpf>(a) < static_cast<mpf>(b);
            }
            else if constexpr (std::is_same_v<decltype(a), mpq> ||
                               std::is_same_v<decltype(b), mpq>)
            {
                return static_cast<mpq>(a) < static_cast<mpq>(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
            return false;
        },
        a, b);
}

template <typename TypeOut, typename TypeIn>
TypeOut coerce_variant(const TypeIn& in)
{
    if constexpr (std::is_same_v<TypeOut, mpz>)
    {
        return static_cast<mpz>(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpf>)
    {
        return static_cast<mpf>(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpc>)
    {
        return static_cast<mpc>(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpq>)
    {
        return static_cast<mpq>(in);
    }
    static_assert("incorrect argument to coerce_variant");
    throw std::invalid_argument("incorrect argument to coerce_variant");
}

/* scaler ops with time_ */
template <typename S, std::enable_if_t<is_mathy_v<S>, bool> = true>
static inline time_ operator+(const S& s, const time_& t)
{
    return time_(t.value + static_cast<mpq>(s), t.absolute);
}
template <>
inline time_ operator+(const mpf& s, const time_& t)
{
    return time_(t.value + make_quotient(s), t.absolute);
}
template <typename S, std::enable_if_t<is_mathy_v<S>, bool> = true>
static inline time_ operator-(const S& s, const time_& t)
{
    return time_(t.value - static_cast<mpq>(s), t.absolute);
}
template <>
inline time_ operator-(const mpf& s, const time_& t)
{
    return time_(t.value - make_quotient(s), t.absolute);
}
template <typename S, std::enable_if_t<is_mathy_v<S>, bool> = true>
static inline time_ operator*(const S& s, const time_& t)
{
    // mult on absolute time makes it a duration
    return time_(t.value * static_cast<mpq>(s), false);
}
template <>
inline time_ operator*(const mpf& s, const time_& t)
{
    // mult on absolute time makes it a duration
    return time_(t.value * make_quotient(s), t.absolute);
}
template <typename S, std::enable_if_t<is_mathy_v<S>, bool> = true>
static inline time_ operator/(const S&, const time_&)
{
    throw std::invalid_argument("Scalar division by time is not allowed");
}

// OPERATORS between numerics
// ADD
template <typename T,
          std::enable_if_t<!std::is_same<T, matrix>::value, bool> = true>
static inline matrix operator+(const matrix&, const T&)
{
    throw std::invalid_argument("Scalar addition with a matrix is not allowed");
}
template <typename T,
          std::enable_if_t<!std::is_same<T, matrix>::value, bool> = true>
static inline matrix operator+(const T&, const matrix&)
{
    throw std::invalid_argument("Scalar addition with a matrix is not allowed");
}
static inline time_ operator+(const mpq& q, const time_& t)
{
    time_ nt = t;
    nt.value += q;
    return nt;
}
static inline time_ operator+(const mpc&, const time_&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline time_ operator+(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value += q;
    return nt;
}
static inline time_ operator+(const time_&, const mpc&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline mpf operator+(const mpf& l, const mpz& r)
{
    return l + mpf(r);
}
static inline mpf operator+(const mpz& l, const mpf& r)
{
    return mpf(l) + r;
}
static inline mpc operator+(const mpf& l, const mpc& r)
{
    return r + static_cast<mpc>(l);
}
static inline mpf operator+(const mpf& l, const mpq& r)
{
    return l + static_cast<mpf>(r);
}
static inline mpf operator+(const mpq& l, const mpf& r)
{
    return static_cast<mpf>(l) + r;
}
static inline mpq operator+(const mpq& l, const mpz& r)
{
    return l + mpq(r);
}
static inline mpq operator+(const mpz& l, const mpq& r)
{
    return mpq(l) + r;
}
static inline mpc operator+(const mpc& c, const mpz& z)
{
    return c + static_cast<mpc>(z);
}
static inline mpc operator+(const mpc& c, const mpf& f)
{
    return c + static_cast<mpc>(f);
}
static inline mpc operator+(const mpz& z, const mpc& c)
{
    return static_cast<mpc>(z) + c;
}
static inline mpc operator+(const mpq& q, const mpc& c)
{
    return static_cast<mpc>(q) + c;
}
static inline mpc operator+(const mpc& c, const mpq& q)
{
    return c + static_cast<mpc>(q);
}

// SUBTRACT
template <typename T,
          std::enable_if_t<!std::is_same<T, matrix>::value, bool> = true>
static inline matrix operator-(const matrix&, const T&)
{
    throw std::invalid_argument(
        "Scalar subtraction with a matrix is not allowed");
}
template <typename T,
          std::enable_if_t<!std::is_same<T, matrix>::value, bool> = true>
static inline matrix operator-(const T&, const matrix&)
{
    throw std::invalid_argument(
        "Scalar subtraction with a matrix is not allowed");
}
static inline time_ operator-(const mpq& q, const time_& t)
{
    time_ nt = t;
    nt.value = q - nt.value;
    return nt;
}
static inline time_ operator-(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value -= q;
    return nt;
}
static inline time_ operator-(const mpc&, const time_&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline time_ operator-(const time_&, const mpc&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline mpc operator-(const mpc& c, const mpz& z)
{
    return c - static_cast<mpc>(z);
}
static inline mpq operator-(const mpz& z, const mpq& q)
{
    return static_cast<mpq>(z) - q;
}
static inline mpf operator-(const mpz& z, const mpf& f)
{
    return static_cast<mpf>(z) - f;
}
static inline mpc operator-(const mpz& z, const mpc& c)
{
    return static_cast<mpc>(z) - c;
}
static inline mpf operator-(const mpf& f, const mpq& q)
{
    return f - static_cast<mpf>(q);
}
static inline mpf operator-(const mpq& q, const mpf& f)
{
    return static_cast<mpf>(q) - f;
}
static inline mpc operator-(const mpf& f, const mpc& c)
{
    return static_cast<mpc>(f) - c;
}
static inline mpc operator-(const mpc& c, const mpf& f)
{
    return c - static_cast<mpc>(f);
}
static inline mpc operator-(const mpq& q, const mpc& c)
{
    return static_cast<mpc>(q) - c;
}
static inline mpc operator-(const mpc& c, const mpq& q)
{
    return c - static_cast<mpc>(q);
}

// MULTIPLY
template <typename T, std::enable_if_t<!std::is_same<T, matrix>::value &&
                                           !std::is_same<T, mpq>::value &&
                                           !std::is_same<T, mpz>::value,
                                       bool> = true>
static inline matrix operator*(const matrix&, const T&)
{
    throw std::invalid_argument(
        "Scalar multiplication with a matrix is not allowed with this type");
}
template <typename T, std::enable_if_t<!std::is_same<T, matrix>::value &&
                                           !std::is_same<T, mpq>::value &&
                                           !std::is_same<T, mpz>::value,
                                       bool> = true>
static inline matrix operator*(const T&, const matrix&)
{
    throw std::invalid_argument(
        "Scalar multiplication with a matrix is not allowed with this type");
}
static inline matrix operator*(const mpz& z, const matrix& m)
{
    return m * static_cast<mpq>(z);
}
static inline matrix operator*(const mpq& q, const matrix& m)
{
    return m * q;
}
static inline mpq operator*(const mpz& z, const mpq& q)
{
    return static_cast<mpq>(z) * q;
}
static inline mpq operator*(const mpq& q, const mpz& z)
{
    return q * static_cast<mpq>(z);
}
static inline mpf operator*(const mpq& q, const mpf& f)
{
    return static_cast<mpf>(q) * f;
}
static inline mpc operator*(const mpq& q, const mpc& c)
{
    return static_cast<mpc>(q) * c;
}
static inline mpf operator*(const mpf& f, const mpq& q)
{
    return f * static_cast<mpf>(q);
}
static inline mpc operator*(const mpf& f, const mpc& c)
{
    return static_cast<mpc>(f) * c;
}
static inline mpc operator*(const mpc& c, const mpq& q)
{
    return c * static_cast<mpc>(q);
}
static inline mpc operator*(const mpc& c, const mpf& f)
{
    return c * static_cast<mpc>(f);
}
static inline mpc operator*(const mpc& c, const mpz& z)
{
    return c * static_cast<mpc>(z);
}
static inline mpc operator*(const mpz& z, const mpc& c)
{
    return static_cast<mpc>(z) * c;
}
static inline time_ operator*(const mpq& q, const time_& t)
{
    time_ nt = t;
    nt.value *= q;
    return nt;
}
static inline time_ operator*(const mpc&, const time_&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline time_ operator*(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value *= q;
    return nt;
}
static inline time_ operator*(const time_&, const mpc&)
{
    throw std::invalid_argument("complex time not allowed");
}

// DIVIDE
static inline matrix operator/(const mpq& q, const matrix& m)
{
    return m.inv() * q;
}
static inline time_ operator/(const mpq&, const time_&)
{
    throw std::invalid_argument("inverse time not allowed");
}
static inline time_ operator/(const mpc&, const time_&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline time_ operator/(const time_&, const mpc&)
{
    throw std::invalid_argument("complex time not allowed");
}
static inline time_ operator/(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value /= q;
    return nt;
}
static inline mpq operator/(const mpz& z, const mpq& q)
{
    return static_cast<mpq>(z) / q;
}
static inline mpc operator/(const mpc& c, const mpq& q)
{
    return c / static_cast<mpc>(q);
}
static inline mpc operator/(const mpq& q, const mpc& c)
{
    return static_cast<mpc>(q) / c;
}
static inline mpc operator/(const mpz& z, const mpc& c)
{
    return static_cast<mpc>(z) / c;
}
static inline mpc operator/(const mpc& c, const mpz& z)
{
    return c / static_cast<mpc>(z);
}
static inline mpc operator/(const mpc& c, const mpf& f)
{
    return c / static_cast<mpc>(f);
}
static inline mpf operator/(const mpq& q, const mpf& f)
{
    return static_cast<mpf>(q) / f;
}
static inline mpf operator/(const mpf& f, const mpq& q)
{
    return f / static_cast<mpf>(q);
}
static inline mpc operator/(const mpf& f, const mpc& c)
{
    return static_cast<mpc>(f) / c;
}

// MODULAR DIVIDE
// z q f
static inline mpq operator%(const mpz& z, const mpq& q)
{
    return static_cast<mpq>(z) % q;
}
static inline mpq operator%(const mpq& q, const mpz& z)
{
    return q % static_cast<mpq>(z);
}
static inline mpf operator%(const mpz& z, const mpf& f)
{
    return static_cast<mpf>(z) % f;
}
static inline mpf operator%(const mpf& f, const mpz& z)
{
    return f % static_cast<mpf>(z);
}
static inline mpf operator%(const mpq& q, const mpf& f)
{
    return static_cast<mpf>(q) % f;
}
static inline mpf operator%(const mpf& f, const mpq& q)
{
    return f % static_cast<mpf>(q);
}

std::string mpz_to_bin_string(const mpz& v, std::streamsize width);

#define NEED_NUMERIC_TYPE_FORMATTERS 1
#include <numeric_format.hpp>
