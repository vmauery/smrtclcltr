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
bool divide_from_stack(Calculator& calc);

mpz factorial(const mpz&);

mpz comb(const mpz& x, const mpz& y);

mpz perm(const mpz& x, const mpz& y);

std::vector<mpz> factor_mpz(const mpz& x);

std::vector<mpz> prime_factor(mpz x);

} // namespace util

} // namespace function

} // namespace smrty
