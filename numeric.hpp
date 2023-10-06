/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <debug.hpp>
#include <format>
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
            // lg::debug("mpq({}): num: {}, den: 1\n", r, num);
            den = 1;
        }
        else
        {
            auto numstr = r.substr(0, d);
            auto denstr = r.substr(d + 1);
            num = std::stoll(numstr);
            // lg::debug("mpq({}): num: {} -> {}\n", r, numstr, num);
            den = std::stoll(denstr);
            // lg::debug("mpq({}): den: {} -> {}\n", r, den, den);
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
    std::string str() const
    {
        return std::format("{}/{}", r, num, r.den);
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
        // lg::debug("std::invalid_argument::what(): {}\n",  e.what());
    }
    catch (std::out_of_range const& e)
    {
        // lg::debug("std::out_of_range::what(): {}\n",  e.what());
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
    return mpc{mpf{v}, 0};
}
static inline mpc to_mpc(const mpf& v)
{
    return v;
}
static inline mpc to_mpc(const mpq& v)
{
    return mpc{mpf{v}, 0};
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

    std::string str() const
    {
        if (absolute)
        {
            long long nanos = static_cast<long long>(
                (helper::numerator(value) * mpz(1'000'000'000ll)) /
                helper::denominator(value));
            // lg::debug("value={}, nanos={}\n", value, nanos);
            std::chrono::duration d = std::chrono::nanoseconds(nanos);
            std::chrono::time_point<std::chrono::system_clock> tp(d);
            return std::format("{:%F %T}", tp);
            // const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
            // return std::strftime(std::localtime(&t_c), "%F %T");
        }
        else
        {
            long long nanos = static_cast<long long>(
                (helper::numerator(value) * mpz(1'000'000'000ll)) /
                helper::denominator(value));
            // lg::debug("value={}, nanos={}\n", value, nanos);
            std::chrono::duration d = std::chrono::nanoseconds(nanos);
            return std::format("{}", d);
            // auto f = to_mpf(value);
            // return f.str(20);
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

std::string mpz_to_bin_string(const mpz& v, std::streamsize width);

#ifndef USE_BASIC_TYPES
template <>
struct std::formatter<mpz>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_fill_and_align(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_sign(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_zero_fill(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'b':
                spec._M_type = std::__format::_Pres_b;
                ++begin;
                break;
            case 'd':
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            case 'o':
                spec._M_type = std::__format::_Pres_o;
                ++begin;
                break;
            case 'x':
                spec._M_type = std::__format::_Pres_x;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const mpz& z, FormatContext& ctx) const -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        std::streamsize width = spec._M_get_width(ctx);
        std::string s{};
        if (spec._M_type == std::__format::_Pres_b)
        {
            // manually make the binary version; mpz does not do it
            s = mpz_to_bin_string(z, width);
        }
        else
        {
            // hex/dec/oct are all handled by mpz
            std::ios_base::fmtflags f = std::ios::showbase;
            if (spec._M_type == std::__format::_Pres_o)
            {
                f |= std::ios::oct;
            }
            else if (spec._M_type == std::__format::_Pres_x)
            {
                f |= std::ios::hex;
            }
            else // if (spec._M_type == std::__format::_Pres_d)
            {
                f |= std::ios::dec;
            }
            s = z.str(width, f);
        }
        auto out = ctx.out();
        if (z >= 0 && spec._M_sign == std::__format::_Sign_plus)
        {
            *out++ = '+';
        }
        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct std::formatter<mpf>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard float parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_fill_and_align(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_sign(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_zero_fill(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'f':
                spec._M_type = std::__format::_Pres_f;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const mpf& f, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1)
        {
            precision = default_precision;
        }
        auto out = ctx.out();
        if (f >= 0 && spec._M_sign == std::__format::_Sign_plus)
        {
            *out++ = '+';
        }
        auto s = f.str(precision);
        return std::copy(s.begin(), s.end(), out);
    }
};
#endif // USE_BASIC_TYPES

template <>
struct std::formatter<mpq>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard float parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_fill_and_align(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_sign(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_zero_fill(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'q': // quotient
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            case 'f': // fraction
                spec._M_type = std::__format::_Pres_f;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const mpq& q, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1)
        {
            precision = default_precision;
        }
        if (spec._M_type == std::__format::_Pres_f)
        {
            mpf f =
                to_mpf(helper::numerator(q)) / to_mpf(helper::denominator(q));
#ifdef USE_BASIC_TYPES
            return std::format_to(ctx.out(), "{:{1}f}", precision, f);
#else
            auto s = f.str(precision);
            return std::copy(s.begin(), s.end(), ctx.out());
#endif
        }
        else // if (spec._M_type == std::__format::_Pres_d)
        {
            // q form is chosen by 'q' presentation type
            mpz num = helper::numerator(q);
            mpz den = helper::denominator(q);
            return std::format_to(ctx.out(), "{}/{}", num, den);
        }
    }
};

template <>
struct std::formatter<mpc>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_fill_and_align(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_sign(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_zero_fill(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'i': // i or j notation
                spec._M_type = std::__format::_Pres_g;
                ++begin;
                break;
            case 'p': // polar
                spec._M_type = std::__format::_Pres_p;
                ++begin;
                break;
            case 'r': // rectangular
                spec._M_type = std::__format::_Pres_f;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const mpc& c, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1)
        {
            precision = default_precision;
        }
        // ctx.out() is an output iterator to write to.
        if (spec._M_type == std::__format::_Pres_f)
        {
            return std::format_to(ctx.out(), "({0:.{2}f}, {1:.{2}f})", c.real(),
                                  c.imag(), precision);
        }
        else if (spec._M_type == std::__format::_Pres_p)
        {
            return std::format_to(ctx.out(), "({0:.{2}f}, <{1:.{2}f})", abs(c),
                                  atan2(c.real(), c.imag()), precision);
        }
        else // if (spec._M_type == std::__format::_Pres_g)
        {
            return std::format_to(ctx.out(), "{0:.{2}f}{1:+.{2}}i", c.real(),
                                  c.imag(), precision);
        }
    }
};

template <>
struct std::formatter<time_>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const time_& t, FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto s = t.str();
        return std::copy(s.begin(), s.end(), ctx.out());
    }
};

template <typename... Types>
struct std::formatter<std::variant<Types...>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // nothing to parse
        auto begin = ctx.begin();
        return begin;
    }

    template <typename FormatContext>
    auto format(const std::variant<Types...>& t, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        return std::visit(
            [&ctx](const auto& v) {
                return std::format_to(ctx.out(), "{}", v);
            },
            t);
    }
};
