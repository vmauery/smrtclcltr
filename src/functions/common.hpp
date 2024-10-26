/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <numeric.hpp>

namespace smrty
{

class Calculator;

namespace function
{

namespace util
{

bool add_from_stack(Calculator& calc);
bool multiply_from_stack(Calculator& calc);
bool divide_from_stack(Calculator& calc);
bool inverse_from_stack(Calculator& calc);
bool power_from_stack(Calculator& calc);

numeric power_direct(const auto& a, const auto& b)
{
    if constexpr (same_type_v<decltype(a), mpz> &&
                  same_type_v<decltype(b), mpz>)
    {
        lg::debug("mpz pow({}, {})\n", a, b);
        if (b > zero)
        {
            return powul_fn(a, static_cast<int>(b));
        }
        return mpq{one, powul_fn(a, static_cast<int>(-b))};
    }
    else if constexpr (same_type_v<symbolic, decltype(a)> ||
                       same_type_v<symbolic, decltype(b)>)
    {
        return pow_fn(symbolic{a}, symbolic{b});
    }
    else if constexpr (same_type_v<mpc, decltype(a)> ||
                       same_type_v<mpc, decltype(b)>)
    {
        if constexpr (same_type_v<mpc, decltype(b)>)
        {
            return pow_fn(static_cast<mpc>(a), b);
        }
        else
        {
            return pow_fn(a, static_cast<mpc>(b));
        }
    }
    else
    {
        // all that is left are mixes of mpz, mpq, and mpf
        // cast to mpf and exponentiate
        return pow_fn(static_cast<mpf>(a), static_cast<mpf>(b));
    }
}

mpz factorial(const mpz&);

mpz comb(const mpz& x, const mpz& y);

mpz perm(const mpz& x, const mpz& y);

std::vector<mpz> factor_mpz(const mpz& x);

std::vector<mpz> prime_factor(mpz x);

} // namespace util

} // namespace function

} // namespace smrty
