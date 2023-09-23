/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>
#include <functions/common.hpp>
#include <version.hpp>

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
        return calc.mpq_mode(Calculator::e_mpq_mode::q);
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
        return calc.mpq_mode(Calculator::e_mpq_mode::f);
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
        return calc.angle_mode(Calculator::e_angle_mode::rad);
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
        return calc.angle_mode(Calculator::e_angle_mode::deg);
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
        return calc.angle_mode(Calculator::e_angle_mode::grad);
    }
};

} // namespace function
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
register_calc_fn(radians);
register_calc_fn(degrees);
register_calc_fn(gradiens);
