/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>
#include <units.hpp>

namespace smrty
{
namespace function
{

struct sine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sin"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sin\n"
            "\n"
            "    Returns the sine of the bottom item on the stack: sin(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return sin_fn(a);
                                          });
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

struct cosine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cos"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x cos\n"
            "\n"
            "    Returns the cosine of the bottom item on the stack: cos(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return cos_fn(a);
                                          });
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

struct tangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"tan"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x tan\n"
            "\n"
            "    Returns the tangent of the bottom item on the stack: tan(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return tan_fn(a);
                                          });
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

struct arcsine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"asin"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x asin\n"
            "\n"
            "    Returns the arcsine of the bottom item "
            "on the stack: asin(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return asin_fn(a);
                                          });
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

struct arccosine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acos"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acos\n"
            "\n"
            "    Returns the arccosine of the bottom item "
            "on the stack: acos(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return acos_fn(a);
                                          });
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

struct arctangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"atan"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x atan\n"
            "\n"
            "    Returns the arctangent of the bottom item "
            "on the stack: atan(x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<
            ITypes<mpz, mpq>, OTypes<mpf, mpf>,
            LTypes<mpf, mpc>>::op(calc,
                                  [&calc](const auto& a, const units::unit& ua)
                                      -> std::tuple<numeric, units::unit> {
                                      if (ua != units::unit())
                                      {
                                          throw std::invalid_argument(
                                              "values with units not allowed");
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return atan_fn(a);
                                          });
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

register_calc_fn(sine);
register_calc_fn(cosine);
register_calc_fn(tangent);
register_calc_fn(arcsine);
register_calc_fn(arccosine);
register_calc_fn(arctangent);
