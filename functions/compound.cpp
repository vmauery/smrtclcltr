/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <exception>
#include <function.hpp>

namespace smrty
{
namespace function
{

namespace util
{
std::vector<std::tuple<numeric, units::unit>>
    split(Calculator&, const matrix& m, const units::unit&)
{
    std::vector<std::tuple<numeric, units::unit>> ret{
        std::forward_as_tuple(list{m.values}, units::unit{}),
        std::forward_as_tuple(mpz{m.cols}, units::unit{}),
        std::forward_as_tuple(mpz{m.rows}, units::unit{})};
    return ret;
}

std::vector<std::tuple<numeric, units::unit>>
    split(Calculator&, const list& lst, const units::unit&)
{
    std::vector<std::tuple<numeric, units::unit>> ret{lst.size() + 1};
    size_t i = 0;
    for (const auto& v : lst.values)
    {
        ret[i++] = std::make_tuple(
            std::visit([](const auto& a) -> numeric { return a; }, v),
            units::unit{});
    }
    ret[i] = std::make_tuple(numeric{mpz{lst.size()}}, units::unit{});
    return ret;
}

std::vector<std::tuple<numeric, units::unit>>
    split(Calculator& calc, const mpc& c, const units::unit&)
{
    if (calc.config.mpc_mode == Calculator::e_mpc_mode::polar)
    {
        return {std::forward_as_tuple(abs_fn(c), units::unit{}),
                std::forward_as_tuple(mpf{atan2(c.real(), c.imag())},
                                      units::unit{})};
    }
    else
    {
        return {std::forward_as_tuple(mpf{c.real()}, units::unit{}),
                std::forward_as_tuple(mpf{c.imag()}, units::unit{})};
    }
}

std::vector<std::tuple<numeric, units::unit>> split(Calculator&, const mpq& q,
                                                    const units::unit&)
{
    return {std::forward_as_tuple(helper::numerator(q), units::unit{}),
            std::forward_as_tuple(helper::denominator(q), units::unit{})};
}

} // namespace util

struct split : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"split"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x split\n"
            "\n"
            "    Returns the underlying parts of the type for the bottom item one the stack\n"
            "    matrix -> cols rows {items}\n"
            "    list -> items\n"
            "    vector -> components\n"
            "    complex -> real imaginary / magnitude angle\n"
            "    rational -> numerator denominator\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_multi_return_op<matrix, list, mpc, mpq>(
            calc, [&calc](const auto& a, const units::unit& ua) {
                return util::split(calc, a, ua);
            });
    }
};

struct Matrix : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"2matrix"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: {...} x y 2matrix\n"
            "\n"
            "    Returns a matrix from items on the stack,\n"
            "    with cols=x and rows=y and a list of x*y items.\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 3)
        {
            return false;
        }
        stack_entry& nl = calc.stack[2];
        stack_entry& nx = calc.stack[1];
        stack_entry& ny = calc.stack[0];
        if ((nx.unit() != units::unit()) || (ny.unit() != units::unit()))
        {
            return false;
        }
        auto pl = const_cast<list*>(std::get_if<list>(&nl.value()));
        auto px = std::get_if<mpz>(&nx.value());
        auto py = std::get_if<mpz>(&ny.value());
        if (!pl || !px || !py)
        {
            return false;
        }
        size_t cols = static_cast<size_t>(*px);
        size_t rows = static_cast<size_t>(*py);
        matrix m(cols, rows, std::move(pl->values));
        // remove lst,x,y
        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.pop_front();
        calc.stack.emplace_front(numeric{m}, calc.config.base,
                                 calc.config.fixed_bits, calc.config.precision,
                                 calc.config.is_signed, calc.flags);
        return true;
    }
};

struct List : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"2list"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ... x 2list\n"
            "\n"
            "    Returns a list from the bottom x items on the stack.\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 2)
        {
            return false;
        }
        stack_entry& nc = calc.stack[0];
        if (nc.unit() != units::unit())
        {
            return false;
        }
        auto pc = std::get_if<mpz>(&nc.value());
        if (!pc)
        {
            return false;
        }
        size_t count = static_cast<size_t>(*pc);
        if (count == 0)
        {
            return false;
        }
        if (calc.stack.size() < (count + 1))
        {
            return false;
        }
        std::vector<mpx> items{};
        items.reserve(count);
        for (size_t i = 0; i < count; i++)
        {
            stack_entry& a = calc.stack[count - i];
            if (auto pz = std::get_if<mpz>(&a.value()); pz)
            {
                lg::debug("mpz\n");
                items.emplace_back(*pz);
            }
            else if (auto pq = std::get_if<mpq>(&a.value()); pq)
            {
                lg::debug("mpq\n");
                items.emplace_back(*pq);
            }
            else if (auto pf = std::get_if<mpf>(&a.value()); pf)
            {
                lg::debug("mpf\n");
                items.emplace_back(*pf);
            }
            else if (auto pc = std::get_if<mpc>(&a.value()); pc)
            {
                lg::debug("mpc\n");
                items.emplace_back(*pc);
            }
            else
            {
                throw std::invalid_argument(
                    "List requires integer, rational, float, or complex "
                    "types only");
            }
        }
        // remove count
        calc.stack.pop_front();
        // remove all the items
        for (size_t i = 0; i < count; i++)
        {
            calc.stack.pop_front();
        }
        calc.stack.emplace_front(numeric{list{std::move(items)}},
                                 calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(split);
register_calc_fn(Matrix);
