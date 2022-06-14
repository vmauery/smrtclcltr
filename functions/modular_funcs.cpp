/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>

namespace function
{
namespace modexp
{

mpz pow(const mpz& base, const mpz& exponent, const mpz& modulus)
{
    mpz b(base), e(exponent), m(modulus);
    mpz result = 1;
    while (e > 0)
    {
        if (e & 1)
        {
            result *= b;
            result %= m;
        }
        e = e >> 1;
        b *= b;
        b %= m;
    }
    return result;
}

bool impl(Calculator& calc)
{
    return three_arg_limited_op(
        calc,
        [](const auto& a, const auto& b, const auto& c) {
            return pow(a, b, c);
        },
        std::tuple<mpz>{});
}

auto constexpr help =
    "\n"
    "    Usage: x y z modexp\n"
    "\n"
    "    Returns modular exponentiation of the bottom three items on "
    "    the stack, e.g., x raised to the y power mod z (x^y mod z)\n";

} // namespace modexp

namespace modinv
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(calc, [](const auto& a, const auto& b) {
        mpz t(0);
        mpz newt(1);
        mpz r = b;
        mpz newr = a;
        while (newr != 0)
        {
            mpz quotient = r / newr;
            std::tie(t, newt) = std::make_tuple(newt, t - quotient * newt);
            std::tie(r, newr) = std::make_tuple(newr, r - quotient * newr);
        }
        if (r > 1)
        {
            throw std::domain_error("x is not invertible");
        }
        if (t < 0)
        {
            t += b;
        }
        return t;
    });
}

auto constexpr help =
    "\n"
    "    Usage: x y modinv\n"
    "\n"
    "    Returns the multiplicative modular inverse of x (mod y)\n";

} // namespace modinv
} // namespace function

namespace functions
{

CalcFunction modexp = {function::modexp::help, function::modexp::impl};
CalcFunction modinv = {function::modinv::help, function::modinv::impl};

} // namespace functions
