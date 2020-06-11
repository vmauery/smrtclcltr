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

auto constexpr help =
    "\n"
    "    Usage: x sin\n"
    "\n"
    "    Returns the sine of the bottom item on the stack: sin(x)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x cos\n"
    "\n"
    "    Returns the cosine of the bottom item on the stack: cos(x)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x tan\n"
    "\n"
    "    Returns the tangent of the bottom item on the stack: tan(x)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x asin\n"
    "\n"
    "    Returns the arcsine of the bottom item on the stack: asin(x)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x acos\n"
    "\n"
    "    Returns the arccosine of the bottom item on the stack: acos(x)\n";

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

auto constexpr help =
    "\n"
    "    Usage: x atan\n"
    "\n"
    "    Returns the arctangent of the bottom item on the stack: atan(x)\n";

} // namespace arctangent

namespace hyperbolic
{

namespace sine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return sinh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x sinh\n"
                      "\n"
                      "    Returns the hyperbolic sine of the bottom item on "
                      "the stack: sinh(x)\n";

} // namespace sine

namespace cosine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return cosh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x cosh\n"
                      "\n"
                      "    Returns the hyperbolic cosine of the bottom item on "
                      "the stack: cosh(x)\n";

} // namespace cosine

namespace tangent
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op(calc, a,
                                  [](const auto& a) { return tanh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x tanh\n"
                      "\n"
                      "    Returns the hyperbolic tangent of the bottom item "
                      "on the stack: tanh(x)\n";

} // namespace tangent

namespace arcsine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return asinh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x asinh\n"
                      "\n"
                      "    Returns the hyperbolic arcsine of the bottom item "
                      "on the stack: asinh(x)\n";

} // namespace arcsine

namespace arccosine
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return acosh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x acosh\n"
                      "\n"
                      "    Returns the hyperbolic arccosine of the bottom item "
                      "on the stack: acosh(x)\n";

} // namespace arccosine

namespace arctangent
{

bool impl(Calculator& calc)
{
    return one_arg_conv_op(
        calc,
        [&calc](const auto& a) -> numeric {
            return scaled_trig_op_inv(calc, a,
                                      [](const auto& a) { return atanh(a); });
        },
        std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{}, std::tuple<mpf, mpc>{});
}

auto constexpr help = "\n"
                      "    Usage: x atanh\n"
                      "\n"
                      "    Returns the hyperbolic arctangent of the bottom "
                      "item on the stack: atanh(x)\n";

} // namespace arctangent

} // namespace hyperbolic

} // namespace function

namespace functions
{

CalcFunction sin = {function::sine::help, function::sine::impl};
CalcFunction cos = {function::cosine::help, function::cosine::impl};
CalcFunction tan = {function::tangent::help, function::tangent::impl};
CalcFunction asin = {function::arcsine::help, function::arcsine::impl};
CalcFunction acos = {function::arccosine::help, function::arccosine::impl};
CalcFunction atan = {function::arctangent::help, function::arctangent::impl};

CalcFunction sinh = {function::hyperbolic::sine::help,
                     function::hyperbolic::sine::impl};
CalcFunction cosh = {function::hyperbolic::cosine::help,
                     function::hyperbolic::cosine::impl};
CalcFunction tanh = {function::hyperbolic::tangent::help,
                     function::hyperbolic::tangent::impl};
CalcFunction asinh = {function::hyperbolic::arcsine::help,
                      function::hyperbolic::arcsine::impl};
CalcFunction acosh = {function::hyperbolic::arccosine::help,
                      function::hyperbolic::arccosine::impl};
CalcFunction atanh = {function::hyperbolic::arctangent::help,
                      function::hyperbolic::arctangent::impl};

} // namespace functions
