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
                      calc.config.is_signed, calc.flags);
        calc.stack.push_front(std::move(e));
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::prefix;
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
                       calc.config.is_signed, calc.flags);
        calc.stack.push_front(std::move(pi));
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::prefix;
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
                      calc.config.precision, calc.config.is_signed, calc.flags);
        calc.stack.push_front(std::move(i));
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::prefix;
    }
};

struct _j : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"j"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: j\n"
            "\n"
            "    Returns constant j (square root of -1)\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        stack_entry i(mpc(0, 1), calc.config.base, calc.config.fixed_bits,
                      calc.config.precision, calc.config.is_signed, calc.flags);
        calc.stack.push_front(std::move(i));
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::prefix;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(_e);
register_calc_fn(_pi);
register_calc_fn(_i);
register_calc_fn(_j);
