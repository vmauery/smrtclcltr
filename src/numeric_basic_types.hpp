/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#ifdef USE_BASIC_TYPES

#include <cmath>
#include <complex>
#include <numeric>
#include <string_view>
#include <tuple>

#define floor_fn smrty::floor
#define ceil_fn smrty::ceil
#define round_fn smrty::round
#define lcm_fn smrty::lcm
#define gcd_fn smrty::gcd
#define pow_fn smrty::pow
#define exp_fn smrty::exp
#define powul_fn smrty::powul
#define gamma_fn smrty::tgamma
#define zeta_fn smrty::zeta
#define abs_fn smrty::abs
#define log_fn smrty::log
#define sqrt_fn smrty::sqrt
#define sin_fn smrty::sin
#define cos_fn smrty::cos
#define tan_fn smrty::tan
#define asin_fn smrty::asin
#define acos_fn smrty::acos
#define atan_fn smrty::atan
#define sinh_fn smrty::sinh
#define cosh_fn smrty::cosh
#define tanh_fn smrty::tanh
#define asinh_fn smrty::asinh
#define acosh_fn smrty::acosh
#define atanh_fn smrty::atanh

static constexpr int builtin_default_precision = 6;
extern int default_precision;
static constexpr unsigned int max_precision = LDBL_DIG;
static constexpr unsigned int max_bits = sizeof(long long) * 8;

static constexpr const char MATH_BACKEND[] = "native types";

static inline void set_default_precision(int iv)
{
    default_precision = iv;
}

template <integer I>
struct checked_int;

template <floating F>
struct checked_float;

template <typename T>
struct rational;

template <typename T>
struct basic_time;

template <typename T>
struct basic_matrix;

using mpz = checked_int<long long>;
using mpf = checked_float<long double>;
using mpc = std::complex<long double>;
using mpq = rational<checked_int<long long>>;

mpq make_quotient(std::string_view s);
// lossless (minimized error)
mpq make_quotient(const mpf& f, int digits);
// lossy (ignored error)
mpq make_quotient(const mpf& f);

// forward declaration of numeric-typed std functions
namespace smrty
{
mpz powul(const mpz& base, int exponent);
std::tuple<mpz, mpz> powl(const mpz& base, int exponent);
constexpr mpf exp(const mpf& exponent);
constexpr mpc exp(const mpc& exponent);
constexpr mpf pow(const mpf& base, const mpf& exponent);
constexpr mpc pow(const mpc& base, const mpc& exponent);
constexpr mpf pow(const mpq& base, const mpq& exponent);
constexpr mpf tgamma(const mpf& v);
constexpr mpf zeta(const mpf& v);
constexpr mpz gcd(const mpz& l, const mpz& r);
constexpr mpz lcm(const mpz& l, const mpz& r);
constexpr mpf abs(const mpf& v);
constexpr mpz abs(const mpz& v);
constexpr mpq abs(const mpq& v);
constexpr mpf abs(const mpc& v);
constexpr mpf log(const mpf& v);
constexpr mpc log(const mpc& v);
constexpr mpf sqrt(const mpf& v);
constexpr mpc sqrt(const mpc& v);
constexpr mpf floor(const mpf& v);
constexpr mpf ceil(const mpf& v);
constexpr mpf round(const mpf& v);
constexpr mpf sin(const mpf& v);
constexpr mpc sin(const mpc& v);
constexpr mpf cos(const mpf& v);
constexpr mpc cos(const mpc& v);
constexpr mpf tan(const mpf& v);
constexpr mpc tan(const mpc& v);
constexpr mpf asin(const mpf& v);
constexpr mpc asin(const mpc& v);
constexpr mpf acos(const mpf& v);
constexpr mpc acos(const mpc& v);
constexpr mpf atan(const mpf& v);
constexpr mpc atan(const mpc& v);
constexpr mpf sinh(const mpf& v);
constexpr mpc sinh(const mpc& v);
constexpr mpf cosh(const mpf& v);
constexpr mpc cosh(const mpc& v);
constexpr mpf tanh(const mpf& v);
constexpr mpc tanh(const mpc& v);
constexpr mpf asinh(const mpf& v);
constexpr mpc asinh(const mpc& v);
constexpr mpf acosh(const mpf& v);
constexpr mpc acosh(const mpc& v);
constexpr mpf atanh(const mpf& v);
constexpr mpc atanh(const mpc& v);
} // namespace smrty

