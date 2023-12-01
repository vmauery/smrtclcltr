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

#include <boost/math/special_functions/gamma.hpp>

static constexpr int builtin_default_precision = 50;
// yes, I know abritrary precision, but be reasonable, my dude!
static constexpr unsigned int max_precision = 1000000;
static constexpr unsigned int max_bits = 64 * 1024;

#define floor_fn boost::multiprecision::floor
#define ceil_fn boost::multiprecision::ceil
#define round_fn boost::multiprecision::round
#define lcm_fn boost::math::lcm
#define gcd_fn boost::math::gcd
#define pow_fn boost::multiprecision::pow
#define powul_fn boost::multiprecision::pow
#define gamma_fn boost::math::tgamma
#define abs_fn abs
#define log_fn log
#define sqrt_fn sqrt
#define sin_fn sin
#define cos_fn cos
#define tan_fn tan
#define asin_fn asin
#define acos_fn acos
#define atan_fn atan
#define sinh_fn sinh
#define cosh_fn cosh
#define tanh_fn tanh
#define asinh_fn asinh
#define acosh_fn acosh
#define atanh_fn atanh

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
extern mpz parse_mpz(std::string_view s, int base = 10);

extern mpc parse_mpc(std::string_view s);

static inline mpq parse_mpq(std::string_view s)
{
    return mpq(s);
}

// free operator for boost mpq
static inline mpq operator%(const mpq& a, const mpq& b)
{
    // q = a/b -> c/d
    // r = a - b * (c - (c % d)) / d
    // equivalent to:
    // a b dup2 / split dup2 rolld swap % - roll / * -
    mpq q{helper::numerator(a) * helper::denominator(b),
          helper::denominator(a) * helper::numerator(b)};
    return a - b * mpq{helper::numerator(q) -
                           helper::numerator(q) % helper::denominator(q),
                       helper::denominator(q)};
}

// free operator for boost mpf
static inline mpf operator%(const mpf& a, const mpf& b)
{
    mpz q = static_cast<mpz>(a / b);
    return a - b * q;
}

