/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace constant
{
namespace _e
{

bool impl(Calculator& calc)
{
    stack_entry e(boost::math::constants::e<mpf>(), calc.config.base,
                  calc.config.fixed_bits, calc.config.precision,
                  calc.config.is_signed);
    calc.stack.push_front(std::move(e));
    return true;
}

auto constexpr help = "pushes constant e onto the stack";

} // namespace _e

namespace _pi
{

bool impl(Calculator& calc)
{
    stack_entry pi(boost::math::constants::pi<mpf>(), calc.config.base,
                   calc.config.fixed_bits, calc.config.precision,
                   calc.config.is_signed);
    calc.stack.push_front(std::move(pi));
    return true;
}

auto constexpr help = "pushes constant Pi onto the stack";

} // namespace _pi

namespace _i
{

bool impl(Calculator& calc)
{
    stack_entry i(mpc(0, 1), calc.config.base, calc.config.fixed_bits,
                  calc.config.precision, calc.config.is_signed);
    calc.stack.push_front(std::move(i));
    return true;
}

auto constexpr help = "pushes constant i onto the stack";

} // namespace _i

} // namespace constant

namespace constants
{

CalcFunction e = {constant::_e::help, constant::_e::impl};
CalcFunction pi = {constant::_pi::help, constant::_pi::impl};
CalcFunction i = {constant::_i::help, constant::_i::impl};

} // namespace constants
