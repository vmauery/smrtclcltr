/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct boolean_and : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"and"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y and\n"
            "\n"
            "    Returns the boolean AND of the bottom two items on "
            "the stack (x and y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<
            ITypes<mpz, mpq, mpf, mpc>, OTypes<bool, bool, bool, bool>,
            LTypes<bool>>::op(calc,
                              [](bool a, bool b, const units::unit& ua,
                                 const units::unit& ub)
                                  -> std::tuple<numeric, units::unit> {
                                  if (ua != ub)
                                  {
                                      throw units_mismatch();
                                  }
                                  return {a && b, ua};
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
        return symbolic_op::none;
    }
};

struct boolean_or : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"or"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y or\n"
            "\n"
            "    Returns the boolean OR of the bottom "
            "two items on the stack (x or y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<
            ITypes<mpz, mpq, mpf, mpc>, OTypes<bool, bool, bool, bool>,
            LTypes<bool>>::op(calc,
                              [](bool a, bool b, const units::unit& ua,
                                 const units::unit& ub)
                                  -> std::tuple<numeric, units::unit> {
                                  if (ua != ub)
                                  {
                                      throw units_mismatch();
                                  }
                                  return {a || b, ua};
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
        return symbolic_op::none;
    }
};

struct boolean_xor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"x0r"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y x0r\n"
            "\n"
            "    Returns the boolean XOR of the bottom two items on "
            "the stack (x x0r y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<
            ITypes<mpz, mpq, mpf, mpc>, OTypes<bool, bool, bool, bool>,
            LTypes<bool>>::op(calc,
                              [](bool a, bool b, const units::unit& ua,
                                 const units::unit& ub)
                                  -> std::tuple<numeric, units::unit> {
                                  if (ua != ub)
                                  {
                                      throw units_mismatch();
                                  }
                                  // != is boolean equivalent of xor
                                  return {a != b, ua};
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
        return symbolic_op::none;
    }
};

struct boolean_negate : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"not"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x not\n"
            "\n"
            "    Returns the boolean negation of the bottom "
            "item on the stack (not x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq, mpf, mpc>, OTypes<bool, bool, bool, bool>,
            LTypes<bool>>::op(calc,
                              [](bool a, const units::unit& ua)
                                  -> std::tuple<numeric, units::unit> {
                                  return {!a, ua};
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
        return symbolic_op::prefix;
    }
};

struct less_than : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"<"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y <\n"
            "\n"
            "    Returns whether the next-to-bottom item is less than the "
            "bottom item\n"
            "    on the stack (x < y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // mpc not supported
        return two_arg_limited_op<bool, mpz, mpq, mpf>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a < b, ua};
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
        return symbolic_op::none;
    }
};

struct greater_than : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{">"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y >\n"
            "\n"
            "    Returns whether the next-to-bottom item is greater than the "
            "bottom item\n"
            "    on the stack (x > y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // mpc not supported
        return two_arg_limited_op<bool, mpz, mpq, mpf>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a > b, ua};
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
        return symbolic_op::none;
    }
};

struct less_than_or_eq : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"<="};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y <=\n"
            "\n"
            "    Returns whether the next-to-bottom item is less than "
            "or equal to the\n"
            "     bottom item on the stack (x <= y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // mpc not supported
        return two_arg_limited_op<bool, mpz, mpq, mpf>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a <= b, ua};
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
        return symbolic_op::none;
    }
};

struct greater_than_or_eq : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{">="};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y >=\n"
            "\n"
            "    Returns whether the next-to-bottom item is greater than "
            "or equal to the\n"
            "     bottom item on the stack (x >= y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // mpc not supported
        return two_arg_limited_op<bool, mpz, mpq, mpf>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a >= b, ua};
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
        return symbolic_op::none;
    }
};

struct equal : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"=="};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y ==\n"
            "\n"
            "    Returns whether the next-to-bottom item is equal to the\n"
            "     bottom item on the stack (x == y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // TODO: add support for more types (matrix, list, time_)
        return two_arg_limited_op<bool, mpz, mpq, mpf, mpc>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a == b, ua};
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
        return symbolic_op::none;
    }
};

struct not_equal : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"!="};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y !=\n"
            "\n"
            "    Returns whether the next-to-bottom item is not equal to the\n"
            "     bottom item on the stack (x != y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // TODO: add support for more types (matrix, list, time_)
        return two_arg_limited_op<bool, mpz, mpq, mpf, mpc>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw units_mismatch();
                }
                return {a != b, ua};
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
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(boolean_and);
register_calc_fn(boolean_or);
register_calc_fn(boolean_xor);
register_calc_fn(boolean_negate);
register_calc_fn(less_than);
register_calc_fn(greater_than);
register_calc_fn(less_than_or_eq);
register_calc_fn(greater_than_or_eq);
register_calc_fn(equal);
register_calc_fn(not_equal);