// commonly used mpz values
static const mpz zero{0};
static const mpz one{1};
static const mpz two{2};
static const mpz ten{10};
static const mpz one_million{1'000'000};
static const mpz one_billion{1'000'000'000};

#else // USE_BASIC_TYPES

template <class T>
concept integer = std::is_integral<T>::value;

template <class T>
concept floating = std::is_floating_point<T>::value;

#define floor_fn std::floor
#define ceil_fn std::ceil
#define round_fn std::round
#define lcm_fn std::lcm
#define gcd_fn std::gcd
#define pow_fn std::pow
#define powul_fn std::powul
#define gamma_fn std::tgamma
#define abs_fn std::abs
#define log_fn std::log
#define sqrt_fn std::sqrt
#define sin_fn std::sin
#define cos_fn std::cos
#define tan_fn std::tan
#define asin_fn std::asin
#define acos_fn std::acos
#define atan_fn std::atan
#define sinh_fn std::sinh
#define cosh_fn std::cosh
#define tanh_fn std::tanh
#define asinh_fn std::asinh
#define acosh_fn std::acosh
#define atanh_fn std::atanh

static constexpr int builtin_default_precision = 6;
static constexpr unsigned int max_precision = LDBL_DIG;
static constexpr unsigned int max_bits = sizeof(long long) * 8;

static constexpr const char MATH_BACKEND[] = "native types";

static inline void set_default_precision(int iv)
{
    default_precision = iv;
}

#include <complex>

template <typename T>
struct rational;

template <integer T>
struct checked_int;

template <floating T>
struct checked_float;

using mpz = checked_int<long long>;
using mpf = checked_float<long double>;
using mpc = std::complex<long double>;
using mpq = rational<checked_int<long long>>;

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

template <integer T>
struct std::numeric_limits<checked_int<T>> : public std::numeric_limits<T>
{
};

template <integer T>
struct checked_int
{
    T value;

    checked_int(const checked_int<T>&) = default;
    checked_int(checked_int<T>&&) = default;
    checked_int& operator=(const checked_int<T>&) = default;
    checked_int& operator=(checked_int<T>&&) = default;

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
        return checked_float<F>(static_cast<F>(value));
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
    constexpr operator bool() const
    {
        return value;
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

template <floating T>
struct std::numeric_limits<checked_float<T>> : public std::numeric_limits<T>
{
};

template <floating T>
struct checked_float
{
    T value;

    checked_float(const checked_float<T>&) = default;
    checked_float(checked_float<T>&&) = default;
    checked_float& operator=(const checked_float<T>&) = default;
    checked_float& operator=(checked_float<T>&&) = default;

    template <floating F>
    constexpr explicit checked_float(const F& v) : value(static_cast<T>(v))
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

    /* automatic conversion to mpc */
    explicit constexpr operator mpc() const
    {
        return mpc(value);
    }
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

// commonly used mpz values
constexpr mpz zero{0};
constexpr mpz one{1};
constexpr mpz two{2};
constexpr mpz ten{10};
constexpr mpz one_million{1'000'000};
constexpr mpz one_billion{1'000'000'000};

namespace std
{
mpz powul(const mpz& base, int exponent);
std::tuple<mpz, mpz> pow(const mpz& base, int exponent);
constexpr mpf pow(const mpf& base, const mpf& exponent)
{
    return mpf{std::pow(base.value, exponent.value)};
}
constexpr mpf tgamma(const mpf& v)
{
    return mpf{std::tgamma(v.value)};
}
constexpr mpz gcd(const mpz& l, const mpz& r)
{
    return mpz{std::gcd(l.value, r.value)};
}
constexpr mpz lcm(const mpz& l, const mpz& r)
{
    return mpz{std::lcm(l.value, r.value)};
}
constexpr mpf abs(const mpf& v)
{
    if (v >= 0.0l)
    {
        return v;
    }
    return -v;
}
constexpr mpz abs(const mpz& v)
{
    if (v >= zero)
    {
        return v;
    }
    return -v;
}
constexpr mpf log(const mpf& v)
{
    return mpf{std::log(v.value)};
}
constexpr mpf sqrt(const mpf& v)
{
    return mpf{std::sqrt(v.value)};
}
constexpr mpf floor(const mpf& v)
{
    return mpf{std::floor(v.value)};
}
constexpr mpf ceil(const mpf& v)
{
    return mpf{std::ceil(v.value)};
}
constexpr mpf round(const mpf& v)
{
    return mpf{std::round(v.value)};
}
constexpr mpf sin(const mpf& v)
{
    return mpf{std::sin(v.value)};
}
constexpr mpf cos(const mpf& v)
{
    return mpf{std::cos(v.value)};
}
constexpr mpf tan(const mpf& v)
{
    return mpf{std::tan(v.value)};
}
constexpr mpf asin(const mpf& v)
{
    return mpf{std::asin(v.value)};
}
constexpr mpf acos(const mpf& v)
{
    return mpf{std::acos(v.value)};
}
constexpr mpf atan(const mpf& v)
{
    return mpf{std::atan(v.value)};
}
constexpr mpf sinh(const mpf& v)
{
    return mpf{std::sinh(v.value)};
}
constexpr mpf cosh(const mpf& v)
{
    return mpf{std::cosh(v.value)};
}
constexpr mpf tanh(const mpf& v)
{
    return mpf{std::tanh(v.value)};
}
constexpr mpf asinh(const mpf& v)
{
    return mpf{std::asinh(v.value)};
}
constexpr mpf acosh(const mpf& v)
{
    return mpf{std::acosh(v.value)};
}
constexpr mpf atanh(const mpf& v)
{
    return mpf{std::atanh(v.value)};
}
} // namespace std

template <typename T>
struct rational
{
    T num = 0;
    T den = 1;

    rational(const rational&) = default;
    rational(rational&&) = default;
    rational& operator=(const rational&) = default;
    rational& operator=(rational&&) = default;

    constexpr rational() : num(0), den(1)
    {
    }

    constexpr rational(const T& n) : num(n), den(1)
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
        T cf = std::gcd(num, den);
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

    explicit constexpr operator long double() const
    {
        return static_cast<long double>(num) / static_cast<long double>(den);
    }

    bool operator==(const rational<T>& r) const
    {
        return num == r.num && den == r.den;
    }
    bool operator!=(const rational<T>& r) const
    {
        return !(num == r.num && den == r.den);
    }
    bool operator<(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
        return num * (lcm / den) < r.num * (lcm / r.den);
    }
    bool operator>(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
        return num * (lcm / den) > r.num * (lcm / r.den);
    }
    bool operator<=(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
        return num * (lcm / den) <= r.num * (lcm / r.den);
    }
    bool operator>=(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
        return num * (lcm / den) >= r.num * (lcm / r.den);
    }
    /* ops with other ratios */
    rational<T> operator+(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
        T nnum = num * (lcm / den) + r.num * (lcm / r.den);
        return rational<T>(nnum, lcm);
    }
    rational<T> operator-(const rational<T>& r) const
    {
        T lcm = std::lcm(den, r.den);
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
        T lcm = std::lcm(den, r.den);
        num = num * (lcm / den) + r.num * (lcm / r.den);
        den = lcm;
        reduce();
        return *this;
    }
    rational<T>& operator-=(const rational<T>& r)
    {
        T lcm = std::lcm(den, r.den);
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

mpq make_quotient(const mpf& f, int digits);
mpq make_quotient(std::string_view s);

static inline mpq abs(const mpq& v)
{
    return (v >= mpq{}) ? v : mpq{} - v;
}

static inline mpz abs(const mpz& v)
{
    return (v >= zero) ? v : -v;
}

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
extern mpz parse_mpz(std::string_view s, int base = 10);
extern mpc parse_mpc(std::string_view s);

static inline mpq parse_mpq(std::string_view s)
{
    return mpq(s);
}

#endif // USE_BASIC_TYPES

// implicit conversions are a nightmare, make it all explicit
// to_mpz
#ifndef USE_BASIC_TYPES
// TODO: unsure why boost types need these, but basic types don't
//       probably some implicit conversion thing
static inline mpz to_mpz(const mpz& v)
{
    return v;
}
static inline mpz to_mpz(const mpq& v)
{
    return helper::numerator(v) / helper::denominator(v);
}
#endif
static inline mpz to_mpz(const mpf& v)
{
    return static_cast<mpz>(v);
}
static inline mpz to_mpz(const mpc& v)
{
    return static_cast<mpz>(v.real());
}
// to_mpf
static inline mpf to_mpf(const mpz& v)
{
    return static_cast<mpf>(v);
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
    return mpf{v.real()};
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
    mpf vmag{abs(v)};
    return to_mpq(vmag);
}
// to_mpc
static inline mpc to_mpc(const mpz& v)
{
    return static_cast<mpc>(static_cast<mpf>(v));
}
static inline mpc to_mpc(const mpf& v)
{
    return static_cast<mpc>(v);
}
static inline mpc to_mpc(const mpq& v)
{
    return static_cast<mpc>(static_cast<mpf>(v));
}
static inline mpc to_mpc(const mpc& v)
{
    return v;
}

struct time_
{
    time_() : value(0, 1), absolute(false)
    {
    }

#ifdef USE_BASIC_TYPES
    template <typename Rep, typename Period>
    explicit time_(const std::chrono::duration<Rep, Period>& d,
                   bool absolute = false) :
        value(std::chrono::duration_cast<std::chrono::microseconds>(d).count(),
              1'000'000ull),
        absolute(absolute)
    {
        lg::debug(
            "time_({}ms, {})\n",
            std::chrono::duration_cast<std::chrono::microseconds>(d).count(),
            absolute);
    }
#else  // !USE_BASIC_TYPES
    template <typename Rep, typename Period>
    explicit time_(const std::chrono::duration<Rep, Period>& d,
                   bool absolute = false) :
        value(std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(),
              1'000'000'000ull),
        absolute(absolute)
    {
        lg::debug(
            "time_({}ns, {})\n",
            std::chrono::duration_cast<std::chrono::nanoseconds>(d).count(),
            absolute);
    }
#endif // USE_BASIC_TYPES

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
    mpq operator/(const time_& t) const
    {
        if (absolute || t.absolute)
        {
            throw std::invalid_argument(
                "cannot perform division with absolute times");
        }
        return value / t.value;
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

    // used by std::format(), but also formats mpq, which is below
    std::string str() const;

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
    return std::visit(
        [](const auto& a, const auto& b) {
            if constexpr (std::is_same_v<decltype(a), decltype(b)>)
            {
                return a < b;
            }
            else if constexpr (std::is_same_v<decltype(a), mpc> ||
                               std::is_same_v<decltype(b), mpc>)
            {
                return to_mpc(a) < to_mpc(b);
            }
            else if constexpr (std::is_same_v<decltype(a), mpf> ||
                               std::is_same_v<decltype(b), mpf>)
            {
                return to_mpf(a) < to_mpf(b);
            }
            else if constexpr (std::is_same_v<decltype(a), mpq> ||
                               std::is_same_v<decltype(b), mpq>)
            {
                return to_mpq(a) < to_mpq(b);
            }
            else
            {
                throw std::invalid_argument("non-comparable values");
            }
            return false;
        },
        a, b);
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
static inline mpf operator+(const mpf& l, const mpz& r)
{
    return l + mpf(r);
}
static inline mpf operator+(const mpz& l, const mpf& r)
{
    return mpf(l) + r;
}
static inline mpf operator+(const mpf& l, const mpc& r)
{
    return l + to_mpf(r);
}
static inline mpf operator+(const mpf& l, const mpq& r)
{
    return l + to_mpf(r);
}
static inline mpf operator+(const mpq& l, const mpf& r)
{
    return to_mpf(l) + r;
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
static inline mpq operator-(const mpz& z, const mpq& q)
{
    return to_mpq(z) - q;
}
static inline mpf operator-(const mpz& z, const mpf& f)
{
    return to_mpf(z) - f;
}
static inline mpc operator-(const mpz& z, const mpc& c)
{
    return to_mpc(z) - c;
}
static inline mpf operator-(const mpf& f, const mpq& q)
{
    return f - to_mpf(q);
}
static inline mpf operator-(const mpq& q, const mpf& f)
{
    return to_mpf(q) - f;
}
static inline mpc operator-(const mpf& f, const mpc& c)
{
    return to_mpc(f) - c;
}
static inline mpc operator-(const mpc& c, const mpf& f)
{
    return c - to_mpc(f);
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
static inline mpf operator*(const mpq& q, const mpf& f)
{
    return to_mpf(q) * f;
}
static inline mpc operator*(const mpq& q, const mpc& c)
{
    return to_mpc(q) * c;
}
static inline mpf operator*(const mpf& f, const mpq& q)
{
    return f * to_mpf(q);
}
static inline mpc operator*(const mpf& f, const mpc& c)
{
    return to_mpc(f) * c;
}
static inline mpc operator*(const mpc& c, const mpq& q)
{
    return c * to_mpc(q);
}
static inline mpc operator*(const mpc& c, const mpf& f)
{
    return c * to_mpc(f);
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
static inline mpc operator/(const mpc& c, const mpf& f)
{
    return c / to_mpc(f);
}
static inline mpf operator/(const mpq& q, const mpf& f)
{
    return to_mpf(q) / f;
}
static inline mpf operator/(const mpf& f, const mpq& q)
{
    return f / to_mpf(q);
}
static inline mpc operator/(const mpf& f, const mpc& c)
{
    return to_mpc(f) / c;
}

// MODULAR DIVIDE
// z q f
static inline mpq operator%(const mpz& z, const mpq& q)
{
    return to_mpq(z) % q;
}
static inline mpq operator%(const mpq& q, const mpz& z)
{
    return q % to_mpq(z);
}
static inline mpf operator%(const mpz& z, const mpf& f)
{
    return to_mpf(z) % f;
}
static inline mpf operator%(const mpf& f, const mpz& z)
{
    return f % to_mpf(z);
}
static inline mpf operator%(const mpq& q, const mpf& f)
{
    return to_mpf(q) % f;
}
static inline mpf operator%(const mpf& f, const mpq& q)
{
    return f % to_mpf(q);
}

std::string mpz_to_bin_string(const mpz& v, std::streamsize width);

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
        begin = spec._M_parse_alternate_form(begin, end);
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
            std::ios_base::fmtflags f{};
            if (spec._M_alt)
            {
                f |= std::ios::showbase;
            }
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
        if (z >= zero && spec._M_sign == std::__format::_Sign_plus)
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
        if (precision < 1.0)
        {
            precision = default_precision;
        }
        auto out = ctx.out();
        if (f >= 0.0 && spec._M_sign == std::__format::_Sign_plus)
        {
            *out++ = '+';
        }
        auto s = f.str(precision);
        return std::copy(s.begin(), s.end(), out);
    }
};

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
            return std::format_to(ctx.out(), "{0:{1}f}", f, precision);
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
            return std::format_to(ctx.out(), "({0:.{2}f},{1:.{2}f})", c.real(),
                                  c.imag(), precision);
        }
        else if (spec._M_type == std::__format::_Pres_p)
        {
            return std::format_to(ctx.out(), "({0:.{2}f},<{1:.{2}f})", abs(c),
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
