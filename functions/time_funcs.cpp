/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <chrono>
#include <function.hpp>

namespace function
{
namespace unix_ts
{

bool impl(Calculator& calc)
{
    time_ now(std::chrono::system_clock::now());
    calc.stack.emplace_front(now, calc.config.base, calc.config.fixed_bits,
                             calc.config.precision, calc.config.is_signed);
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: now\n"
    "\n"
    "    return a unix timestamp with sub-second precision based\n"
    "    on the system clock";

} // namespace unix_ts
} // namespace function

namespace functions
{

CalcFunction unix_ts = {function::unix_ts::help, function::unix_ts::impl};

}
