/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>
#include <functions/common.hpp>

namespace smrty
{
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
    bool product_from_stack(Calculator& calc, const mpz& v) const
    {
        size_t count = static_cast<size_t>(v) - 1;
        calc.stack.pop_front();
        for (; count > 0; count--)
        {
            if (!util::multiply_from_stack(calc))
            {
                return false;
            }
        }
        return true;
    }
    bool product_from_list(Calculator& calc, const list& lst) const
    {
        list::element_type sum{mpz{1}};
        for (const auto& v : lst.values)
        {
            sum = sum * v;
        }
        calc.stack.pop_front();
        std::visit(
            [&](auto&& v) {
                calc.stack.emplace_front(
                    numeric{v}, calc.config.base, calc.config.fixed_bits,
                    calc.config.precision, calc.config.is_signed, calc.flags);
            },
            std::move(sum));
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 1)
        {
            return false;
        }
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v && (*v < static_cast<mpz>(calc.stack.size())))
        {
            return product_from_stack(calc, *v);
        }
        if (auto lp = std::get_if<list>(&e.value()); lp)
        {
            return product_from_list(calc, *lp);
        }
        return false;
    }
};
} // namespace function
} // namespace smrty

register_calc_fn(product);
