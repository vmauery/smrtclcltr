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

struct adjoint : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"adjoint"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x adjoint\n"
            "\n"
            "    Returns the adjoint of the matrix x\n"
            "    calculated by swapping rows and columns\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<matrix>(
            calc,
            [](const matrix& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if (ua != units::unit())
                {
                    throw std::invalid_argument("units not permitted");
                }
                numeric inv{a.adjoint()};
                return {inv, ua};
            });
    }
};

struct determinant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"det"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x det\n"
            "\n"
            "    Returns the determinant of the bottom matrix on the stack: |x|\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<matrix>(
            calc,
            [](const matrix& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if (ua != units::unit())
                {
                    throw std::invalid_argument("units not permitted");
                }
                numeric det = std::visit(
                    [](const auto& a) -> numeric { return a; }, a.det());
                return {det, ua};
            });
    }
};

struct eye : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"eye"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x eye\n"
            "\n"
            "    Returns the an identity matrix of rank x\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<mpz>(
            calc,
            [](const mpz& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if (ua != units::unit())
                {
                    throw std::invalid_argument("units not permitted");
                }
                return {matrix::I(static_cast<size_t>(a)), ua};
            });
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(adjoint);
register_calc_fn(determinant);
register_calc_fn(eye);
