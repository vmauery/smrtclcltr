/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/math/special_functions/gamma.hpp>
#include <exception>
#include <function.hpp>
namespace function
{

namespace util
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
} //  namespace util

struct factorial : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"!"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x !\n"
            "\n"
            "    Returns the factorial of the bottom item on the stack (x!)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<mpz, mpf, mpq>(
            calc, [](const auto& x) { return util::factorial(x); });
    }
};

} // namespace function

register_calc_fn(factorial);
