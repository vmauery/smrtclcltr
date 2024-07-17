/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct bitwise_and : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"&"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y &\n"
            "\n"
            "    Returns the bitwise AND of the bottom two items on "
            "the stack (x & y)\n"
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
                    throw units_mismatch();
                }
                return {a & b, ua};
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

struct bitwise_or : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"|"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y |\n"
            "\n"
            "    Returns the bitwise OR of the bottom "
            "two items on the stack (x & y)\n"
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
                    throw units_mismatch();
                }
                return {a | b, ua};
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

struct bitwise_xor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"xor"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y xor\n"
            "\n"
            "    Returns the bitwise XOR of the bottom two items on "
            "the stack (x xor y)\n"
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
                    throw units_mismatch();
                }
                return {a ^ b, ua};
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

struct bitwise_inv : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"~"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x ~\n"
            "\n"
            "    Returns the bitwise negation of the bottom "
            "item on the stack (~x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return one_arg_limited_op<mpz>(
            calc,
            [](const auto& a, const units::unit& ua)
                -> std::tuple<numeric, units::unit> { return {~a, ua}; });
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
                    throw units_mismatch();
                }
                return {a << static_cast<unsigned long long>(b), ua};
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
                    throw units_mismatch();
                }
                return {a >> static_cast<unsigned long long>(b), ua};
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

} // namespace function
} // namespace smrty

register_calc_fn(bitwise_and);
register_calc_fn(bitwise_or);
register_calc_fn(bitwise_xor);
register_calc_fn(bitwise_inv);
register_calc_fn(lshift);
register_calc_fn(rshift);
