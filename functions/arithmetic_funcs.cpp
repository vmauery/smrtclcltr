/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace add
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a + b; });
}

auto constexpr help = "addition for first two items on stack";

} // namespace add

namespace subtract
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a - b; });
}

auto constexpr help = "subtraction for first two items on stack";

} // namespace subtract

namespace multiply
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a * b; });
}

auto constexpr help = "multiplication for first two items on stack";

} // namespace multiply

namespace divide
{

bool impl(Calculator& calc)
{
    return two_arg_conv_op(
        calc, [](const auto& a, const auto& b) { return a / b; },
        std::tuple<mpz>{}, std::tuple<mpq>{}, std::tuple<mpq, mpf, mpc>{});
}

auto constexpr help = "division for first two items on stack";

} // namespace divide

namespace negate
{

bool impl(Calculator& calc)
{
    return one_arg_op(calc, [](const auto& a) { return -a; });
}

auto constexpr help = "negates the first item on the stack";

} // namespace negate

namespace inverse
{

bool impl(Calculator& calc)
{
    return one_arg_op(calc, [](const auto& a) { return 1 / a; });
}

auto constexpr help = "inverse of the first item on the stack (1/X)";

} // namespace inverse

namespace divmod
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(
        calc, [](const auto& a, const auto& b) { return a % b; });
}

auto constexpr help = "modular division for the first two items on the stack";

} // namespace divmod

namespace power
{

numeric pow(const mpz& base, const mpz& exponent)
{
    mpz b(base), e(exponent);
    bool invert = false;
    if (e < 0)
    {
        invert = true;
        e = -e;
    }
    mpz result = 1;
    while (e > 0)
    {
        if (e & 1)
        {
            result *= b;
        }
        e = e >> 1;
        b *= b;
    }
    if (invert)
    {
        return mpq(1, result);
    }
    return result;
}

bool impl(Calculator& calc)
{
    return two_arg_conv_op(
        calc, [](const auto& a, const auto& b) { return pow(a, b); },
        std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
}

auto constexpr help = "exponentiation for the first two items on the stack";

} // namespace power

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

auto constexpr help = "modular exponentiation using the first three items on "
                      "the stack:\n(X ^ Y) mod Z";

} // namespace modexp

} // namespace function

namespace functions
{

CalcFunction add = {function::add::help, function::add::impl};
CalcFunction subtract = {function::subtract::help, function::subtract::impl};
CalcFunction multiply = {function::multiply::help, function::multiply::impl};
CalcFunction divide = {function::divide::help, function::divide::impl};
CalcFunction negate = {function::negate::help, function::negate::impl};
CalcFunction inverse = {function::inverse::help, function::inverse::impl};
CalcFunction divmod = {function::divmod::help, function::divmod::impl};
CalcFunction power = {function::power::help, function::power::impl};
CalcFunction modexp = {function::modexp::help, function::modexp::impl};

} // namespace functions
