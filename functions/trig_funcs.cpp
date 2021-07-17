/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{

namespace sine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return sin(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Sine of the first item on the stack";

} // namespace sine

namespace cosine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return cos(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Cosine of the first item on the stack";

} // namespace cosine

namespace tangent
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return tan(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Tangent of the first item on the stack";

} // namespace tangent

namespace arcsine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return asin(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Arcsine of the first item on the stack";

} // namespace arcsine

namespace arccosine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return acos(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Arccosine of the first item on the stack";

} // namespace arccosine

namespace arctangent
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return atan(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "returns Arctangent of the first item on the stack";

} // namespace arctangent

} // namespace function

namespace functions
{

CalcFunction sin = {function::sine::help, function::sine::impl};
CalcFunction cos = {function::cosine::help, function::cosine::impl};
CalcFunction tan = {function::tangent::help, function::tangent::impl};
CalcFunction asin = {function::arcsine::help, function::arcsine::impl};
CalcFunction acos = {function::arccosine::help, function::arccosine::impl};
CalcFunction atan = {function::arctangent::help, function::arctangent::impl};

} // namespace functions
