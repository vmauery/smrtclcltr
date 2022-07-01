/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return sin(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return cos(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return tan(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return asin(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return acos(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return atan(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    }
};

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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return sinh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return cosh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op(calc, a,
                                      [](const auto& a) { return tanh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return asinh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return acosh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [&calc](const auto& a) -> numeric {
                return scaled_trig_op_inv(
                    calc, a, [](const auto& a) { return atanh(a); });
            },
            std::tuple<mpz, mpq>{}, std::tuple<mpf, mpf>{},
            std::tuple<mpf, mpc>{});
    }
};

} // namespace function

register_calc_fn(sine);
register_calc_fn(cosine);
register_calc_fn(tangent);
register_calc_fn(arcsine);
register_calc_fn(arccosine);
register_calc_fn(arctangent);

register_calc_fn(hyperbolic_sine);
register_calc_fn(hyperbolic_cosine);
register_calc_fn(hyperbolic_tangent);
register_calc_fn(hyperbolic_arcsine);
register_calc_fn(hyperbolic_arccosine);
register_calc_fn(hyperbolic_arctangent);
