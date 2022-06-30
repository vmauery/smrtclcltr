/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

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
        stack_entry xe = calc.stack.front();
        calc.stack.pop_front();
        stack_entry ye = calc.stack.front();
        const mpz* x = std::get_if<mpz>(&xe.value());
        const mpz* y = std::get_if<mpz>(&ye.value());
        if (!x || !y)
        {
            calc.stack.push_front(xe);
            return false;
        }
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
            ve.value(v);
            calc.stack.emplace_front(
                v, calc.config.base, calc.config.fixed_bits,
                calc.config.precision, calc.config.is_signed);
            v += step;
        }
        return true;
    }
};

} // namespace function

register_calc_fn(range);
