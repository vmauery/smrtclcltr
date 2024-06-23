/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct range : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"range"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y range\n"
            "\n"
            "    Returns the numbers in the range of [x,y) of the "
            "bottom two items on the stack: x x+1 ... y-1\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry xe = calc.stack[0];
        stack_entry ye = calc.stack[1];
        const mpz* x = std::get_if<mpz>(&xe.value());
        const mpz* y = std::get_if<mpz>(&ye.value());
        if (!x || !y)
        {
            throw std::runtime_error("range requires two integers");
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz step, count;
        if (*x > *y)
        {
            count = *x - *y;
            step = 1;
        }
        else
        {
            count = *y - *x;
            step = -1;
        }
        mpz v = *y;
        for (; count > 0; count--)
        {
            stack_entry ve;
            ve.value(v, calc.flags);
            calc.stack.emplace_front(
                v, calc.config.base, calc.config.fixed_bits,
                calc.config.precision, calc.config.is_signed, calc.flags);
            v += step;
        }
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return -1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(range);
