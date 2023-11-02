/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>

namespace smrty
{
namespace function
{

mpz pow(const mpz& base, const mpz& exponent, const mpz& modulus)
{
    mpz b(base), e(exponent), m(modulus);
    mpz result{1};
    while (e > zero)
    {
        if (e & one)
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

struct modexp : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"modexp"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y z modexp\n"
            "\n"
            "    Returns modular exponentiation of the bottom three items on "
            "    the stack, e.g., x raised to the y power mod z (x^y mod z)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return three_arg_limited_op(
            calc,
            [](const auto& a, const auto& b, const auto& c,
               const units::unit& ua, const units::unit& ub,
               const units::unit& uc) -> std::tuple<numeric, units::unit> {
                if (ua != ub || ua != uc || ua != units::unit())
                {
                    throw std::invalid_argument("units not permitted");
                }
                return {pow(a, b, c), ua};
            },
            std::tuple<mpz>{});
    }
};

struct modinv : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"modinv"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y modinv\n"
            "\n"
            "    Returns the multiplicative modular inverse of x (mod y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                mpz t(0);
                mpz newt(1);
                mpz r = b;
                mpz newr = a;
                while (newr != 0)
                {
                    mpz quotient = r / newr;
                    std::tie(t, newt) =
                        std::make_tuple(newt, t - quotient * newt);
                    std::tie(r, newr) =
                        std::make_tuple(newr, r - quotient * newr);
                }
                if (r > 1)
                {
                    throw std::domain_error("x is not invertible");
                }
                if (t < 0)
                {
                    t += b;
                }
                return {t, ua};
            });
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(modexp);
register_calc_fn(modinv);
