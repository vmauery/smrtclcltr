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

struct ceil : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"ceil"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x ceil\n"
            "\n"
            "    Returns the smallest integer greater than the bottom "
            "item on the stack (round up)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpq>, OTypes<mpf>, LTypes<mpz, mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit& ua) -> std::tuple<numeric, units::unit> {
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       // complex adapter doesn't work with ceil
                       mpf rp{ceil_fn(a.real())};
                       mpf ip{ceil_fn(a.imag())};
                       return {mpc(rp, ip), ua};
                   }
                   else if constexpr (std::is_same<decltype(a),
                                                   const mpz&>::value)
                   {
                       // integers are already there
                       return {a, ua};
                   }
                   else
                   {
                       return {ceil_fn(a), ua};
                   }
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

struct floor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"floor"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x floor\n"
            "\n"
            "    Returns the smallest integer less than the bottom "
            "item on the stack (round down)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpq>, OTypes<mpf>, LTypes<mpz, mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit& ua) -> std::tuple<numeric, units::unit> {
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       // complex adapter doesn't work with floor
                       mpf rp{floor_fn(a.real())};
                       mpf ip{floor_fn(a.imag())};
                       return {mpc(rp, ip), ua};
                   }
                   else if constexpr (std::is_same<decltype(a),
                                                   const mpz&>::value)
                   {
                       // integers are already there
                       return {a, ua};
                   }
                   else
                   {
                       return {floor_fn(a), ua};
                   }
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

struct round : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"round"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x round\n"
            "\n"
            "    Returns the nearest integer to the bottom "
            "item on the stack (classic round)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpq>, OTypes<mpf>, LTypes<mpz, mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit& ua) -> std::tuple<numeric, units::unit> {
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       // complex adapter doesn't work with round
                       mpf rp{round_fn(a.real())};
                       mpf ip{round_fn(a.imag())};
                       return {mpc(rp, ip), ua};
                   }
                   else if constexpr (std::is_same<decltype(a),
                                                   const mpz&>::value)
                   {
                       // integers are already there
                       return {a, ua};
                   }
                   else
                   {
                       return {round_fn(a), ua};
                   }
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

register_calc_fn(ceil);
register_calc_fn(floor);
register_calc_fn(round);
