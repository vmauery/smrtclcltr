/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <boost/math/constants/constants.hpp>
#include <chrono>
#include <concepts>
#include <debug.hpp>
#include <exception.hpp>
#include <format>
#include <program.hpp>
#include <symbolic.hpp>
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
struct is_complex
    : std::integral_constant<bool,
                             std::is_same<mpc, std::remove_cvref_t<T>>::value>
{
};

template <class T>
struct is_real
    : std::integral_constant<bool, is_integer<T>::value || is_float<T>::value ||
                                       is_rational<T>::value>
{
};
template <class T>
inline constexpr bool is_real_v = is_real<T>::value;

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

#ifdef USE_BASIC_TYPES
static inline mpq reduce_mpq(const mpq& q)
{
    return q;
}
#else
static inline mpq reduce_mpq(const mpq& q)
{
    mpz c = gcd_fn(helper::numerator(q), helper::denominator(q));
    if (c > 1)
    {
        if (c == helper::denominator(q))
        {
            return mpq(helper::numerator(q) / c, 1);
        }
        return mpq(helper::numerator(q) / c, helper::denominator(q) / c);
    }
    lg::debug("reduce: no change\n");
    return q;
}
#endif

template <typename... T>
std::variant<T...> reduce_numeric(const std::variant<T...>& n,
                                  int precision = 2)
{
    if (precision == 0)
    {
        precision = default_precision;
    }
    std::visit(
        [](const auto& v) {
            lg::debug("reduce({} (type {}))\n", v, DEBUG_TYPE(v));
        },
        n);
    /*
     * may be lossy if precision is low... mpf to mpq/mpz might be a lie
     * mpc -> mpf for imaginary = 0
     * mpf -> mpz if no fractional part
     * mpf -> mpq for perfect fractions?
     * mpq -> mpz for denominator = 1
     */
    if (auto q = std::get_if<mpq>(&n); q)
    {
        mpq rq = reduce_mpq(*q);
        if (helper::denominator(rq) == one)
        {
            lg::debug("reduce: denominator is one\n");
            return helper::numerator(*q);
        }
        return n;
    }
    else if (auto f = std::get_if<mpf>(&n); f)
    {
        if (*f == mpf(0.0))
        {
            return zero;
        }
        // internally, make_quotient will do calculations
        // with a higher precision than the current precision
        // but we will limit the size of the denominator to
        // a reasonable size to keep irrationals from getting
        // turned into rationals
        try
        {
            // make_quotient might return a reducible q
            // so call reduce again
            return reduce_numeric(
                std::variant<T...>{make_quotient(*f, precision / 5)},
                precision);
        }
        catch (const std::exception& e)
        {
            return n;
        }
    }
    else if (auto c = std::get_if<mpc>(&n); c)
    {
        if (c->imag() == mpf(0.0))
        {
            return reduce_numeric(std::variant<T...>{mpf{c->real()}},
                                  precision);
        }
        return n;
    }
    return n;
}

// clang-format off
#include <variant_math.hpp>
#include <list.hpp>
#include <matrix.hpp>
#include <time.hpp>
// clang-format on

using mpx = std::variant<mpz, mpq, mpf, mpc>;

using time_ = basic_time<mpq>;
using matrix = basic_matrix<mpx>;
using list = basic_list<mpx>;

using numeric = std::variant<mpz, mpq, mpf, mpc, list, matrix, time_,
                             smrty::program, smrty::symbolic>;

static constexpr auto numeric_types = std::to_array<const char*>({
    "mpz",
    "mpq",
    "mpf",
    "mpc",
    "list",
    "matrix",
    "time",
});

mpz make_fixed(const mpz& v, int bits, bool is_signed);

// string parsers
extern mpz parse_mpz(std::string_view s, int base = 10);
extern numeric parse_mpz(const smrty::single_number_parts&);
extern mpc parse_mpc(std::string_view s);
static inline mpq parse_mpq(std::string_view s)
{
    return mpq(s);
}
mpq parse_mpf(std::string_view s);
time_ parse_time(std::string_view s);
matrix parse_matrix(std::string_view s);
list parse_list(std::string_view s);

numeric make_numeric(const smrty::number_parts&);
numeric make_numeric(const smrty::single_number_parts&);
numeric make_numeric(const smrty::two_number_parts&);
numeric make_numeric(const smrty::time_parts&);
numeric make_numeric(const smrty::compound_parts&);

template <typename TypeOut, typename TypeIn>
TypeOut coerce_variant(const TypeIn& in)
{
    if constexpr (same_type_v<TypeOut, mpz>)
    {
        return static_cast<mpz>(in);
    }
    else if constexpr (same_type_v<TypeOut, mpf>)
    {
        return static_cast<mpf>(in);
    }
    else if constexpr (same_type_v<TypeOut, mpc>)
    {
        return static_cast<mpc>(in);
    }
    else if constexpr (same_type_v<TypeIn, mpf> && same_type_v<TypeOut, mpq>)
    {
        return make_quotient(in);
    }
    else if constexpr (same_type_v<TypeOut, mpq>)
    {
        return static_cast<mpq>(in);
    }
    static_assert("incorrect argument to coerce_variant");
    throw std::invalid_argument("incorrect argument to coerce_variant");
}

