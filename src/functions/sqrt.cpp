/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct square_root : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sqrt"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sqrt\n"
            "\n"
            "    Returns the square root of the bottom item on the stack: "
            "sqrt(x)\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>,
                            LTypes<mpf, mpc, symbolic>>::
            op(calc,
               [](const auto& a,
                  const units::unit& ua) -> std::tuple<numeric, units::unit> {
                   if constexpr (same_type_v<decltype(a), symbolic> ||
                                 same_type_v<decltype(a), mpc>)
                   {
                       return {sqrt_fn(a), units::pow(ua, mpf{0.5l})};
                   }
                   else
                   {
                       if (a >= decltype(a){0.0l})
                       {
                           return {sqrt_fn(mpf{a}), units::pow(ua, mpf{0.5l})};
                       }
                       else
                       {
                           return {sqrt_fn(mpc{a}), units::pow(ua, mpf{0.5l})};
                       }
                   }
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
        return symbolic_op::paren;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(square_root);
