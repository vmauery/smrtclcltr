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
    return two_arg_uconv_op(
        calc,
        [](const auto& a, const auto& b, const units::unit& ua,
           const units::unit& ub) -> std::tuple<numeric, units::unit> {
            if (ua != ub)
            {
                throw std::invalid_argument("units do not match");
            }
            return {a + b, ua};
        });
}

bool divide_from_stack(Calculator& calc)
{
    return two_arg_conv_op(
        calc,
        [](const auto& a, const auto& b, const units::unit& ua,
           const units::unit& ub) -> std::tuple<numeric, units::unit> {
            lg::debug("a: ({} (type {}))\n", a, DEBUG_TYPE(a));
            lg::debug("b: ({} (type {}))\n", b, DEBUG_TYPE(b));
            auto r = a / b;
            lg::debug("r: ({} (type {}))\n", r, DEBUG_TYPE(r));
            return {r, ua / ub};
        },
        std::tuple<mpz>{}, std::tuple<mpq>{},
        std::tuple<mpq, mpf, mpc, time_>{});
}

} // namespace util

struct add : public CalcFunction
{
    const std::string& name() const
    {
        static const std::string _name{"+"};
        return _name;
    }
    const std::string& help() const
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y +\n"
            "\n"
            "    Returns the sum of the bottom two items on the stack (x + y)\n"
            // clang-format on
        };
        return _help;
    }
    bool op(Calculator& calc) const
    {
        return util::add_from_stack(calc);
    }
}; // struct add

struct subtract : public CalcFunction
{
    const std::string& name() const
    {
        static const std::string _name{"-"};
        return _name;
    }
    const std::string& help() const
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
    bool op(Calculator& calc) const
    {
        return two_arg_uconv_op(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                return {a - b, ua};
            });
    }
};

struct multiply : public CalcFunction
{
    const std::string& name() const
    {
        static const std::string _name{"*"};
        return _name;
    }
    const std::string& help() const
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
    bool op(Calculator& calc) const
    {
        return two_arg_op(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                return {a * b, ua * ub};
            });
    }
};

struct divide : public CalcFunction
{
    const std::string& name() const
    {
        static const std::string _name{"/"};
        return _name;
    }
    const std::string& help() const
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
    bool op(Calculator& calc) const
    {
        return util::divide_from_stack(calc);
    }
};

struct sum : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sum"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ... x sum\n"
            "\n"
            "    Returns the sum of the "
            "bottom x items on the stack: Nx * Nx-1 * ... * N0\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (!v || (*v > 1000000000) ||
            (*v >= static_cast<mpz>(calc.stack.size())))
        {
            return false;
        }
        calc.stack.pop_front();
        size_t count = static_cast<size_t>(*v - 1);
        add add_fn{};
        for (; count > 0; count--)
        {
            if (!add_fn.op(calc))
            {
                return false;
            }
        }
        return true;
    }
};

struct product : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"product"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ... x product\n"
            "\n"
            "    Returns the product of the "
            "bottom x items on the stack: Nx * Nx-1 * ... * N0\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (!v || (*v > 1000000000) ||
            (*v >= static_cast<mpz>(calc.stack.size())))
        {
            return false;
        }
        calc.stack.pop_front();
        size_t count = static_cast<size_t>(*v - 1);
        multiply mult{};
        for (; count > 0; count--)
        {
            if (!mult.op(calc))
            {
                return false;
            }
        }
        return true;
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
        return two_arg_conv_op(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                return {(b - a) / a, units::unit{}};
            },
            std::tuple<mpz>{}, std::tuple<mpq>{},
            std::tuple<mpq, mpf, mpc, time_>{});
    }
};

struct lshift : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"<<"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y <<\n"
            "\n"
            "    Returns the next-to-bottom item left-shifted by the "
            "bottom item\n"
            "    on the stack (x << y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                return {a << static_cast<unsigned long long>(b), ua};
            });
    }
};

struct rshift : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{">>"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y >>\n"
            "\n"
            "    Returns the next-to-bottom item right-shifted by the "
            "bottom item\n"
            "    on the stack (x >> y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_limited_op<mpz>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                return {a >> static_cast<unsigned long long>(b), ua};
            });
    }
};

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
        return one_arg_conv_op(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with ceil
                    mpf rp = ceil_fn(a.real());
                    mpf ip = ceil_fn(a.imag());
                    return {mpc(rp, ip), ua};
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return {a, ua};
                }
                else
                {
                    return {ceil_fn(a), ua};
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with floor
                    mpf rp = floor_fn(a.real());
                    mpf ip = floor_fn(a.imag());
                    return {mpc(rp, ip), ua};
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return {a, ua};
                }
                else
                {
                    return {floor_fn(a), ua};
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
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
        return one_arg_conv_op(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with round
                    mpf rp = round_fn(a.real());
                    mpf ip = round_fn(a.imag());
                    return {mpc(rp, ip), ua};
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return {a, ua};
                }
                else
                {
                    return {round_fn(a), ua};
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
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
                              return {std::decay_t<decltype(a)>{} - a, ua};
                          });
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
        return one_arg_conv_op(
            calc,
            [](const auto& a,
               const units::unit& ua) -> std::tuple<numeric, units::unit> {
                return {mpq(1, 1) / a, units::unit() / ua};
            },
            std::tuple<mpz>{}, std::tuple<mpq>{}, std::tuple<mpq, mpf, mpc>{});
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
        return two_arg_limited_op<mpz>(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != ub)
                {
                    throw std::invalid_argument("units do not match");
                }
                return {a % b, ua};
            });
    }
};

namespace util
{

numeric pow(const mpz& base, const mpz& exponent)
{
    mpz b(base), e(exponent);
    bool invert = false;
    if (e < 0)
    {
        invert = true;
        e = -e;
    }
    mpz result = 1;
    while (e > 0)
    {
        if (e & 1)
        {
            result *= b;
        }
        e = e >> 1;
        b *= b;
    }
    if (invert)
    {
        return mpq(1, result);
    }
    return result;
}

} // namespace util

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
        return two_arg_conv_op(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ub != units::unit())
                {
                    throw std::invalid_argument("cannot raise to a unit power");
                }
                if constexpr (std::is_same_v<decltype(a), const mpz&> &&
                              std::is_same_v<decltype(b), const mpz&>)
                {
                    return {util::pow(a, b), units::pow(ua, to_mpf(b))};
                }
                else if constexpr (std::is_same_v<decltype(a), decltype(b)>)
                {
                    return {pow_fn(a, b), units::pow(ua, to_mpf(b))};
                }
                else if constexpr (std::is_same_v<decltype(a), const mpc&> ||
                                   std::is_same_v<decltype(b), const mpc&>)
                {
                    return {pow_fn(to_mpc(a), to_mpc(b)),
                            units::pow(ua, to_mpf(b))};
                }
                else
                {
                    return {pow_fn(to_mpf(a), to_mpf(b)),
                            units::pow(ua, to_mpf(b))};
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(add);
register_calc_fn(subtract);
register_calc_fn(multiply);
register_calc_fn(divide);
register_calc_fn(sum);
register_calc_fn(product);
register_calc_fn(percent_change);
register_calc_fn(lshift);
register_calc_fn(rshift);
register_calc_fn(floor);
register_calc_fn(ceil);
register_calc_fn(round);
register_calc_fn(negate);
register_calc_fn(inverse);
register_calc_fn(divmod);
register_calc_fn(power);
