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

mpc factorial(const mpc&)
{
    throw std::invalid_argument("Not implemented for complex numbers");
}

mpz factorial(const mpz& x)
{
    if (x == 0)
    {
        return 1;
    }
    if (x < 0)
    {
        throw std::range_error("Undefined for integers x < 0");
    }
    mpz f = x;
    mpz n = x;
    while (--n > 1)
    {
        f *= n;
    }
    return f;
}

bool impl(Calculator& calc)
{
    return one_arg_op(calc, [](const auto& x) { return factorial(x); });
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
