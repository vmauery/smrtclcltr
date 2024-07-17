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

struct hyperbolic_sine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sinh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sinh\n"
            "\n"
            "    Returns the hyperbolic sine of the bottom item on "
            "the stack: sinh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return sinh_fn(a);
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

struct hyperbolic_cosine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cosh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x cosh\n"
            "\n"
            "    Returns the hyperbolic cosine of the bottom item on "
            "the stack: cosh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return cosh_fn(a);
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

struct hyperbolic_tangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"tanh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x tanh\n"
            "\n"
            "    Returns the hyperbolic tangent of the bottom item "
            "on the stack: tanh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op(
                                          calc, a, [](const auto& a) {
                                              return tanh_fn(a);
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

struct hyperbolic_arcsine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"asinh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x asinh\n"
            "\n"
            "    Returns the hyperbolic arcsine of the bottom item "
            "on the stack: asinh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return asinh_fn(a);
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

struct hyperbolic_arccosine : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acosh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acosh\n"
            "\n"
            "    Returns the hyperbolic arccosine of the bottom item "
            "on the stack: acosh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return acosh_fn(a);
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

struct hyperbolic_arctangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"atanh"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x atanh\n"
            "\n"
            "    Returns the hyperbolic arctangent of the bottom "
            "item on the stack: atanh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return atanh_fn(a);
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

struct hyperbolic_secant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sech"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sech\n"
            "\n"
            "    Returns the hyperbolic secant of the bottom item "
            "on the stack: sec(x) = 1/cosh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return mpf{1.0} / cosh_fn(a);
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

struct hyperbolic_cosecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"csch"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x csch\n"
            "\n"
            "    Returns the hyperbolic cosecant of the bottom item "
            "on the stack: csch(x) = 1/sinh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return mpf{1.0} / sinh_fn(a);
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

struct hyperbolic_cotangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"coth"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x coth\n"
            "\n"
            "    Returns the hyperbolic cotangent of the bottom item "
            "on the stack: coth(x) = cosh(x)/sinh(x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return cosh_fn(a) / sinh_fn(a);
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

struct hyperbolic_arcsecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"asech"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x asech\n"
            "\n"
            "    Returns the hyperbolic arcsecant of the bottom item "
            "on the stack: asech(x) = acosh(1/x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return acosh_fn(mpf{1} / a);
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

struct hyperbolic_arccosecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acsch"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acsch\n"
            "\n"
            "    Returns the hyperbolic cosecant of the bottom item "
            "on the stack: acsch(x) = asinh(1/x)\n"
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
                                          throw units_prohibited();
                                      }
                                      return scaled_trig_op_inv(
                                          calc, a, [](const auto& a) {
                                              return asinh_fn(mpf{1.0} / a);
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

struct hyperbolic_arccotangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acoth"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acoth\n"
            "\n"
            "    Returns the hyperbolic cotangent of the bottom item on the stack:"
            "              acoth(x) = atanh(1/x)      (x > 0)\n"
            "              acoth(x) = atanh(1/x) + pi (x < 0)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>,
                            LTypes<mpf, mpc>>::
            op(calc,
               [&calc](const auto& a, const units::unit& ua)
                   -> std::tuple<numeric, units::unit> {
                   if (ua != units::unit())
                   {
                       throw units_prohibited();
                   }
                   return scaled_trig_op_inv(calc, a, [](const auto& a) {
                       if constexpr (same_type_v<decltype(a), mpf>)
                       {
                           auto pi = boost::math::constants::pi<mpf>();
                           if (a > decltype(a){0})
                           {
                               return atanh_fn(mpf{1} / a);
                           }
                           return atanh_fn(mpf{1} / a) + pi;
                       }
                       else
                       {
                           return atanh_fn(mpf{1} / a);
                       }
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

register_calc_fn(hyperbolic_sine);
register_calc_fn(hyperbolic_cosine);
register_calc_fn(hyperbolic_tangent);
register_calc_fn(hyperbolic_arcsine);
register_calc_fn(hyperbolic_arccosine);
register_calc_fn(hyperbolic_arctangent);

register_calc_fn(hyperbolic_secant);
register_calc_fn(hyperbolic_cosecant);
register_calc_fn(hyperbolic_cotangent);
register_calc_fn(hyperbolic_arcsecant);
register_calc_fn(hyperbolic_arccosecant);
register_calc_fn(hyperbolic_arccotangent);
