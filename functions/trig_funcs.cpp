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
                                          throw units_prohibited();
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
                                          throw units_prohibited();
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
                                          throw units_prohibited();
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
                                          throw units_prohibited();
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
                                          throw units_prohibited();
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
                                          throw units_prohibited();
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

struct arctangent2 : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"atan2"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y atan2\n"
            "\n"
            "    Returns the arctangent of the bottom two items on the stack:\n"
            "          atan2(y, x) = atan(y/x)         if x > 0\n"
            "          atan2(y, x) = atan(y/x) + pi    if x < 0, y >= 0\n"
            "          atan2(y, x) = atan(y/x) - pi    if x < 0, y < 0\n"
            "          atan2(y, x) = pi/2              if x = 0, y > 0\n"
            "          atan2(y, x) = -pi/2             if x = 0, y < 0\n"
            "          atan2(y, x) = undefined         if x = 0, y = 0\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<ITypes<mpz, mpq>, OTypes<mpf, mpf>, LTypes<mpf>>::
            op(calc,
               [&calc](
                   const auto& a, const auto& b, const units::unit& ua,
                   const units::unit& ub) -> std::tuple<numeric, units::unit> {
                   if (ua != units::unit() || ub != units::unit())
                   {
                       throw units_prohibited();
                   }
                   return scaled_trig_two_arg_op_inv(
                       calc, a, b, [](const auto& a, const auto& b) {
                           auto pi = boost::math::constants::pi<mpf>();
                           auto zero = mpf{0};
                           auto pos = mpf{1};
                           auto neg = mpf{-1};
                           if (a == zero)
                           {
                               if (b == zero)
                               {
                                   throw std::domain_error(
                                       "atan2(0, 0) is undefined");
                               }
                               auto two = mpf{2};
                               return (b < zero ? neg : pos) * pi / two;
                           }
                           auto v = atan_fn(b / a);
                           if (a < zero)
                           {
                               return v + (b < zero ? pos : pos) * pi;
                           }
                           return v;
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

struct secant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sec"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sec\n"
            "\n"
            "    Returns the secant of the bottom item "
            "on the stack: sec(x) = 1/cos(x)\n"
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
                                              return mpf{1} / cos_fn(a);
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

struct cosecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"csc"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x csc\n"
            "\n"
            "    Returns the cosecant of the bottom item "
            "on the stack: csc(x) = 1/sin(x)\n"
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
                                              return mpf{1} / sin_fn(a);
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

struct cotangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cot"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x cot\n"
            "\n"
            "    Returns the cotangent of the bottom item "
            "on the stack: cot(x) = cos(x)/sin(x)\n"
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
                                              return cos_fn(a) / sin_fn(a);
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

struct arcsecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"asec"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x asec\n"
            "\n"
            "    Returns the arcsecant of the bottom item "
            "on the stack: asec(x) = acos(1/x)\n"
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
                                              return acos_fn(mpf{1} / a);
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

struct arccosecant : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acsc"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acsc\n"
            "\n"
            "    Returns the arccosecant of the bottom item "
            "on the stack: acsc(x) = asin(1/x)\n"
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
                                              return asin_fn(mpf{1} / a);
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

struct arccotangent : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"acot"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x acot\n"
            "\n"
            "    Returns the arccotangent of the bottom item on the stack:"
            "              acot(x) = atan(1/x)      (x > 0)\n"
            "              acot(x) = atan(1/x) + pi (x < 0)\n"
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
                               return atan_fn(mpf{1} / a);
                           }
                           return atan_fn(mpf{1} / a) + pi;
                       }
                       else
                       {
                           return atan_fn(mpf{1} / a);
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

register_calc_fn(sine);
register_calc_fn(cosine);
register_calc_fn(tangent);
register_calc_fn(arcsine);
register_calc_fn(arccosine);
register_calc_fn(arctangent);
register_calc_fn(arctangent2);

register_calc_fn(secant);
register_calc_fn(cosecant);
register_calc_fn(cotangent);
register_calc_fn(arcsecant);
register_calc_fn(arccosecant);
register_calc_fn(arccotangent);
