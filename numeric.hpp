/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <fmt/format.h>
#include <fmt/std.h>

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <debug.hpp>
#include <iostream>
#include <variant>

#ifdef USE_BASIC_TYPES
#include <cmath>
#include <numeric>
#else
#include <boost/integer/common_factor_rt.hpp>
#endif

extern int default_precision;

#if (USE_BOOST_CPP_BACKEND || USE_GMP_BACKEND || USE_MPFR_BACKEND)

static constexpr int builtin_default_precision = 50;
// yes, I know abritrary precision, but be reasonable, my dude!
static constexpr int max_precision = 1000000;

#define floor_fn boost::multiprecision::floor
#define ceil_fn boost::multiprecision::ceil
#define round_fn boost::multiprecision::round
#define lcm_fn boost::math::lcm
#define gcd_fn boost::math::gcd
#define pow_fn boost::multiprecision::pow

#ifdef USE_BOOST_CPP_BACKEND
#include <boost/serialization/nvp.hpp>
// stay
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#elif defined(USE_GMP_BACKEND)
#include <boost/multiprecision/gmp.hpp>
#elif defined(USE_MPFR_BACKEND)
#include <boost/multiprecision/mpfr.hpp>
#endif
#include <boost/multiprecision/complex_adaptor.hpp>
#include <boost/multiprecision/number.hpp>

#ifdef USE_BOOST_CPP_BACKEND
static constexpr const char MATH_BACKEND[] = "boost::multiprecision::cpp_*";
static constexpr size_t max_digits = 1000;
static constexpr int32_t exp_min = -262142;
static constexpr int32_t exp_max = 262143;
static constexpr size_t min_int_bits = 128;
static constexpr size_t max_int_bits = 0;

using exp_type = std::decay<decltype(exp_max)>::type;

using int_backend = boost::multiprecision::cpp_int_backend<
    min_int_bits, max_int_bits, boost::multiprecision::signed_magnitude,
    boost::multiprecision::unchecked>;

using float_backend = boost::multiprecision::backends::cpp_bin_float<
    max_digits, boost::multiprecision::backends::digit_base_10, void, exp_type,
    exp_min, exp_max>;

using complex_backend = boost::multiprecision::complex_adaptor<float_backend>;

using rational_backend =
    boost::multiprecision::backends::rational_adaptor<int_backend>;

static inline void set_default_precision(int iv)
{
    default_precision = iv;
}

#elif defined(USE_GMP_BACKEND)
static constexpr const char MATH_BACKEND[] = "boost::multiprecision::gmp_*";

using int_backend = boost::multiprecision::gmp_int;

using float_backend = boost::multiprecision::gmp_float<0>;

using complex_backend =
    boost::multiprecision::complex_adaptor<boost::multiprecision::gmp_float<0>>;

using rational_backend = boost::multiprecision::gmp_rational;

static inline void set_default_precision(int iv)
{
    default_precision = iv;
    boost::multiprecision::number<
        float_backend, boost::multiprecision::et_off>::default_precision(iv);
}

#elif defined(USE_MPFR_BACKEND)
static constexpr const char MATH_BACKEND[] = "boost::multiprecision::mpfr+gmp";

using int_backend = boost::multiprecision::gmp_int;

using float_backend = boost::multiprecision::mpfr_float_backend<0>;

using complex_backend = boost::multiprecision::complex_adaptor<float_backend>;

using rational_backend = boost::multiprecision::gmp_rational;

static inline void set_default_precision(int iv)
{
    default_precision = iv;
    boost::multiprecision::number<
        float_backend, boost::multiprecision::et_off>::default_precision(iv);
}

#endif // CPP / GMP / MPFR

using mpz =
    boost::multiprecision::number<int_backend, boost::multiprecision::et_off>;

using mpf =
    boost::multiprecision::number<float_backend, boost::multiprecision::et_off>;

using mpc = boost::multiprecision::number<complex_backend,
                                          boost::multiprecision::et_off>;

