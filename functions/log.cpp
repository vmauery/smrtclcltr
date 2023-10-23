/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct logarithm : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"log"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x log\n"
            "\n"
            "    Returns the base-10 logarithm of the "
            "bottom item on the stack: log10(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>,
                            LTypes<mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit&) -> std::tuple<numeric, units::unit> {
                   mpf log10 = log(mpf{10});
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       return {log(a) / log10, units::unit()};
                   }
                   else
                   {
                       if (a > decltype(a)(0))
                       {
                           return {log(mpf{a}) / log10, units::unit()};
                       }
                       else
                       {
                           return {log(mpc{a}) / log10, units::unit()};
                       }
                   }
               });
    }
};

struct natural_logarithm : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"ln"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x ln\n"
            "\n"
            "    Returns the base-e logarithm of the "
            "bottom item on the stack: ln(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>,
                            LTypes<mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit&) -> std::tuple<numeric, units::unit> {
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       return {log(a), units::unit()};
                   }
                   else
                   {
                       if (a > decltype(a)(0))
                       {
                           return {log(mpf{a}), units::unit()};
                       }
                       else
                       {
                           return {log(mpc{a}), units::unit()};
                       }
                   }
               });
    }
};

struct log_base_two : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"log2"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x log2\n"
            "\n"
            "    Returns the base-2 logarithm of the "
            "bottom item on the stack: log2(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>,
                            LTypes<mpf, mpc>>::
            op(calc,
               [](const auto& a,
                  const units::unit&) -> std::tuple<numeric, units::unit> {
                   mpf log2 = log(mpf{2});
                   if constexpr (std::is_same<decltype(a), const mpc&>::value)
                   {
                       return {log(a) / log2, units::unit()};
                   }
                   else
                   {
                       if (a > decltype(a)(0))
                       {
                           return {log(mpf{a}) / log2, units::unit()};
                       }
                       else
                       {
                           return {log(mpc{a}) / log2, units::unit()};
                       }
                   }
               });
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(logarithm);
register_calc_fn(natural_logarithm);
register_calc_fn(log_base_two);
