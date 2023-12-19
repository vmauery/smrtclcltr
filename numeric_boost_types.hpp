/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#if (USE_BOOST_CPP_BACKEND || USE_GMP_BACKEND || USE_MPFR_BACKEND)

#include <boost/integer/common_factor_rt.hpp>
#include <boost/math/special_functions/gamma.hpp>

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

extern int default_precision;
static constexpr int builtin_default_precision = 50;
// yes, I know abritrary precision, but be reasonable, my dude!
static constexpr unsigned int max_precision = 1000000;
static constexpr unsigned int max_bits = 64 * 1024;

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

#endif // if/elif CPP / GMP / MPFR

using mpz =
    boost::multiprecision::number<int_backend, boost::multiprecision::et_off>;

using mpf =
    boost::multiprecision::number<float_backend, boost::multiprecision::et_off>;

using mpc = boost::multiprecision::number<complex_backend,
                                          boost::multiprecision::et_off>;

using mpq = boost::multiprecision::number<rational_backend,
                                          boost::multiprecision::et_off>;

mpq make_quotient(std::string_view s);
// lossless (minimized error)
mpq make_quotient(const mpf& f, int digits);
// lossy (ignored error)
mpq make_quotient(const mpf& f);

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

#endif // (CPP || MPFR || GMP)
