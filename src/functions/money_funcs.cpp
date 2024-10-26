/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>
#include <functions/common.hpp>

namespace smrty
{

namespace function
{

struct future_value : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"fv"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: p r n d fv\n"
            "\n"
            "    Returns the future value using the inputs:\n"
            "    p = initial principal\n"
            "    r = interest rate per period\n"
            "    n = number of periods\n"
            "    d = periodic deposit/(-withdrawal) amount\n"
            "\n"
            "                             (1 + r)^n - 1\n"
            "    fv = p * (1 + r)^n + d * -------------\n"
            "                                 r\n"
            "\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return four_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>, LTypes<mpf>>::
            // clang-format is doing some weird stuff here
            op(calc,
               [](const mpf& p, const mpf& r, const mpf& n,
                  const mpf& d) -> numeric {
                   mpf rate = pow_fn(mpf{1.0} + r, n);
                   return p * rate + d * (rate - mpf{1.0}) / r;
               });
    }
    int num_args() const final
    {
        return 4;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct present_value : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"pv"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: p r n d pv\n"
            "\n"
            "    Returns the present value using the inputs:\n"
            "    p = future principal\n"
            "    r = interest rate per period\n"
            "    n = number of periods\n"
            "    d = periodic deposit/(-withdrawal) amount\n"
            "\n"
            "                              1 - (1 + r)^-n\n"
            "    pv = p * (1 + r)^-n + d * --------------\n"
            "                                   r\n"
            "\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return four_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>, LTypes<mpf>>::
            // clang-format is doing some weird stuff here
            op(calc,
               [](const mpf& p, const mpf& r, const mpf& n,
                  const mpf& d) -> numeric {
                   mpf rate = pow_fn(mpf{1.0} + r, -n);
                   return p * rate + d * (mpf{1.0} - rate) / r;
               });
    }
    int num_args() const final
    {
        return 4;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct payment : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"pmt"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: p r n pmt\n"
            "\n"
            "    Returns the payment amount using the inputs:\n"
            "    p = borrowed principal\n"
            "    r = interest rate per period\n"
            "    n = number of periods\n"
            "\n"
            "              p * r\n"
            "    pmt = --------------\n"
            "          1 - (1 + r)^-n\n"
            "\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return three_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>, LTypes<mpf>>::
            op(calc, [](const mpf& p, const mpf& r, const mpf& n) -> numeric {
                return p * r / (mpf{1.0} - pow_fn(mpf{1.0} + r, -n));
            });
    }
    int num_args() const final
    {
        return 3;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(future_value);
register_calc_fn(present_value);
register_calc_fn(payment);