using mpq = boost::multiprecision::number<rational_backend,
                                          boost::multiprecision::et_off>;

template <class T>
struct is_arithmetic
    : std::integral_constant<bool, std::is_integral<T>::value ||
                                       std::is_floating_point<T>::value ||
                                       std::is_same_v<T, mpz> ||
                                       std::is_same_v<T, mpf>>
{
};
template <class T>
inline constexpr bool is_arithmetic_v = is_arithmetic<T>::value;

mpq make_quotient(const mpf& f, int digits);
mpq make_quotient(std::string_view s);

namespace helper
{
static inline mpz numerator(const mpq& q)
{
    return boost::multiprecision::numerator(q);
}
static inline mpz denominator(const mpq& q)
{
    return boost::multiprecision::denominator(q);
}
} // namespace helper

// explicit string parsers
extern mpz parse_mpz(std::string_view s);

extern mpc parse_mpc(std::string_view s);

static inline mpq parse_mpq(std::string_view s)
{
    return mpq(s);
}

#else // USE_BASIC_TYPES

#define floor_fn floorl
#define ceil_fn ceill
#define round_fn roundl
#define lcm_fn std::lcm
#define gcd_fn std::gcd
#define pow_fn powl

static constexpr int builtin_default_precision = 20;
static constexpr int max_precision = 20;

static constexpr const char MATH_BACKEND[] = "native types";

static inline void set_default_precision(int iv)
{
    default_precision = iv;
}

#include <complex>

template <typename T>
struct rational;

using mpz = long long;
using mpf = long double;
using mpc = std::complex<long double>;
using mpq = rational<long long>;

template <class T>
inline constexpr bool is_arithmetic_v = std::is_arithmetic<T>::value;

template <typename T>
struct rational
{
    T num = 0;
    T den = 1;

    rational() = default;

    rational(const rational&) = default;
    rational(rational&&) = default;
    rational& operator=(const rational&) = default;
    rational& operator=(rational&&) = default;

    rational(const mpz& n, const mpz& d) : num(n), den(d)
    {
        reduce();
    }

    explicit rational(std::string_view r)
    {
        auto d = r.find("/");
        if (d == std::string::npos)
        {
            num = std::stoll(r);
            lg::debug("mpq({}): num: {}, den: 1\n", r, num);
            den = 1;
        }
        else
        {
            auto numstr = r.substr(0, d);
            auto denstr = r.substr(d + 1);
            num = std::stoll(numstr);
            lg::debug("mpq({}): num: {} -> {}\n", r, numstr, num);
            den = std::stoll(denstr);
            lg::debug("mpq({}): den: {} -> {}\n", r, den, den);
        }
        reduce();
    }

