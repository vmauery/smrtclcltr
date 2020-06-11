/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <iostream>
#include <variant>

// #define TEST_BASIC_TYPES 1

#ifndef TEST_BASIC_TYPES

// #define USE_BOOST_CPP 1

#ifdef USE_BOOST_CPP
#include <boost/serialization/nvp.hpp>
// stay
#include <boost/multiprecision/cpp_bin_float.hpp>
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#else
#include <boost/multiprecision/gmp.hpp>
#endif
#include <boost/multiprecision/complex_adaptor.hpp>
#include <boost/multiprecision/number.hpp>

#ifdef USE_BOOST_CPP
static constexpr size_t max_digits = 1000;
static constexpr int32_t exp_min = -262142;
static constexpr int32_t exp_max = 262143;

using exp_type = std::decay<decltype(exp_max)>::type;

using mpz = boost::multiprecision::cpp_int;

using mpf = boost::multiprecision::number<
    boost::multiprecision::backends::cpp_bin_float<
        max_digits, boost::multiprecision::backends::digit_base_10, void,
        exp_type, exp_min, exp_max>,
    boost::multiprecision::et_on>;

using mpc = boost::multiprecision::number<
    boost::multiprecision::backends::complex_adaptor<
        boost::multiprecision::cpp_bin_float<
            max_digits, boost::multiprecision::backends::digit_base_10, void,
            exp_type, exp_min, exp_max>>,
    boost::multiprecision::et_on>;

using mpq = boost::multiprecision::cpp_rational;

#else /* USE_GMP */

using mpz = boost::multiprecision::number<boost::multiprecision::gmp_int,
                                          boost::multiprecision::et_off>;
using mpf = boost::multiprecision::number<boost::multiprecision::gmp_float<0>,
                                          boost::multiprecision::et_off>;
using mpc = boost::multiprecision::number<
    boost::multiprecision::complex_adaptor<boost::multiprecision::gmp_float<0>>,
    boost::multiprecision::et_off>;
using mpq = boost::multiprecision::number<boost::multiprecision::gmp_rational,
                                          boost::multiprecision::et_off>;

#endif // CPP / GMP

#else // basic types

#include <complex>

template <typename T>
struct ratio;

using mpz = long;
using mpf = double;
using mpc = std::complex<double>;
using mpq = ratio<long>;

template <typename T>
struct ratio
{
    T num;
    T den;
    ratio(T n, T d) : num(n), den(d)
    {
    }
    explicit ratio(const std::string& r)
    {
        auto d = r.find("/");
        if (d != std::string::npos)
        {
            num = std::stoi(r);
            den = 1;
        }
        else
        {
            num = std::stoi(r.substr(0, d));
            den = std::stoi(r.substr(d + 1));
        }
    }

    explicit ratio(const mpz& r) : num(r), den(1)
    {
    }

    /*
    ratio& operator=(const ratio& r)
    {
        num = r.num;
        den = r.den;
        return *this;
    }
    */

    bool operator==(const ratio& r) const
    {
        return num == r.num && den == r.den;
    }
    bool operator!=(const ratio& r) const
    {
        return !(num == r.num && den == r.den);
    }
    bool operator<(const ratio& r) const
    {
        return double(num) / double(den) < double(r.num) / double(r.den);
    }
    bool operator>(const ratio& r) const
    {
        return double(num) / double(den) > double(r.num) / double(r.den);
    }
    bool operator<=(const ratio& r) const
    {
        return double(num) / double(den) <= double(r.num) / double(r.den);
    }
    bool operator>=(const ratio& r) const
    {
        return double(num) / double(den) >= double(r.num) / double(r.den);
    }
    /* ops with other ratios */
    ratio operator+(const ratio& r) const
    {
        T nden = std::__gcd(den, r.den);
        T nnum = num * nden / den + r.num * nden / r.den;
        return ratio(nnum, nden);
    }
    ratio operator-(const ratio& r) const
    {
        T nden = std::__gcd(den, r.den);
        T nnum = num * nden / den - r.num * nden / r.den;
        return ratio(nnum, nden);
    }
    ratio operator*(const ratio& r) const
    {
        T nnum = num * r.num;
        T nden = den * r.den;
        return ratio(nnum, nden);
    }
    ratio operator/(const ratio& r) const
    {
        T nnum = num * r.den;
        T nden = den * r.num;
        return ratio(nnum, nden);
    }
    /* ops with scalers */
    template <typename S>
    ratio operator+(const S& s) const
    {
        T nnum = num + s * den;
        return ratio(nnum, den);
    }
    template <typename S>
    ratio operator-(const S& s) const
    {
        T nnum = num - s * den;
        return ratio(nnum, den);
    }
    template <typename S>
    ratio operator*(const S& s) const
    {
        T nnum = num * s;
        return ratio(nnum, den);
    }
    template <typename S>
    ratio operator/(const S& s) const
    {
        T nden = den * s;
        return ratio(num, nden);
    }
    /*
    ratio& operator+=(const ratio& r)
    {
        nden = __gcd(den, r.den);
        num = num * nden / den + r.num * nden / r.den;
        den = nden;
        return *this;
    }
    ratio& operator-=(const ratio& r)
    {
        T nden = __gcd(den, r.den);
        num = num * nden / den - r.num * nden / r.den;
        den = nden;
        return *this;
    }
    ratio& operator*=(const ratio& r)
    {
        num *= r.num;
        den *= r.den;
        return *this;
    }
    ratio& operator/=(const ratio& r)
    {
        num *= r.den;
        den *= r.num;
        return *this;
    }
    */
    operator mpz() const
    {
        return static_cast<mpz>(num / den);
    }
    operator mpf() const
    {
        return static_cast<mpf>(num) / static_cast<mpf>(den);
    }
    operator mpc() const
    {
        return static_cast<mpf>(num) / static_cast<mpf>(den);
    }
    friend std::ostream& operator<<(std::ostream& output, const ratio& r)
    {
        return output << r.num << "/" << r.den;
    }
    friend std::istream& operator>>(std::istream& input, ratio& r)
    {
        input >> r.num >> "/" >> r.den;
        return input;
    }
};

#endif // TEST_BASIC_TYPES

// using date = std::chrono::system_clock::time_point;

using numeric = std::variant<mpz, mpf, mpc, mpq>;

template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpz& bb)
{
    return mpz(fn(aa, bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpq& bb)
{
    return mpq(fn(mpq(aa), bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpf& bb)
{
    return mpf(fn(mpf(aa), bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpc& bb)
{
    return fn(mpc(aa), bb);
}

// mpq
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpz& bb)
{
    return mpq(fn(aa, mpq(bb)));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpq& bb)
{
    return mpq(fn(aa, bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpf& bb)
{
    return mpf(fn(mpf(aa), bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpc& bb)
{
    return fn(mpc(aa), bb);
}

// mpf
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpz& bb)
{
    return mpf(fn(aa, mpf(bb)));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpq& bb)
{
    return mpf(fn(aa, mpf(bb)));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpf& bb)
{
    return mpf(fn(aa, bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpc& bb)
{
    return fn(mpc(aa), bb);
}

// mpc
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpz& bb)
{
    return fn(aa, mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpq& bb)
{
    return fn(aa, mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpf& bb)
{
    return fn(aa, mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpc& bb)
{
    return fn(aa, bb);
}

template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa)
{
    return fn(aa);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa)
{
    return fn(aa);
}

std::ostream& operator<<(std::ostream& out, const numeric& n);

mpz make_fixed(const mpz& v, int bits, bool is_signed);