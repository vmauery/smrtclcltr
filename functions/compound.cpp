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
        ret.emplace_back(std::forward_as_tuple(v, units::unit{}));
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
        ret.emplace_back(std::forward_as_tuple(abs(c), units::unit{}));
        ret.emplace_back(
            std::forward_as_tuple(atan2(c.real(), c.imag()), units::unit{}));
    }
    else
    {
        ret.emplace_back(std::forward_as_tuple(c.real(), units::unit{}));
        ret.emplace_back(std::forward_as_tuple(c.imag(), units::unit{}));
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

} // namespace function
} // namespace smrty

register_calc_fn(split);