    void reduce()
    {
        T cf = gcd_fn(num, den);
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

    bool operator==(const rational& r) const
    {
        return num == r.num && den == r.den;
    }
    bool operator!=(const rational& r) const
    {
        return !(num == r.num && den == r.den);
    }
    bool operator<(const rational& r) const
    {
        return num * r.den < r.num * den;
    }
    bool operator>(const rational& r) const
    {
        return num * r.den > r.num * den;
    }
    bool operator<=(const rational& r) const
    {
        return num * r.den >= r.num * den;
    }
    bool operator>=(const rational& r) const
    {
        return num * r.den >= r.num * den;
    }
    /* ops with other ratios */
    rational operator+(const rational& r) const
    {
        T nden = gcd_fn(den, r.den);
        T nnum = num * nden / den + r.num * nden / r.den;
        return rational(nnum, nden);
    }
    rational operator-(const rational& r) const
    {
        T nden = gcd_fn(den, r.den);
        T nnum = num * nden / den - r.num * nden / r.den;
        return rational(nnum, nden);
    }
    rational operator*(const rational& r) const
    {
        T nnum = num * r.num;
        T nden = den * r.den;
        return rational(nnum, nden);
    }
    rational operator/(const rational& r) const
    {
        T nnum = num * r.den;
        T nden = den * r.num;
        return rational(nnum, nden);
    }
    rational& operator+=(const rational& r)
    {
        T nden = gcd_fn(den, r.den);
        num = num * nden / den + r.num * nden / r.den;
        den = nden;
        reduce();
        return *this;
    }
    rational& operator-=(const rational& r)
    {
        T nden = gcd_fn(den, r.den);
        num = num * nden / den - r.num * nden / r.den;
        reduce();
        den = nden;
        return *this;
    }
    rational& operator*=(const rational& r)
    {
        num *= r.num;
        den *= r.den;
        reduce();
        return *this;
    }
    rational& operator/=(const rational& r)
    {
        num *= r.den;
        den *= r.num;
        reduce();
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& output, const rational& r)
    {
        return output << r.num << "/" << r.den;
    }
    friend std::istream& operator>>(std::istream& input, rational& r)
    {
        input >> r.num >> "/" >> r.den;
        return input;
    }
};

mpq make_quotient(const mpf& f, int digits);
mpq make_quotient(std::string_view s);

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

// string parsers
static inline mpz parse_mpz(std::string_view s)
{
    size_t end = 0;
    mpz ret{};
    try
    {
        ret = std::stoll(s, &end, 0);
    }
    catch (std::invalid_argument const& e)
    {
        // std::cerr << "std::invalid_argument::what(): " << e.what() << '\n';
    }
    catch (std::out_of_range const& e)
    {
        // std::cerr << "std::out_of_range::what(): " << e.what() << '\n';
    }
    // possible exponent?
    if (end != s.size())
    {
        if (s[end] == 'e')
        {
            std::string exps = s.substr(end + 1);
            end = 0;
            mpf exp{};
            try
            {
                exp = std::stoll(exps, &end);
            }
            catch (const std::exception& e)
            {
            }
            if (end != exps.size())
            {
                throw std::invalid_argument("input has an invalid exponent");
            }
            exp = pow_fn(10.0l, exp);
            mpf retf = ret * exp;
            if (exp == HUGE_VALL ||
                ((retf) > mpf(std::numeric_limits<mpz>::max())))
            {
                throw std::overflow_error("overflow with exponent");
            }
            ret = mpz(retf);
        }
        else
        {
            throw std::invalid_argument("input is not an integer");
        }
    }
    return ret;
}

extern mpc parse_mpc(std::string_view s);

static inline mpq parse_mpq(std::string_view s)
{
    return mpq(s);
}

#endif // USE_BASIC_TYPES

// implicit conversions are a nightmare, make it all explicit
// to_mpz
static inline mpz to_mpz(const mpz& v)
{
    return v;
}
static inline mpz to_mpz(const mpf& v)
{
    return static_cast<mpz>(v);
}

static inline mpz to_mpz(const mpq& v)
{
    return helper::numerator(v) / helper::denominator(v);
}
static inline mpz to_mpz(const mpc& v)
{
    return static_cast<mpz>(v.real());
}
// to_mpf
static inline mpf to_mpf(const mpz& v)
{
    return mpf(v);
}
static inline mpf to_mpf(const mpf& v)
{
    return v;
}
static inline mpf to_mpf(const mpq& v)
{
    return mpf(helper::numerator(v)) / mpf(helper::denominator(v));
}
static inline mpf to_mpf(const mpc& v)
{
    return v.real();
}
// to_mpq
static inline mpq to_mpq(const mpz& v)
{
    return mpq(v, 1);
}
static inline mpq to_mpq(const mpf& v)
{
    try
    {
        // try an exact quotient first
        return make_quotient(v, default_precision);
    }
    catch (const std::exception& e)
    {
        // do the best with our precision (we cannot fail)
        mpz den(1);
        den <<= default_precision;
        return mpq(to_mpz(v * mpf(den)), den);
    }
}
static inline mpq to_mpq(const mpq& v)
{
    return v;
}
static inline mpq to_mpq(const mpc& v)
{
    // magnitude here instead?
    mpf vmag = abs(v);
    return to_mpq(vmag);
}
// to_mpc
static inline mpc to_mpc(const mpz& v)
{
    return mpc(mpf(v), 0);
}
static inline mpc to_mpc(const mpf& v)
{
    return v;
}
static inline mpc to_mpc(const mpq& v)
{
    return mpc(to_mpf(v), 0);
}
static inline mpc to_mpc(const mpc& v)
{
    return v;
}

struct time_
{
    time_() : value(0, 0), absolute(false)
    {
    }

