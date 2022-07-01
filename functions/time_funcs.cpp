/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <chrono>
#include <function.hpp>

namespace function
{

struct unix_ts : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"now"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: now\n"
            "\n"
            "    return a unix timestamp with sub-second precision based\n"
            "    on the system clock"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        time_ now(std::chrono::system_clock::now());
        calc.stack.emplace_front(now, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
    }
};

} // namespace function

register_calc_fn(unix_ts);