/*********************************************************************
 *
 *   numeric operators
 *
 ********************************************************************/

/* scaler ops with time_ */
template <typename S, std::enable_if_t<is_real_v<S>, bool> = true>
static inline time_ operator+(const S& s, const time_& t)
{
    return time_(t.value + static_cast<mpq>(s), t.absolute);
}
template <>
inline time_ operator+(const mpf& s, const time_& t)
{
    return time_(t.value + make_quotient(s), t.absolute);
}
template <typename S, std::enable_if_t<is_real_v<S>, bool> = true>
static inline time_ operator-(const S& s, const time_& t)
{
    return time_(t.value - static_cast<mpq>(s), t.absolute);
}
template <>
inline time_ operator-(const mpf& s, const time_& t)
{
    return time_(t.value - make_quotient(s), t.absolute);
}
template <typename S, std::enable_if_t<is_real_v<S>, bool> = true>
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

static inline smrty::symbolic operator+(const smrty::symbolic& s, const auto& o)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic addition");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator-(const smrty::symbolic& s, const auto& o)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic subtraction");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator*(const smrty::symbolic& s, const auto& o)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic multiplication");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator/(const smrty::symbolic& s, const auto& o)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic division");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator+(const auto& o, const smrty::symbolic& s)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic addition");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator-(const auto& o, const smrty::symbolic& s)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic subtraction");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator*(const auto& o, const smrty::symbolic& s)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic multiplication");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}
static inline smrty::symbolic operator/(const auto& o, const smrty::symbolic& s)
{
    if constexpr (is_one_of_v<decltype(o), mpx>)
    {
        throw smrty::not_yet_implemented("symbolic division");
    }
    else
    {
        throw std::invalid_argument("Invalid argument for symbolic operations");
    }
}

// OPERATORS between numerics
// ADD
template <typename T>
    requires is_one_of_v<T, list::element_type>
static inline list operator+(const T& t, const list& lst)
{
    return lst + t;
}
static inline time_ operator+(const mpq& q, const time_& t)
{
    time_ nt = t;
    nt.value += q;
    return nt;
}
static inline time_ operator+(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value += q;
    return nt;
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
template <typename T>
    requires(is_one_of_v<T, matrix::element_type>)
static inline matrix operator*(const T& t, const matrix& m)
{
    return m * t;
}
template <typename T>
    requires(is_one_of_v<T, list::element_type>)
static inline list operator*(const T& t, const list& lst)
{
    return lst * t;
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
static inline time_ operator*(const time_& t, const mpq& q)
{
    time_ nt = t;
    nt.value *= q;
    return nt;
}

// DIVIDE
template <typename T>
    requires(is_one_of_v<T, matrix::element_type>)
static inline matrix operator/(const T& t, const matrix& m)
{
    return m.inv() * t;
}
static inline matrix operator/(const matrix& ml, const matrix& mr)
{
    return ml * mr.inv();
}
template <typename T>
    requires(is_one_of_v<T, list::element_type>)
static inline list operator/(const T& t, const list& lst)
{
    return lst / t;
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

// As the number of types are held within numeric grows, the
// number of operators between them that *might* be needed
// grows as a factorial of N. Since this quickly gets out of
// hand for writing each pair, we start with a catch-all
// operator of each kind that throws a runtime error saying
// that this kind of operation is not supported between these
// two types. Then, we go ahead and implement all the pairs
// that actually make sense.

template <typename T>
concept require_numeric_types = variant_has_member<T, numeric>::value;

template <typename L, typename R>
struct combined_type
{
    using type = L;
};
template <typename L, typename R>
using combined_type_t = combined_type<L, R>::type;

auto operator+(const require_numeric_types auto& l,
               const require_numeric_types auto& r)
    -> combined_type_t<decltype(l), decltype(r)>
{
    throw std::invalid_argument("Invalid operands for addition");
}
auto operator-(const require_numeric_types auto& l,
               const require_numeric_types auto& r)
    -> combined_type_t<decltype(l), decltype(r)>
{
    throw std::invalid_argument("Invalid operands for subtraction");
}
auto operator*(const require_numeric_types auto& l,
               const require_numeric_types auto& r)
    -> combined_type_t<decltype(l), decltype(r)>
{
    throw std::invalid_argument("Invalid operands for multiplication");
}
auto operator/(const require_numeric_types auto& l,
               const require_numeric_types auto& r)
    -> combined_type_t<decltype(l), decltype(r)>
{
    throw std::invalid_argument("Invalid operands for division");
}

std::string mpz_to_bin_string(const mpz& v, std::streamsize width);

namespace smrty
{
static inline numeric abs(const matrix& m)
{
    return variant_cast(m.det());
}
} // namespace smrty

#define NEED_NUMERIC_TYPE_FORMATTERS 1
#include <numeric_format.hpp>
