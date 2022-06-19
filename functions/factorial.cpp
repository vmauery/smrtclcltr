/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/math/special_functions/gamma.hpp>
#include <exception>
#include <function.hpp>
namespace function
{
namespace factorial
{

mpf factorial(const mpf& x)
{
    // non-int types get gamma treatment
    return boost::math::tgamma(x + 1);
}

mpf factorial(const mpq& x)
{
    mpf xp(x + 1);
    // non-int types get gamma treatment
    return boost::math::tgamma(xp);
}

mpz bin_split_factorial(const mpz& a, const mpz& b)
{
    mpz d = a - b;
    if (d <= 0)
        return 1;
    if (d < 4)
    {
        if (d == 1)
        {
            return a;
        }
        if (d == 2)
        {
            return a * (a - 1);
        }
        // d == 3
        return a * (a - 1) * (a - 2);
    }
    // all other values >= 4
    mpz m = (a + b) / 2;
    return bin_split_factorial(a, m) * bin_split_factorial(m, b);
}

mpz factorial(const mpz& x)
{
    if (x < 0)
    {
        throw std::range_error("Undefined for integers x < 0");
    }
    if (x < 2)
    {
        return 1;
    }
    return bin_split_factorial(x, 0);
}

bool impl(Calculator& calc)
{
    return one_arg_limited_op<mpz, mpf, mpq>(
        calc, [](const auto& x) { return factorial(x); });
}

auto constexpr help =
    "\n"
    "    Usage: x !\n"
    "\n"
    "    Returns the factorial of the bottom item on the stack (x!)\n";

} // namespace factorial
} // namespace function

namespace functions
{

CalcFunction factorial = {function::factorial::help, function::factorial::impl};

}