    template <typename Rep, typename Period>
    explicit time_(const std::chrono::duration<Rep, Period>& d,
                   bool absolute = false) :
        value(std::chrono::nanoseconds(d).count(), 1'000'000'000ull),
        absolute(absolute)
    {
    }

    template <typename Clock, typename Duration>
    explicit time_(const std::chrono::time_point<Clock, Duration>& tp) :
        time_(tp.time_since_epoch(), true)
    {
    }

    // parse time_ from string
    explicit time_(std::string_view t) :
        value(make_quotient(t)), absolute(false)
    {
    }

    // numbers to time_ (default to absolute times)
    time_(const mpz& t, bool absolute) : value(to_mpq(t)), absolute(absolute)
    {
    }
    time_(const mpq& t, bool absolute) : value(t), absolute(absolute)
    {
    }
    time_(const mpf& t, bool absolute) : value(to_mpq(t)), absolute(absolute)
    {
    }
    time_(const mpc& t, bool absolute) : value(to_mpq(t)), absolute(absolute)
    {
    }

    bool operator==(const time_& t) const
    {
        return absolute == t.absolute && value == t.value;
    }
    bool operator!=(const time_& t) const
    {
        return !(absolute == t.absolute && value == t.value);
    }
    bool operator<(const time_& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value < t.value;
    }
    bool operator>(const time_& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value > t.value;
    }
    bool operator<=(const time_& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value <= t.value;
    }
    bool operator>=(const time_& t) const
    {
        if (absolute != t.absolute)
        {
            throw std::invalid_argument(
                "cannot compare absolute and relative times");
        }
        return value >= t.value;
    }
    /* ops with other times */
    time_ operator+(const time_& t) const
    {
        if (absolute && t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform arithmetic with two absolute times");
        }
        return time_(value + t.value, absolute | t.absolute);
    }
    time_ operator-(const time_& t) const
    {
        return time_(value - t.value, absolute ^ t.absolute);
    }
    time_ operator*(const time_&) const
    {
        throw std::invalid_argument("cannot perform multiplication with times");
    }
    time_ operator/(const time_& t) const
    {
        if (absolute || t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform division with absolute times");
        }
        return time_(value / t.value, absolute);
    }
    time_& operator+=(const time_& t)
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
    time_& operator-=(const time_& t)
    {
        value -= t.value;
        absolute ^= t.absolute;
        return *this;
    }
    time_& operator*=(const time_& t)
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
    time_& operator/=(const time_& t)
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
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_ operator+(const T& t) const
    {
        return time_(value + to_mpq(t), absolute);
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_ operator-(const T& t) const
    {
        return time_(value - to_mpq(t), absolute);
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_ operator*(const T& t) const
    {
        // mult on absolute time makes it a duration
        return time_(value * to_mpq(t), false);
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_ operator/(const T& t) const
    {
        // div on absolute time makes it a duration
        return time_(value / to_mpq(t), false);
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_& operator+=(const T& t)
    {
        value += to_mpq(t);
        return *this;
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_& operator-=(const T& t)
    {
        value -= to_mpq(t);
        return *this;
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_& operator*=(const T& t)
    {
        // mult on absolute time makes it a duration
        value *= to_mpq(t);
        absolute = false;
        return *this;
    }
    template <typename T, std::enable_if_t<is_arithmetic_v<T>, bool> = true>
    time_& operator/=(const T& t)
    {
        // div on absolute time makes it a duration
        value /= to_mpq(t);
        absolute = false;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& output, const time_& t)
    {
        if (t.absolute)
        {
            long long nanos = static_cast<long long>(
                (helper::numerator(t.value) * mpz(1'000'000'000ll)) /
                helper::denominator(t.value));
            lg::debug("value={}, nanos={}\n", t.value, nanos);
            std::chrono::duration d = std::chrono::nanoseconds(nanos);
            std::chrono::time_point<std::chrono::system_clock> tp(d);
            const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
            return output << std::put_time(std::localtime(&t_c), "%F %T");
        }
        else
        {
            auto f = to_mpf(t.value);
            return output << std::setprecision(20) << f;
        }
    }

    mpq value;
    bool absolute;
};

/* scaler ops with time_ */
template <typename S, std::enable_if_t<is_arithmetic_v<S>, bool> = true>
time_ operator+(const S& s, const time_& t)
{
    return time_(t.value + to_mpq(s), t.absolute);
}
template <typename S, std::enable_if_t<is_arithmetic_v<S>, bool> = true>
time_ operator-(const S& s, const time_& t)
{
    return time_(t.value - to_mpq(s), t.absolute);
}
template <typename S, std::enable_if_t<is_arithmetic_v<S>, bool> = true>
time_ operator*(const S& s, const time_& t)
{
    // mult on absolute time makes it a duration
    return time_(t.value * to_mpq(s), false);
}
template <typename S, std::enable_if_t<is_arithmetic_v<S>, bool> = true>
time_ operator/(const S& s, const time_& t)
{
    // div on absolute time makes it a duration
    return time_(t.value / to_mpq(s), false);
}

using numeric = std::variant<mpz, mpf, mpc, mpq, time_>;

numeric reduce_numeric(const numeric& n, int precision = 2);

static constexpr auto numeric_types = std::to_array<const char*>({
    "mpz",
    "mpf",
    "mpc",
    "mpq",
    "time",
});

std::ostream& operator<<(std::ostream& out, const numeric& n);

mpz make_fixed(const mpz& v, int bits, bool is_signed);
mpq parse_mpf(std::string_view s);
std::optional<time_> parse_time(std::string_view s);

static inline bool operator<(const numeric& a, const numeric& b)
{
    return std::visit([](const auto& a, const auto& b) { return a < b; }, a, b);
}

static inline mpz to_mpz(const numeric& n)
{
    return std::visit([](const auto& a) { return to_mpz(a); }, n);
}
static inline mpf to_mpf(const numeric& n)
{
    return std::visit([](const auto& a) { return to_mpf(a); }, n);
}
static inline mpq to_mpq(const numeric& n)
{
    return std::visit([](const auto& a) { return to_mpq(a); }, n);
}
static inline mpc to_mpc(const numeric& n)
{
    return std::visit([](const auto& a) { return to_mpc(a); }, n);
}

template <typename TypeOut, typename TypeIn>
TypeOut coerce_variant(const TypeIn& in)
{
    if constexpr (std::is_same_v<TypeOut, mpz>)
    {
        return to_mpz(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpf>)
    {
        return to_mpf(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpc>)
    {
        return to_mpc(in);
    }
    else if constexpr (std::is_same_v<TypeOut, mpq>)
    {
        return to_mpq(in);
    }
    static_assert("incorrect argument to coerce_variant");
    throw std::invalid_argument("incorrect argument to coerce_variant");
}

// OPERATORS between numerics
// ADD
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
static inline mpc operator+(const mpc& c, const mpz& z)
{
    return c + to_mpc(z);
}
static inline mpc operator+(const mpc& c, const mpf& f)
{
    return c + to_mpc(f);
}
static inline mpc operator+(const mpz& z, const mpc& c)
{
    return to_mpc(z) + c;
}
static inline mpc operator+(const mpq& q, const mpc& c)
{
    return to_mpc(q) + c;
}
static inline mpc operator+(const mpc& c, const mpq& q)
{
    return c + to_mpc(q);
}

// SUBTRACT
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
    return c - to_mpc(z);
}
static inline mpc operator-(const mpz& z, const mpc& c)
{
    return to_mpc(z) - c;
}
static inline mpc operator-(const mpq& q, const mpc& c)
{
    return to_mpc(q) - c;
}
static inline mpc operator-(const mpc& c, const mpq& q)
{
    return c - to_mpc(q);
}

// MULTIPLY
static inline mpq operator*(const mpz& z, const mpq& q)
{
    return to_mpq(z) * q;
}
static inline mpq operator*(const mpq& q, const mpz& z)
{
    return q * to_mpq(z);
}
static inline mpc operator*(const mpq& q, const mpc& c)
{
    return to_mpc(q) * c;
}
static inline mpc operator*(const mpc& c, const mpq& q)
{
    return c * to_mpc(q);
}
static inline mpc operator*(const mpc& c, const mpz& z)
{
    return c * to_mpc(z);
}
static inline mpc operator*(const mpz& z, const mpc& c)
{
    return to_mpc(z) * c;
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
    return to_mpq(z) / q;
}
static inline mpc operator/(const mpc& c, const mpq& q)
{
    return c / to_mpc(q);
}
static inline mpc operator/(const mpq& q, const mpc& c)
{
    return to_mpc(q) / c;
}
static inline mpc operator/(const mpz& z, const mpc& c)
{
    return to_mpc(z) / c;
}
static inline mpc operator/(const mpc& c, const mpz& z)
{
    return c / to_mpc(z);
}
static inline mpf operator/(const mpq& q, const mpf& f)
{
    return to_mpf(q) / f;
}
static inline mpf operator/(const mpf& f, const mpq& q)
{
    return f / to_mpf(q);
}

struct binary_wrapper
{
    explicit binary_wrapper(const mpz& v, int bits) : v(v), bits(bits)
    {
    }
    const mpz& v;
    int bits;
};

struct oct_wrapper
{
    explicit oct_wrapper(const mpz& v, int bits) : v(v), bits(bits)
    {
    }
    const mpz& v;
    int bits;
};

struct hex_wrapper
{
    explicit hex_wrapper(const mpz& v, int bits) : v(v), bits(bits)
    {
    }
    const mpz& v;
    int bits;
};

std::ostream& operator<<(std::ostream& os, const binary_wrapper& bw);
std::ostream& operator<<(std::ostream& os, const oct_wrapper& bw);
std::ostream& operator<<(std::ostream& os, const hex_wrapper& bw);

#ifndef USE_BASIC_TYPES
template <>
struct fmt::formatter<mpz>
{
    fmt::detail::dynamic_format_specs<char> specs;

    // Parses format like the standard int formatter
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from fmt/core.h
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        using handler_type = fmt::detail::dynamic_specs_handler<ParseContext>;
        auto checker = fmt::detail::specs_checker<handler_type>(
            handler_type(specs, ctx), fmt::detail::type::int_type);
        auto it = fmt::detail::parse_format_specs(begin, end, checker);
        auto eh = ctx.error_handler();
        fmt::detail::check_int_type_spec(specs.type, eh);

        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const mpz& z, FormatContext& ctx) -> decltype(ctx.out())
    {
        // this part was found in fmt/format.h
        if (specs.width_ref.kind != fmt::detail::arg_id_kind::none)
        {
            fmt::detail::handle_dynamic_spec<fmt::detail::width_checker>(
                specs.width, specs.width_ref, ctx);
        }
        // ctx.out() is an output iterator to write to.
        std::stringstream ss;
        if (specs.type == fmt::presentation_type::bin_lower)
        {
            ss << binary_wrapper{z, specs.width};
        }
        else if (specs.type == fmt::presentation_type::oct)
        {
            ss << oct_wrapper{z, specs.width};
        }
        else if (specs.type == fmt::presentation_type::hex_lower)
        {
            ss << hex_wrapper{z, specs.width};
        }
        else // if (specs.type == fmt::presentation_type::dec)
        {
            ss << z;
        }
        auto s = ss.str();
        std::copy(s.begin(), s.end(), ctx.out());
        return ctx.out();
    }
};

template <typename Char>
struct fmt::formatter<mpf, Char>
{
    detail::dynamic_format_specs<Char> specs_;
    const Char* format_str_;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        format_str_ = ctx.begin();
        // Checks are deferred to formatting time when the argument type is
        // known.
        detail::dynamic_specs_handler<ParseContext> handler(specs_, ctx);
        return detail::parse_format_specs(ctx.begin(), ctx.end(), handler);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    // FormatContext is something like basic_format_context
    template <typename FormatContext>
    auto format(const mpf& f, FormatContext& ctx) -> decltype(ctx.out())
    {
        auto specs = specs_;
        detail::handle_dynamic_spec<detail::width_checker>(
            specs.width, specs.width_ref, ctx);
        detail::handle_dynamic_spec<detail::precision_checker>(
            specs.precision, specs.precision_ref, ctx);
        if (specs.precision < 1)
        {
            specs.precision = default_precision;
        }
        return fmt::format_to(ctx.out(), "{}", f.str(specs.precision));
    }
};
#endif

template <typename Char>
struct fmt::formatter<mpq, Char>
{
    detail::dynamic_format_specs<Char> specs_;
    const Char* format_str_;

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        format_str_ = ctx.begin();
        // Checks are deferred to formatting time when the argument type is
        // known.
        detail::dynamic_specs_handler<ParseContext> handler(specs_, ctx);
        return detail::parse_format_specs(ctx.begin(), ctx.end(), handler);
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    // FormatContext is something like basic_format_context
    template <typename FormatContext>
    auto format(const mpq& q, FormatContext& ctx) -> decltype(ctx.out())
    {
        auto specs = specs_;
        if (specs.type == fmt::presentation_type::fixed_lower)
        {
            detail::handle_dynamic_spec<detail::width_checker>(
                specs.width, specs.width_ref, ctx);
            detail::handle_dynamic_spec<detail::precision_checker>(
                specs.precision, specs.precision_ref, ctx);
            if (specs.precision < 1)
            {
                specs.precision = default_precision;
            }
            mpf f =
                to_mpf(helper::numerator(q)) / to_mpf(helper::denominator(q));
#ifdef USE_BASIC_TYPES
            return fmt::format_to(ctx.out(), "{}", std::to_string(f));
#else
            return fmt::format_to(ctx.out(), "{}", f.str(specs.precision));
#endif
        }
        // q form is chosen by 'g' (general_lower) presentation type
        mpz num = helper::numerator(q);
        mpz den = helper::denominator(q);
        return fmt::format_to(ctx.out(), "{}/{}", num, den);
    }
};

template <>
struct fmt::formatter<mpc>
{
    char presentation = 'r';

    // Parses format specifications of the form ['r' | 'p' | 'i'].
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        if (it != end && (*it == 'r' || *it == 'p' || *it == 'i'))
            presentation = *it++;

        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const mpc& c, FormatContext& ctx) -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        if (presentation == 'r')
        {
            return fmt::format_to(ctx.out(), "({}, {})", c.real(), c.imag());
        }
        else if (presentation == 'p')
        {
            return fmt::format_to(ctx.out(), "({}, <{})", abs(c),
                                  atan2(c.real(), c.imag()));
        }
        else
        {
            return fmt::format_to(ctx.out(), "{}{}{}i", c.real(),
                                  (c.imag() > 0 ? '+' : '-'), c.imag());
        }
    }
};

template <>
struct fmt::formatter<time_>
{
    // Parses format specifications of the form
    constexpr auto parse(format_parse_context& ctx) -> decltype(ctx.begin())
    {
        auto it = ctx.begin(), end = ctx.end();
        // Check if reached the end of the range:
        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const time_& t, FormatContext& ctx) -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        std::stringstream ss;
        ss << t;
        return fmt::format_to(ctx.out(), "{}", ss.str());
    }
};
