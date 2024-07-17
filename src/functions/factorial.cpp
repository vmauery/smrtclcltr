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

mpz gamma(const mpz& x)
{
    return factorial(x - mpz{1});
}

mpf gamma(const mpq& x)
{
    return gamma_fn(mpf{x});
}

mpf gamma(const mpf& x)
{
    return gamma_fn(x);
}

mpc gamma(const mpc& x)
{
    return gamma_mpc(x);
}

mpf zeta(const mpf& x)
{
    return zeta_fn(x);
}

mpc d_kn(const mpz& k, const mpz& n)
{
    mpz sum{0};
    mpz one{1};
    mpz two{2};
    for (mpz i = 0; i < (k + one); i++)
    {
        mpz four_pow_i = one << (2 * static_cast<long long>(i));
        sum += (factorial(n + i - one) * four_pow_i) /
               (factorial(n - i) * factorial(two * i));
    }
    return mpc{mpf{n * sum}};
}

mpc zeta(const mpc& x)
{
    // implemented from "An Efficient Algorithm for the Riemann Zeta Function"
    // by Peter Borwein (algorithm 2):
    // http://www.cecm.sfu.ca/personal/pborwein/PAPERS/P155.pdf

    // provide a little extra precision to meet needs
    auto prec = default_precision;
    set_default_precision(prec * 1.1);

    // n = ceil(log[base(3+sqrt(8))](1.36*10^prec/abs((1-2^(1-x))*gamma(x))))
    auto n = static_cast<mpz>(
        ceil_fn(
            log_fn(mpf{1.36} * pow_fn(mpf{10}, prec) /
                   abs_fn((mpc{1} - pow_fn(mpc{2}, mpc{1} - x)) * gamma(x)))) /
        log_fn(mpf{3} + sqrt_fn(mpf{8})));
    lg::debug("zeta using {} iterations\n", n);

    mpc sum{0};
    mpz one{1};
    mpc sign{1};
    mpc dn = d_kn(n, n);
    for (mpz k = 0; k < n; k++)
    {
        sum += sign * (d_kn(k, n) - dn) / pow_fn(mpc{k + one}, x);
        sign = -sign;
    }
    // final product
    mpc prefix = -dn * (mpc{1} - pow_fn(mpc{2}, (mpc{1} - x)));
    mpc z = sum / prefix;

    set_default_precision(prec);
    return z;
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
                    throw units_prohibited();
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

struct gamma : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"gamma"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x gamma\n"
            "\n"
            "    Returns gamma(x) of the bottom item on the stack x\n"
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
                    throw units_prohibited();
                }
                return {util::gamma(a), ua};
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

struct zeta : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"zeta"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x zeta\n"
            "\n"
            "    Returns Riemann-zeta(x) of the bottom item on the stack x\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw units_prohibited();
                                      }
                                      return {util::zeta(a), ua};
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
register_calc_fn(gamma);
register_calc_fn(zeta);
