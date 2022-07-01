/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>
#include <functions/common.hpp>

namespace function
{

struct product : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"product"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ... x product\n"
            "\n"
            "    Returns the product of the "
            "bottom x items on the stack: Nx * Nx-1 * ... * N0\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        mpz* v = std::get_if<mpz>(&e.value());
        if (!v || (*v > 1000000000) ||
            (*v >= static_cast<mpz>(calc.stack.size())))
        {
            return false;
        }
        calc.stack.pop_front();
        size_t count = static_cast<size_t>(*v - 1);
        multiply mult{};
        for (; count > 0; count--)
        {
            if (!mult.op(calc))
            {
                return false;
            }
        }
        return true;
    }
};

} // namespace function

register_calc_fn(product);
