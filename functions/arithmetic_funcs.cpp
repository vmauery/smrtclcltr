/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>
#include <functions/common.hpp>

namespace function
{

// struct add : public CalcFunction
// {
const std::string& add::name() const
{
    static const std::string _name{"+"};
    return _name;
}
const std::string& add::help() const
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
bool add::op(Calculator& calc) const
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a + b; });
}
// }; // struct add

struct subtract : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"-"};
        return _name;
    }
    virtual const std::string& help() const final
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
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_op(calc,
                          [](const auto& a, const auto& b) { return a - b; });
    }
};

// struct multiply : public CalcFunction
// {
const std::string& multiply::name() const
{
    static const std::string _name{"*"};
    return _name;
}
const std::string& multiply::help() const
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
bool multiply::op(Calculator& calc) const
{
    return two_arg_op(calc, [](const auto& a, const auto& b) { return a * b; });
}
// };

struct divide : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"/"};
        return _name;
    }
    virtual const std::string& help() const final
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
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv_op(
            calc, [](const auto& a, const auto& b) { return a / b; },
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
        return two_arg_limited_op<mpz>(calc, [](const auto& a, const auto& b) {
            return a << static_cast<unsigned long>(b);
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
        return two_arg_limited_op<mpz>(calc, [](const auto& a, const auto& b) {
            return a >> static_cast<unsigned long>(b);
        });
    }
};

#ifdef USE_BASIC_TYPES
#define ceil_fn ceill
#else
#define ceil_fn boost::multiprecision::ceil
#endif

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
            [](const auto& a) -> numeric {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with ceil
                    mpf rp = ceil_fn(a.real());
                    mpf ip = ceil_fn(a.imag());
                    return mpc(rp, ip);
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return a;
                }
                else
                {
                    return ceil_fn(a);
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
    }
};

#ifdef USE_BASIC_TYPES
#define floor_fn floorl
#else
#define floor_fn boost::multiprecision::floor
#endif

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
            [](const auto& a) -> numeric {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with floor
                    mpf rp = floor_fn(a.real());
                    mpf ip = floor_fn(a.imag());
                    return mpc(rp, ip);
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return a;
                }
                else
                {
                    return floor_fn(a);
                }
            },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
    }
};

#ifdef USE_BASIC_TYPES
#define round_fn roundl
#else
#define round_fn boost::multiprecision::round
#endif

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
            [](const auto& a) -> numeric {
                if constexpr (std::is_same<decltype(a), const mpc&>::value)
                {
                    // complex adapter doesn't work with round
                    mpf rp = round_fn(a.real());
                    mpf ip = round_fn(a.imag());
                    return mpc(rp, ip);
                }
                else if constexpr (std::is_same<decltype(a), const mpz&>::value)
                {
                    // integers are already there
                    return a;
                }
                else
                {
                    return round_fn(a);
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
                          [](const auto& a) { return decltype(a){} - a; });
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
        return one_arg_limited_op<mpz, mpf, mpq, mpc>(
            calc, [](const auto& a) { return decltype(a)(1) / a; });
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
            calc, [](const auto& a, const auto& b) { return a % b; });
    }
};

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
            calc, [](const auto& a, const auto& b) { return pow(a, b); },
            std::tuple<mpq>{}, std::tuple<mpf>{}, std::tuple<mpz, mpf, mpc>{});
        return true;
    }
};

} // namespace function

register_calc_fn(add);
register_calc_fn(subtract);
register_calc_fn(multiply);
register_calc_fn(divide);
register_calc_fn(lshift);
register_calc_fn(rshift);
register_calc_fn(floor);
register_calc_fn(ceil);
register_calc_fn(round);
register_calc_fn(negate);
register_calc_fn(inverse);
register_calc_fn(divmod);
register_calc_fn(power);
