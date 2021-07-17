/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace logarithm
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            mpf log10 = log(mpf{10});
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                return log(a) / log10;
            }
            else
            {
                if (a > decltype(a)(0))
                {
                    return log(mpf{a}) / log10;
                }
                else
                {
                    return log(mpc{a}) / log10;
                }
            }
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "base 10 logarithm of X";

} // namespace logarithm

namespace natural_logarithm
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                return log(a);
            }
            else
            {
                if (a > decltype(a)(0))
                {
                    return log(mpf{a});
                }
                else
                {
                    return log(mpc{a});
                }
            }
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "base e logarithm of X";

} // namespace natural_logarithm
} // namespace function

namespace functions
{

CalcFunction log = {function::logarithm::help, function::logarithm::impl};
CalcFunction ln = {function::natural_logarithm::help,
                   function::natural_logarithm::impl};

} // namespace functions
