/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace square_root
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [](const auto& a) -> numeric {
            if constexpr (std::is_same<decltype(a), const mpc&>::value)
            {
                return sqrt(a);
            }
            else
            {
                if (a >= decltype(a)(0))
                {
                    return sqrt(mpf{a});
                }
                else
                {
                    return sqrt(mpc{a});
                }
            }
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help =
    "\n"
    "    Usage: x sqrt\n"
    "\n"
    "    Returns the square root of the bottom item on the stack: sqrt(x)\n";

} // namespace square_root
} // namespace function

namespace functions
{

CalcFunction sqrt = {function::square_root::help, function::square_root::impl};

}
