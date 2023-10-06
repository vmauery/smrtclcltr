/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <charconv>
#include <cmath>
#include <function.hpp>
#include <functions/common.hpp>
#include <version.hpp>

namespace smrty
{
// internal calculator functions
namespace function
{
struct version : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"version"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: version\n"
            "\n"
            "    Display version info\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator&) const final
    {
        ui::get()->out("Version: {}\n", Version::full());
        return true;
    }
};

struct debug : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"debug"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: debug\n"
            "\n"
            "    Toggle debug mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.debug();
    }
};

struct verbose : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"verbose"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: n verbose\n"
            "\n"
            "    Set verbosity to level n (0-7)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v)
        {
            auto lvl = static_cast<lg::level>(static_cast<int>(*v));
            if (lvl >= lg::level::emergency && lvl <= lg::level::debug)
            {
                lg::debug_level = lvl;
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument("Invalid verbosity: must be 0..7");
    }
};

struct undo : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"undo"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: undo\n"
            "\n"
            "    Undo last operation or command line\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.undo();
    }
};

struct base : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"base"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x base\n"
            "\n"
            "    Sets the numeric base to the bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v)
        {
            calc.stack.pop_front();
            auto iv = static_cast<unsigned int>(*v);
            return calc.base(iv);
        }
        return false;
    }
};

struct cbase : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"cbase"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: cbase\n"
            "\n"
            "    Changes the numeric base of the bottom item to be the current base\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.cbase();
    }
};

struct fixed_bits : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"fixed_bits"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x fixed_bits\n"
            "\n"
            "    Sets the number of fixed bits to the "
            "bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v)
        {
            calc.stack.pop_front();
            auto iv = static_cast<unsigned int>(*v);
            return calc.fixed_bits(iv);
        }
        return false;
    }
};

struct precision : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"precision"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x precision\n"
            "\n"
            "    Sets the precision to the bottom item on the stack (x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v)
        {
            calc.stack.pop_front();
            auto iv = static_cast<unsigned int>(*v);
            return calc.precision(iv);
        }
        return false;
    }
};

struct quotient : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"q"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: q\n"
            "\n"
            "    Print quotients as quotients instead of floats\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpq_mode(Calculator::e_mpq_mode::quotient);
    }
};

struct floats : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"f"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x f\n"
            "\n"
            "    Print quotients as floats instead of quotients\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpq_mode(Calculator::e_mpq_mode::floating);
    }
};

struct ij : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"ij"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ij\n"
            "\n"
            "    Print complex numbers in rectangular x+iy format\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpc_mode(Calculator::e_mpc_mode::ij);
    }
};

struct rectangular : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rectangular"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rectangular\n"
            "\n"
            "    Print complex numbers in rectangular (x,y) format\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpc_mode(Calculator::e_mpc_mode::rectangular);
    }
};

struct polar : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"polar"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: polar\n"
            "\n"
            "    Print complex numbers in polar (m,<a) format\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.mpc_mode(Calculator::e_mpc_mode::polar);
    }
};

struct signed_mode : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"signed"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: signed\n"
            "\n"
            "    Sets signed mode for integers\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.signed_mode(true);
    }
};

struct unsigned_mode : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"unsigned"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: unsigned\n"
            "\n"
            "    Sets unsigned mode for integers\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.signed_mode(false);
    }
};

struct int_type : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"int_type"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y int_type\n"
            "\n"
            "    Sets integer type for new integers where x is 0 or 1\n"
            "    and denotes unsigned (0) or signed (1) and\n"
            "    y denotes the number of bits\n"
            "\n"
            "    Alternate mechanism is of the form [su][0-9]+ where\n"
            "    the signed/unsigned and bits are put together, e.g. s32\n"
            "    for 32-bit signed, or u16 for 16-bit unsigned\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 2)
        {
            return false;
        }
        stack_entry y = calc.stack.front();
        calc.stack.pop_front();
        stack_entry x = calc.stack.front();
        calc.stack.pop_front();
        const mpz* su = std::get_if<mpz>(&x.value());
        const mpz* bits = std::get_if<mpz>(&y.value());
        if (su && bits && ((*su == 0) || (*su == 1)) && (*bits > 0))
        {
            bool mode = (*su == 1);
            auto ibits = static_cast<unsigned int>(*bits);
            return calc.signed_mode(mode) && calc.fixed_bits(ibits);
        }
        return false;
    }
    virtual bool reop(Calculator& calc, const std::cmatch& match) const final
    {
        bool mode = match[1].str() == "s";
        unsigned int bits{};
        const std::string x = match[2].str();
        if (std::from_chars(x.data(), x.data() + x.size(), bits).ec !=
            std::errc{})
        {
            return false;
        }
        return calc.signed_mode(mode) && calc.fixed_bits(bits);
    }
    virtual const std::optional<std::regex>& regex() const final
    {
        static const std::optional<std::regex> _regex{"([us])([0-9]+)"};
        return _regex;
    }
};

struct radians : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rad"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rad\n"
            "\n"
            "    Sets radians angle mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::radians);
    }
};

struct degrees : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"deg"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: deg\n"
            "\n"
            "    Sets degrees angle mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::degrees);
    }
};

struct gradiens : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"grad"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: grad\n"
            "\n"
            "    Sets gradiens degree mode\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.angle_mode(Calculator::e_angle_mode::gradians);
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(version);
register_calc_fn(debug);
register_calc_fn(verbose);
register_calc_fn(undo);
register_calc_fn(base);
register_calc_fn(cbase);
register_calc_fn(fixed_bits);
register_calc_fn(precision);
register_calc_fn(quotient);
register_calc_fn(floats);
register_calc_fn(signed_mode);
register_calc_fn(unsigned_mode);
register_calc_fn(int_type);
register_calc_fn(radians);
register_calc_fn(degrees);
register_calc_fn(gradiens);
register_calc_fn(ij);
register_calc_fn(polar);
register_calc_fn(rectangular);
