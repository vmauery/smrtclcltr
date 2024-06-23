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

struct absval : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"abs"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x abs\n"
            "\n"
            "    Returns absolute value of x: |x|\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<mpz, mpq, mpf, mpc, matrix>(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                return {abs_fn(a), ua};
            });
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct negate : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"neg"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x neg\n"
            "\n"
            "    Returns the negation of the bottom item on the stack (-x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_op(calc,
                          [](const auto& a, const units::unit& ua)
                              -> std::tuple<numeric, units::unit> {
                              return {mpz{-1} * a, ua};
                          });
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(absval);
register_calc_fn(negate);
