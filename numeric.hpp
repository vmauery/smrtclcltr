/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

// #define USE_BOOST_CPP 1

#include <boost/math/constants/constants.hpp>
#ifdef USE_BOOST_CPP
#include <boost/multiprecision/cpp_complex.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#else
#include <boost/multiprecision/gmp.hpp>
#endif
#include <boost/multiprecision/complex_adaptor.hpp>
#include <boost/multiprecision/number.hpp>
#include <chrono>
#include <iostream>
#include <variant>

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
    boost::multiprecision::et_off>;

using mpc = boost::multiprecision::number<
    boost::multiprecision::backends::complex_adaptor<
        boost::multiprecision::cpp_bin_float<
            max_digits, boost::multiprecision::backends::digit_base_10, void,
            exp_type, exp_min, exp_max>>,
    boost::multiprecision::et_off>;

using mpq = boost::multiprecision::cpp_rational;
#else
using mpz = boost::multiprecision::number<boost::multiprecision::gmp_int>;
using mpf = boost::multiprecision::number<boost::multiprecision::gmp_float<0>>;
using mpc =
    boost::multiprecision::number<boost::multiprecision::complex_adaptor<
        boost::multiprecision::gmp_float<0>>>;
using mpq = boost::multiprecision::number<boost::multiprecision::gmp_rational>;
#endif

using date = std::chrono::system_clock::time_point;

using numeric = std::variant<mpz, mpf, mpc, mpq, date>;

template <class... Ts>
struct overload : Ts...
{
    using Ts::operator()...;
};
template <class... Ts>
overload(Ts...) -> overload<Ts...>;

template <typename Fn>
numeric operate(const Fn& fn, const numeric& a, const numeric& b)
{
    // mpq is the only one that does not play nicely with mpf or mpc
    // mpz < mpq < mpf < mpc
    return std::visit(overload{
                          [&fn](const mpq& aa, const mpf& bb) -> numeric {
                              return fn(mpf(aa), bb);
                          },
                          [&fn](const mpf& aa, const mpq& bb) -> numeric {
                              return fn(aa, mpf(bb));
                          },
                          [&fn](const mpq& aa, const mpc& bb) -> numeric {
                              return fn(mpc(aa), bb);
                          },
                          [&fn](const mpc& aa, const mpq& bb) -> numeric {
                              return fn(aa, mpc(bb));
                          },
                          [&fn](const mpc& aa, const mpz& bb) -> numeric {
                              return fn(aa, mpc(bb));
                          },
                          [&fn](const mpz& aa, const mpc& bb) -> numeric {
                              return fn(mpc(aa), bb);
                          },
                          [&fn](const auto& aa, const auto& bb) -> numeric {
                              return fn(aa, bb);
                          },
                      },
                      a, b);
}

std::ostream& operator<<(std::ostream& out, const numeric& n);

mpz make_fixed(const mpz& v, int bits, bool is_signed);
