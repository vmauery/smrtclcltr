/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{
struct _e : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"e"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
             "\n"
             "    Usage: e\n"
             "\n"
             "    Returns constant e (Euler's number)\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        stack_entry e(boost::math::constants::e<mpf>(), calc.config.base,
                      calc.config.fixed_bits, calc.config.precision,
                      calc.config.is_signed);
        calc.stack.push_front(std::move(e));
        return true;
    }
};

struct _pi : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"pi"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
             "\n"
             "    Usage: pi\n"
             "\n"
             "    Returns constant pi\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        stack_entry pi(boost::math::constants::pi<mpf>(), calc.config.base,
                       calc.config.fixed_bits, calc.config.precision,
                       calc.config.is_signed);
        calc.stack.push_front(std::move(pi));
        return true;
    }
};

struct _i : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"i"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: i\n"
            "\n"
            "    Returns constant i (square root of -1)\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        stack_entry i(mpc(0, 1), calc.config.base, calc.config.fixed_bits,
                      calc.config.precision, calc.config.is_signed);
        calc.stack.push_front(std::move(i));
        return true;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(_e);
register_calc_fn(_pi);
register_calc_fn(_i);
