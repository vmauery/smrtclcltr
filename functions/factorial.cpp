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

mpc gamma_mpc(const mpc& zp)
{
    // using Spouge's approximation
    // gamma(z+1) = (z+a)^(z+1/2)*e^(-z-a)*(c0 + sum(1,a-1,cn/(z+n)))
    // need to boost precision by 25% to result in full precision
    auto prec = default_precision;
    set_default_precision(prec * 5 / 4);

    // it would be possible to pre-calculate cn for any given precision
    // but this is already incredibly fast

    const mpf half{0.5l};
    const mpz one{1};
    const mpf two{2.0l};
    const mpf pi{boost::math::constants::pi<mpf>()};
    const mpz a{default_precision};
    const mpc z{zp - mpc{1}};
    mpc g{pow_fn(z + a, z + half) * exp_fn(-z - a)};
    mpc cn{sqrt_fn(two * pi)};
    mpc sum{cn};
    mpf sign{-1};
    for (mpz n = mpz{1}; n < a; n++)
    {
        sign = -sign;
        cn = (sign / mpf{factorial(n - one)}) * pow_fn(mpf{-n + a}, n - half) *
             exp_fn(mpf{-n + a});
        sum += cn / (z + n);
    }
    g *= sum;
    set_default_precision(prec);
    return g;
}

mpc factorial(const mpc& x)
{
    // non-int types get gamma treatment
    return gamma_mpc(x + mpc{1.0l});
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
        return one_arg_limited_op<mpz, mpf, mpq, mpc>(
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
