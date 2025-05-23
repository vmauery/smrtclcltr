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
namespace util
{

bool add_from_stack(Calculator& calc)
{
    return two_arg_limited_op<mpz, mpq, mpf, mpc, time_, matrix, list,
                              symbolic>(
        calc,
        [](const auto& a, const auto& b, const units::unit& ua,
           const units::unit& ub) -> std::tuple<numeric, units::unit> {
            if (ua != ub)
            {
                throw units_mismatch();
            }
            return {a + b, ua};
        });
}

bool multiply_from_stack(Calculator& calc)
{
    return two_arg_limited_op<mpz, mpq, mpf, mpc, time_, matrix, list,
                              symbolic>(
        calc,
        [](const auto& a, const auto& b, const units::unit& ua,
           const units::unit& ub) -> std::tuple<numeric, units::unit> {
            return {a * b, ua * ub};
        });
}

bool divide_from_stack(Calculator& calc)
{
    return two_arg_conv<ITypes<mpz>, OTypes<mpq>,
                        LTypes<mpq, mpf, mpc, time_, matrix, list, symbolic>>::
        op(calc,
           [](const auto& a, const auto& b, const units::unit& ua,
              const units::unit& ub) -> std::tuple<numeric, units::unit> {
               lg::debug("a: ({} (type {}))\n", a, DEBUG_TYPE(a));
               lg::debug("b: ({} (type {}))\n", b, DEBUG_TYPE(b));
               auto r = a / b;
               lg::debug("r: ({} (type {}))\n", r, DEBUG_TYPE(r));
               return {r, ua / ub};
           });
}

bool power_from_stack(Calculator& calc)
{
    return two_arg_conv<ITypes<mpq>, OTypes<mpf>,
                        LTypes<mpz, mpf, mpc, symbolic>>::
        op(calc,
           [](const auto& a, const auto& b, const units::unit& ua,
              const units::unit& ub) -> std::tuple<numeric, units::unit> {
               if (ub != units::unit())
               {
                   throw std::invalid_argument("cannot raise to a unit power");
               }
               if constexpr (same_type_v<mpc, decltype(b)>)
               {
                   if (ua != units::unit())
                   {
                       throw std::invalid_argument(
                           "cannot raise units to a complex power");
                   }
                   return {util::power_direct(a, b), ua};
               }
               else if constexpr (same_type_v<symbolic, decltype(a)> ||
                                  same_type_v<symbolic, decltype(b)>)
               {
                   return {pow_fn(symbolic{a}, symbolic{b}), ub};
               }
               else
               {
                   auto ret_unit = units::pow(ua, static_cast<mpf>(b));
                   return {util::power_direct(a, b), ret_unit};
               }
           });
}

bool inverse_from_stack(Calculator& calc)
{

    return one_arg_conv<ITypes<mpz>, OTypes<mpq>,
                        LTypes<mpq, mpf, mpc, matrix, symbolic>>::
        op(calc,
           [](const auto& a,
              const units::unit& ua) -> std::tuple<numeric, units::unit> {
               return {mpq(1, 1) / a, units::unit() / ua};
           });
}

} // namespace util

struct add : public CalcFunction
{
    const std::string& name() const final
    {
        static const std::string _name{"+"};
        return _name;
    }
    const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y +\n"
            "\n"
            "    Returns the sum of the bottom two items on the stack (x + y)\n"
            "    This can merge two lists into one or add to each item\n"
            // clang-format on
        };
        return _help;
    }
    bool op(Calculator& calc) const final
    {
        return util::add_from_stack(calc);
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
}; // struct add

struct subtract : public CalcFunction
{
    const std::string& name() const final
    {
        static const std::string _name{"-"};
        return _name;
    }
    const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y -\n"
            "\n"
            "    Returns the difference of the bottom two items "
            "on the stack (x - y)\n"
            // clang-format on
        };
        return _help;
    }
    bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz, mpq, mpf, mpc, time_, matrix, list,
                                  symbolic>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a - b, ua};
            });
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
};

struct multiply : public CalcFunction
{
    const std::string& name() const final
    {
        static const std::string _name{"*"};
        return _name;
    }
    const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y *\n"
            "\n"
            "    Returns the product of the bottom two items "
            "on the stack (x * y)\n"
            // clang-format on
        };
        return _help;
    }
    bool op(Calculator& calc) const final
    {
        return util::multiply_from_stack(calc);
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
};

struct divide : public CalcFunction
{
    const std::string& name() const final
    {
        static const std::string _name{"/"};
        return _name;
    }
    const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y /\n"
            "\n"
            "    Returns the quotient of the bottom two items "
            "on the stack (x / y)\n"
            // clang-format on
        };
        return _help;
    }
    bool op(Calculator& calc) const final
    {
        return util::divide_from_stack(calc);
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
};

struct percent_change : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"%ch"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y %ch\n"
            "\n"
            "    Returns the percent change from x to y\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<ITypes<mpz>, OTypes<mpq>,
                            LTypes<mpq, mpf, mpc, time_, symbolic>>::
            op(calc,
               [](const auto& a, const auto& b, const units::unit& ua,
                  const units::unit& ub) -> std::tuple<numeric, units::unit> {
                   if (ua != ub)
                   {
                       throw units_mismatch();
                   }
                   return {(b - a) / a, units::unit{}};
               });
    }
    int num_args() const final
    {
        return 2;
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

struct inverse : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"inv"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x inv\n"
            "\n"
            "    Returns the multiplicative inverse of the bottom "
            "item on the stack (1/x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return util::inverse_from_stack(calc);
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

struct divmod : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"%"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y %\n"
            "\n"
            "    Returns the division remainder of the bottom two "
            "items on the stack (x mod y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz, mpq, mpf, symbolic>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a % b, ua};
            });
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
};

struct power : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"^"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y ^\n"
            "\n"
            "    Returns exponentiation of the bottom two items on "
            "the stack, e.g., x raised to the y power (x^y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return util::power_from_stack(calc);
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::infix;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(add);
register_calc_fn(subtract);
register_calc_fn(multiply);
register_calc_fn(divide);
register_calc_fn(percent_change);
register_calc_fn(inverse);
register_calc_fn(divmod);
register_calc_fn(power);
