/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace bitwise_and
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(
        calc, [](const auto& a, const auto& b) { return a & b; });
}

auto constexpr help = "bitwise AND of the first two items on the stack";

} // namespace bitwise_and

namespace bitwise_or
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(
        calc, [](const auto& a, const auto& b) { return a | b; });
}

auto constexpr help = "bitwise OR of the first two items on the stack";

} // namespace bitwise_or

namespace bitwise_xor
{

bool impl(Calculator& calc)
{
    return two_arg_limited_op<mpz>(
        calc, [](const auto& a, const auto& b) { return a % b; });
}

auto constexpr help = "bitwise XOR of the first two items on the stack";

} // namespace bitwise_xor

namespace bitwise_inv
{

bool impl(Calculator& calc)
{
    return one_arg_limited_op<mpz>(calc, [](const auto& a) { return ~a; });
}

auto constexpr help = "bitwise INV of the first two items on the stack";

} // namespace bitwise_inv

} // namespace function

namespace functions
{

CalcFunction bitwise_and = {function::bitwise_and::help,
                            function::bitwise_and::impl};
CalcFunction bitwise_or = {function::bitwise_or::help,
                           function::bitwise_or::impl};
CalcFunction bitwise_xor = {function::bitwise_xor::help,
                            function::bitwise_xor::impl};
CalcFunction bitwise_inv = {function::bitwise_inv::help,
                            function::bitwise_inv::impl};

} // namespace functions
