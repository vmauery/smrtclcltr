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
        return one_arg_conv_op(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    return {sqrt(a), units::pow(ua, 0.5l)};
                }
                else
                {
                    if (a >= decltype(a)(0))
                    {
                        return {sqrt(mpf{a}), units::pow(ua, 0.5l)};
                    }
                    else
                    {
                        return {sqrt(mpc{a}), units::pow(ua, 0.5l)};
                    }
                }
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(square_root);
