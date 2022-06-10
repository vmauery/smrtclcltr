/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>

namespace function
{
namespace add
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a + b; });
}

auto constexpr help =
    "\n"
    "    Usage: x y +\n"
    "\n"
    "    Returns the sum of the bottom two items on the stack (x + y)\n";

} // namespace add

namespace subtract
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a - b; });
}

auto constexpr help =
    "\n"
    "    Usage: x y -\n"
    "\n"
    "    Returns the difference of the bottom two items on the stack (x - y)\n";

} // namespace subtract

namespace multiply
{

bool impl(Calculator& calc)
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a * b; });
}

auto constexpr help =
    "\n"
    "    Usage: x y *\n"
    "\n"
    "    Returns the product of the bottom two items on the stack (x * y)\n";

} // namespace multiply

namespace divide
{

bool impl(Calculator& calc)
{
    return two_arg_conv_op(
        calc, [](const auto& a, const auto& b) { return a / b; },
        std::tuple<mpz>{}, std::tuple<mpq>{}, std::tuple<mpq, mpf, mpc>{});
}

auto constexpr help =
    "\n"
    "    Usage: x y /\n"
    "\n"
    "    Returns the quotient of the bottom two items on the stack (x / y)\n";

} // namespace divide

namespace ceil
{

#ifdef USE_BASIC_TYPES
#define ceil_fn ceill
#else
#define ceil_fn boost::multiprecision::ceil
#endif

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                // complex adapter doesn't work with ceil
                mpf rp = ceil_fn(a.real());
                mpf ip = ceil_fn(a.imag());
                return mpc(rp, ip);
            }
            else if constexpr (std::is_same<decltype(a), const mpz&>::value)
            {
                // integers are already there
                return a;
            }
            else
            {
                return ceil_fn(a);
            }
        },
        std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
}

auto constexpr help =
    "\n"
    "    Usage: x ceil\n"
    "\n"
    "    Returns the smallest integer greater than the bottom "
    "item on the stack (round up)\n";

} // namespace ceil

namespace floor
{

#ifdef USE_BASIC_TYPES
#define floor_fn floorl
#else
#define floor_fn boost::multiprecision::floor
#endif

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                // complex adapter doesn't work with floor
                mpf rp = floor_fn(a.real());
                mpf ip = floor_fn(a.imag());
                return mpc(rp, ip);
            }
            else if constexpr (std::is_same<decltype(a), const mpz&>::value)
            {
                // integers are already there
                return a;
            }
            else
            {
                return floor_fn(a);
            }
        },
        std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x floor\n"
                      "\n"
                      "    Returns the smallest integer less than the bottom "
                      "item on the stack (round down)\n";

} // namespace floor

namespace round
{

#ifdef USE_BASIC_TYPES
#define round_fn roundl
#else
#define round_fn boost::multiprecision::round
#endif

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                // complex adapter doesn't work with round
                mpf rp = round_fn(a.real());
                mpf ip = round_fn(a.imag());
                return mpc(rp, ip);
            }
            else if constexpr (std::is_same<decltype(a), const mpz&>::value)
            {
                // integers are already there
                return a;
            }
            else
            {
                return round_fn(a);
            }
        },
        std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x round\n"
                      "\n"
                      "    Returns the nearest integer to the bottom "
                      "item on the stack (classic round)\n";

} // namespace round

namespace negate
{

bool impl(Calculator& calc)
{
    return one_arg_op(calc, [](const auto& a) { return decltype(a)(0) - a; });
}

auto constexpr help =
    "\n"
    "    Usage: x neg\n"
    "\n"
    "    Returns the negation of the bottom item on the stack (-x)\n";

} // namespace negate

namespace inverse
{

bool impl(Calculator& calc)
{
    return one_arg_op(calc, [](const auto& a) { return decltype(a)(1) / a; });
}

auto constexpr help = "\n"
                      "    Usage: x inv\n"
                      "\n"
                      "    Returns the multiplicative inverse of the bottom "
                      "item on the stack (1/x)\n";

} // namespace inverse

namespace divmod
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(
        calc, [](const auto& a, const auto& b) { return a % b; });
}

auto constexpr help = "\n"
                      "    Usage: x y %\n"
                      "\n"
                      "    Returns the division remainder of the bottom two "
                      "items on the stack (x mod y)\n";

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

auto constexpr help = "\n"
                      "    Usage: x y ^\n"
                      "\n"
                      "    Returns exponentiation of the bottom two items on "
                      "the stack, e.g., x raised to the y power (x^y)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x y z modexp\n"
    "\n"
    "    Returns modular exponentiation of the bottom three items on "
    "the stack, e.g., x raised to the y power mod z (x^y mod z)\n";

} // namespace modexp

} // namespace function

namespace functions
{

CalcFunction add = {function::add::help, function::add::impl};
CalcFunction subtract = {function::subtract::help, function::subtract::impl};
CalcFunction multiply = {function::multiply::help, function::multiply::impl};
CalcFunction divide = {function::divide::help, function::divide::impl};
CalcFunction floor = {function::floor::help, function::floor::impl};
CalcFunction ceil = {function::ceil::help, function::ceil::impl};
CalcFunction round = {function::round::help, function::round::impl};
CalcFunction negate = {function::negate::help, function::negate::impl};
CalcFunction inverse = {function::inverse::help, function::inverse::impl};
CalcFunction divmod = {function::divmod::help, function::divmod::impl};
CalcFunction power = {function::power::help, function::power::impl};
CalcFunction modexp = {function::modexp::help, function::modexp::impl};

} // namespace functions
