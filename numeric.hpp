/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <iostream>
#include <variant>

extern int default_precision;
#if (USE_BOOST_CPP_BACKEND || USE_GMP_BACKEND || USE_MPFR_BACKEND)

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
    return static_cast<mpz>(static_cast<mpf>(v));
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
    return v;
}
static inline mpq to_mpq(const mpf& v)
{
    mpz den(1);
    den <<= default_precision;
    return mpq(to_mpz(v * mpf(den)), den);
}
static inline mpq to_mpq(const mpq& v)
{
    return v;
}
static inline mpq to_mpq(const mpc& v)
{
    mpz den(1);
    den <<= default_precision;
    return mpq(to_mpz(v.real() * mpf(den)), den);
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

// explicit string parsers
static inline mpz parse_mpz(const std::string& s)
{
    // if s is base 10 (does not start with 0), and has an e
    // peel off the e and handle that separately.
    if (size_t epos{}; s[0] != '0' && (epos = s.find('e')) != std::string::npos)
    {
        mpz ret(s.substr(0, epos));
        std::string exps = s.substr(epos + 1);
        unsigned int exp{};
        size_t end{};
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
        ret *= mpz(boost::multiprecision::pow(mpf(10), exp));
        return ret;
    }
    return mpz(s);
}
static inline mpf parse_mpf(const std::string& s)
{
    return mpf(s);
}

extern mpc parse_mpc(const std::string& s);

static inline mpq parse_mpq(const std::string& s)
{
    return mpq(s);
}

#else // USE_BASIC_TYPES

static constexpr const char MATH_BACKEND[] = "native types";

static inline void set_default_precision(int iv)
{
    default_precision = iv;
}

#include <complex>

template <typename T>
struct rational;

template <typename T>
class complexr;

using mpz = long long;
using mpf = long double;
using mpc = std::complex<long double>;
using mpq = rational<long long>;

/*
template <class T>
class complexr : public std::complex<T>
{
  public:
    constexpr complexr(const T& re = T(), const T& im = T()) :
        std::complex<T>(re, im)
    {
    }
    constexpr complexr(const complexr<T>& t) : std::complex<T>(t)
    {
    }

    template <typename R>
    explicit constexpr complexr(const rational<R>& r) :
        std::complex<T>(static_cast<T>(r.numerator()) /
                        static_cast<T>(r.denominator()))
    {
    }
};
*/

template <typename T>
struct rational
{
    T num;
    T den;
    rational(const mpz& n = 0, const mpz& d = 1) : num(n), den(d)
    {
    }

    explicit rational(const std::string& r)
    {
        auto d = r.find("/");
        if (d != std::string::npos)
        {
            num = std::stoll(r);
            den = 1;
        }
        else
        {
            num = std::stoll(r.substr(0, d));
            den = std::stoll(r.substr(d + 1));
        }
    }

    const T& numerator() const
    {
        return num;
    }
    const T& denominator() const
    {
        return den;
    }

    /*
    rational& operator=(const rational& r)
    {
        num = r.num;
        den = r.den;
        return *this;
    }
    */

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
        return double(num) / double(den) < double(r.num) / double(r.den);
    }
    bool operator>(const rational& r) const
    {
        return double(num) / double(den) > double(r.num) / double(r.den);
    }
    bool operator<=(const rational& r) const
    {
        return double(num) / double(den) <= double(r.num) / double(r.den);
    }
    bool operator>=(const rational& r) const
    {
        return double(num) / double(den) >= double(r.num) / double(r.den);
    }
    /* ops with other ratios */
    rational operator+(const rational& r) const
    {
        T nden = std::__gcd(den, r.den);
        T nnum = num * nden / den + r.num * nden / r.den;
        return rational(nnum, nden);
    }
    rational operator-(const rational& r) const
    {
        T nden = std::__gcd(den, r.den);
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
    /* ops with scalers */
    template <typename S>
    rational operator+(const S& s) const
    {
        T nnum = num + s * den;
        return rational(nnum, den);
    }
    template <typename S>
    rational operator-(const S& s) const
    {
        T nnum = num - s * den;
        return rational(nnum, den);
    }
    template <typename S>
    rational operator*(const S& s) const
    {
        T nnum = num * s;
        return rational(nnum, den);
    }
    template <typename S>
    rational operator/(const S& s) const
    {
        T nden = den * s;
        return rational(num, nden);
    }
    rational& operator+=(const rational& r)
    {
        T nden = __gcd(den, r.den);
        num = num * nden / den + r.num * nden / r.den;
        den = nden;
        return *this;
    }
    rational& operator-=(const rational& r)
    {
        T nden = __gcd(den, r.den);
        num = num * nden / den - r.num * nden / r.den;
        den = nden;
        return *this;
    }
    rational& operator*=(const rational& r)
    {
        num *= r.num;
        den *= r.den;
        return *this;
    }
    rational& operator/=(const rational& r)
    {
        num *= r.den;
        den *= r.num;
        return *this;
    }
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
    return static_cast<mpz>(static_cast<mpf>(v));
}
static inline mpz to_mpz(const mpc& v)
{
    return static_cast<mpz>(v.real());
}
// to_mpf
static inline mpf to_mpf(const mpz& v)
{
    return v;
}
static inline mpf to_mpf(const mpf& v)
{
    return v;
}
static inline mpf to_mpf(const mpq& v)
{
    return v;
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
    mpz den(1);
    den <<= default_precision;
    return mpq(to_mpz(v * den), den);
}
static inline mpq to_mpq(const mpq& v)
{
    return v;
}
static inline mpq to_mpq(const mpc& v)
{
    mpz den(1);
    den <<= default_precision;
    return mpq(to_mpz(v.real() * den), den);
}
// to_mpc
static inline mpc to_mpc(const mpz& v)
{
    return mpc(v);
}
static inline mpc to_mpc(const mpf& v)
{
    return v;
}
static inline mpc to_mpc(const mpq& v)
{
    return mpc(v, 0);
}
static inline mpc to_mpc(const mpc& v)
{
    return v;
}

// string parsers
static inline mpz parse_mpz(const std::string& s)
{
    size_t end = 0;
    mpz ret{};
    try
    {
        ret = std::stoll(s, &end, 0);
    }
    catch (std::invalid_argument const& e)
    {
        std::cerr << "std::invalid_argument::what(): " << e.what() << '\n';
    }
    catch (std::out_of_range const& e)
    {
        std::cerr << "std::out_of_range::what(): " << e.what() << '\n';
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
            exp = powl(10.0l, exp);
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

static inline mpf parse_mpf(const std::string& s)
{
    return std::stold(s);
}

extern mpc parse_mpc(const std::string& s);

static inline mpq parse_mpq(const std::string& s)
{
    return mpq(s);
}

#endif // USE_BASIC_TYPES

namespace util
{

template <typename From, typename To>
class implicit
{
    using success = char;
    using dummy = long double;
    static success test(To);
    static dummy test(...);
    static From f();

  public:
    static constexpr bool exists = sizeof(test(f())) == sizeof(success);
};

} // namespace util

static constexpr bool has_mpz_to_mpc = util::implicit<mpz, mpc>::exists;
static constexpr bool has_mpq_to_mpc = util::implicit<mpq, mpc>::exists;

using datetime = std::chrono::time_point<std::chrono::system_clock>;
#ifdef COMMENT_CODE
std::tuple<std::chrono::year_month_day, std::chrono::hh_mm_ss>
    parse_datetime(const datetime& dt)
{
    auto tp = std::chrono::zoned_time{std::chrono::current_zone(), dt}
                  .get_local_time();
    auto dp = floor<std::chrono::days>(tp);
    std::chrono::year_month_day ymd{dp};
    std::chrono::hh_mm_ss hms{floor<std::chrono::milliseconds>(tp - dp)};
    /*
    auto y = ymd.year();
    auto m = ymd.month();
    auto d = ymd.day();
    auto h = hms.hours();
    auto M = hms.minutes();
    auto s = hms.seconds();
    auto ms = hms.subseconds();
    */
    return {ymd, hms};
}
#endif

using numeric = std::variant<mpz, mpf, mpc, mpq>;

static constexpr auto numeric_types = std::to_array<const char*>({
    "mpz",
    "mpf",
    "mpc",
    "mpq",
});

// TODO: add the other 80 combinations? maybe as needed?
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpz& bb, const mpz& cc)
{
    return fn(aa, bb, cc);
}

template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpz& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpq& bb)
{
    return fn(to_mpq(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpf& bb)
{
    return fn(to_mpf(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpz& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}

// mpq
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpz& bb)
{
    return fn(aa, to_mpq(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpq& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpf& bb)
{
    return fn(to_mpf(aa), bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpq& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}

// mpf
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpz& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpq& bb)
{
    return fn(aa, to_mpf(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpf& bb)
{
    return fn(aa, bb);
}
template <typename Fn>
numeric operate(const Fn& fn, const mpf& aa, const mpc& bb)
{
    return fn(to_mpc(aa), bb);
}

// mpc
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpz& bb)
{
    return fn(aa, to_mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpq& bb)
{
    return fn(aa, to_mpc(bb));
}
template <typename Fn>
numeric operate(const Fn& fn, const mpc& aa, const mpf& bb)
{
    return fn(aa, to_mpc(bb));
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
