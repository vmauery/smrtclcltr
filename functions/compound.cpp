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
    std::vector<std::tuple<numeric, units::unit>> ret{};
    ret.reserve(m.size() + 2);
    ret.emplace_back(std::forward_as_tuple(mpz{m.cols}, units::unit{}));
    ret.emplace_back(std::forward_as_tuple(mpz{m.rows}, units::unit{}));
    for (const auto& v : m.values)
    {
        std::visit(
            [&ret](const auto& vv) {
                ret.emplace_back(std::forward_as_tuple(vv, units::unit{}));
            },
            v);
    }
    return ret;
}

std::vector<std::tuple<numeric, units::unit>>
    split(Calculator& calc, const mpc& c, const units::unit&)
{
    std::vector<std::tuple<numeric, units::unit>> ret{};
    ret.reserve(2);
    if (calc.config.mpc_mode == Calculator::e_mpc_mode::polar)
    {
        ret.emplace_back(std::forward_as_tuple(abs_fn(c), units::unit{}));
        ret.emplace_back(std::forward_as_tuple(mpf{atan2(c.real(), c.imag())},
                                               units::unit{}));
    }
    else
    {
        ret.emplace_back(std::forward_as_tuple(mpf{c.real()}, units::unit{}));
        ret.emplace_back(std::forward_as_tuple(mpf{c.imag()}, units::unit{}));
    }
    return ret;
}

std::vector<std::tuple<numeric, units::unit>> split(Calculator&, const mpq& q,
                                                    const units::unit&)
{
    std::vector<std::tuple<numeric, units::unit>> ret{};
    ret.reserve(2);
    ret.emplace_back(
        std::forward_as_tuple(helper::numerator(q), units::unit{}));
    ret.emplace_back(
        std::forward_as_tuple(helper::denominator(q), units::unit{}));
    return ret;
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
        return one_arg_limited_multi_return_op<matrix, mpc, mpq>(
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
            "           ... x y 2matrix\n"
            "\n"
            "    Returns a matrix from the items on the stack,\n"
            "    with cols=x and rows=y and x*y items\n"
            "    Items can be a single list of x*y items or\n"
            "    individual items on the stack\n"
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
        stack_entry& nx = calc.stack[1];
        stack_entry& ny = calc.stack[0];
        if ((nx.unit() != units::unit()) || (ny.unit() != units::unit()))
        {
            return false;
        }
        auto px = std::get_if<mpz>(&nx.value());
        auto py = std::get_if<mpz>(&ny.value());
        if (!px || !py)
        {
            return false;
        }
        size_t cols = static_cast<size_t>(*px);
        size_t rows = static_cast<size_t>(*py);
        size_t count = rows * cols;
        if (count == 0)
        {
            return false;
        }
        std::vector<mpx> items{};
        if (calc.stack.size() < (count + 2))
        {
            // check for list
            lg::error("list format not supported yet");
            return false;
        }
        else
        {
            items.reserve(count);
            for (size_t i = 0; i < count; i++)
            {
                stack_entry& a =
                    calc.stack[count - i - 1 + 2]; // offset for x,y
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
                        "Matrix requires integer, rational, float, or complex "
                        "types only");
                }
            }
            // remove x,y
            calc.stack.pop_front();
            calc.stack.pop_front();
            // remove all the items
            for (size_t i = 0; i < count; i++)
            {
                calc.stack.pop_front();
            }
        }
        matrix m(cols, rows, std::move(items));
        calc.stack.emplace_front(numeric{std::move(m)}, calc.config.base,
                                 calc.config.fixed_bits, calc.config.precision,
                                 calc.config.is_signed);
        return true;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(split);
register_calc_fn(Matrix);