// clang-format off
#include <checked_int.hpp>
#include <checked_float.hpp>
#include <rational.hpp>
// clang-format on

// commonly used mpz values
constexpr mpz zero{0};
constexpr mpz one{1};
constexpr mpz two{2};
constexpr mpz ten{10};
constexpr mpz one_million{1'000'000};
constexpr mpz one_billion{1'000'000'000};

namespace smrty
{
constexpr mpf exp(const mpf& exponent)
{
    return mpf{std::exp(exponent.value)};
}
constexpr mpc exp(const mpc& exponent)
{
    return std::exp(exponent);
}
constexpr mpf pow(const mpf& base, const mpf& exponent)
{
    return mpf{std::pow(base.value, exponent.value)};
}
constexpr mpc pow(const mpc& base, const mpc& exponent)
{
    return std::pow(base, exponent);
}
constexpr mpf pow(const mpq& base, const mpq& exponent)
{
    return std::pow(mpf{base}, mpf{exponent});
}
constexpr mpf tgamma(const mpf& v)
{
    return mpf{std::tgamma(v.value)};
}
constexpr mpf zeta(const mpf& v)
{
    return mpf{std::riemann_zeta(v.value)};
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
    return mpf{std::abs(v.value)};
}
constexpr mpz abs(const mpz& v)
{
    return std::abs(v.value);
}
constexpr mpq abs(const mpq& v)
{
    if (v < mpq{})
    {
        return v * mpq{-1};
    }
    return v;
}
constexpr mpf abs(const mpc& v)
{
    return mpf{std::abs(v)};
}
constexpr mpf log(const mpf& v)
{
    return mpf{std::log(v.value)};
}
constexpr mpc log(const mpc& v)
{
    return std::log(v);
}
constexpr mpf sqrt(const mpf& v)
{
    return mpf{std::sqrt(v.value)};
}
constexpr mpc sqrt(const mpc& v)
{
    return std::sqrt(v);
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
constexpr mpc sin(const mpc& v)
{
    return std::sin(v);
}
constexpr mpf cos(const mpf& v)
{
    return mpf{std::cos(v.value)};
}
constexpr mpc cos(const mpc& v)
{
    return std::cos(v);
}
constexpr mpf tan(const mpf& v)
{
    return mpf{std::tan(v.value)};
}
constexpr mpc tan(const mpc& v)
{
    return std::tan(v);
}
constexpr mpf asin(const mpf& v)
{
    return mpf{std::asin(v.value)};
}
constexpr mpc asin(const mpc& v)
{
    return std::asin(v);
}
constexpr mpf acos(const mpf& v)
{
    return mpf{std::acos(v.value)};
}
constexpr mpc acos(const mpc& v)
{
    return std::acos(v);
}
constexpr mpf atan(const mpf& v)
{
    return mpf{std::atan(v.value)};
}
constexpr mpc atan(const mpc& v)
{
    return std::atan(v);
}
constexpr mpf sinh(const mpf& v)
{
    return mpf{std::sinh(v.value)};
}
constexpr mpc sinh(const mpc& v)
{
    return std::sinh(v);
}
constexpr mpf cosh(const mpf& v)
{
    return mpf{std::cosh(v.value)};
}
constexpr mpc cosh(const mpc& v)
{
    return std::cosh(v);
}
constexpr mpf tanh(const mpf& v)
{
    return mpf{std::tanh(v.value)};
}
constexpr mpc tanh(const mpc& v)
{
    return std::tanh(v);
}
constexpr mpf asinh(const mpf& v)
{
    return mpf{std::asinh(v.value)};
}
constexpr mpc asinh(const mpc& v)
{
    return std::asinh(v);
}
constexpr mpf acosh(const mpf& v)
{
    return mpf{std::acosh(v.value)};
}
constexpr mpc acosh(const mpc& v)
{
    return std::acosh(v);
}
constexpr mpf atanh(const mpf& v)
{
    return mpf{std::atanh(v.value)};
}
constexpr mpc atanh(const mpc& v)
{
    return std::atanh(v);
}
} // namespace smrty

#endif // USE_BASIC_TYPES
