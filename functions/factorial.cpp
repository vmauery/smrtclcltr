/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <exception>
#include <function.hpp>

namespace smrty
{
namespace function
{

namespace util
{
mpf factorial(const mpf& x)
{
    // non-int types get gamma treatment
    return gamma_fn(x + mpf{1.0l});
}

mpf factorial(const mpq& x)
{
    mpf xp = static_cast<mpf>(x + one);
    // non-int types get gamma treatment
    return gamma_fn(xp);
}

mpz bin_split_factorial(const mpz& a, const mpz& b)
{
    mpz d = a - b;
    if (d <= 0)
        return 1;
    if (d < 4)
    {
        if (d == one)
        {
            return a;
        }
        if (d == two)
        {
            return a * (a - one);
        }
        // d == 3
        return a * (a - one) * (a - two);
    }
    // all other values >= 4
    mpz m = (a + b) / two;
    return bin_split_factorial(a, m) * bin_split_factorial(m, b);
}

mpz factorial(const mpz& x)
{
    if (x < zero)
    {
        throw std::range_error("Undefined for integers x < 0");
    }
    if (x < two)
    {
        return one;
    }
    return bin_split_factorial(x, zero);
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
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if (ua != units::unit())
                {
                    throw std::invalid_argument("units not permitted");
                }
                return {util::factorial(a), ua};
            });
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::postfix;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(factorial);
