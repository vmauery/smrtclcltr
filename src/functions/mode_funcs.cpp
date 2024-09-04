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
struct Exit : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"exit"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: exit\n"
            "    Alias: quit\n"
            "\n"
            "    Stops execution of smrtclcltr and exits\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        calc.stop();
        return false;
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "quit";
        return _regex;
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        calc.stop();
        return false;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

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
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
            "    Set verbosity to level n (0-9)\n"
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
            std::print("request to set verbosity to {} ({})\n",
                       static_cast<int>(lvl), *v);
            if (lvl >= lg::level::emergency && lvl <= lg::level::trace)
            {
                lg::debug_level = lvl;
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument(
            std::format("Invalid verbosity: must be {}..{}",
                        static_cast<int>(lg::level::emergency),
                        static_cast<int>(lg::level::trace)));
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
            calc.base(iv);
            return false;
        }
        throw std::invalid_argument("requires an integer (2, 8, 10, 16)");
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        return symbolic_op::none;
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
        if (v && *v > mpz{0})
        {
            calc.stack.pop_front();
            auto iv = static_cast<unsigned int>(*v);
            calc.fixed_bits(iv);
            return false;
        }
        throw std::invalid_argument("requires a positive integer");
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        if (v && (*v > mpz{0}))
        {
            calc.stack.pop_front();
            auto iv = static_cast<unsigned int>(*v);
            calc.precision(iv);
            return false;
        }
        throw std::invalid_argument("requires a positive integer");
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.mpq_mode(Calculator::e_mpq_mode::quotient);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
            "    Usage: f\n"
            "\n"
            "    Print quotients as floats instead of quotients\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        calc.mpq_mode(Calculator::e_mpq_mode::floating);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.mpc_mode(Calculator::e_mpc_mode::ij);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.mpc_mode(Calculator::e_mpc_mode::rectangular);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.mpc_mode(Calculator::e_mpc_mode::polar);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.signed_mode(true);
        return false;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.signed_mode(false);
        return false;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        // two args are provided by num_args
        stack_entry y = calc.stack.front();
        calc.stack.pop_front();
        stack_entry x = calc.stack.front();
        calc.stack.pop_front();
        const mpz* su = std::get_if<mpz>(&x.value());
        const mpz* bits = std::get_if<mpz>(&y.value());
        if (su && bits && ((*su == 0) || (*su == 1)) && (*bits >= 0))
        {
            bool mode = (*su == 1);
            auto ibits = static_cast<unsigned int>(*bits);
            calc.signed_mode(mode);
            calc.fixed_bits(ibits);
            return false;
        }
        throw std::invalid_argument(
            "requires two integers: [0|1] and non-negative");
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        bool mode = match[1] == "s";
        unsigned int bits{};
        std::string_view x = match[2];
        if (std::from_chars(x.data(), x.data() + x.size(), bits).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        calc.signed_mode(mode);
        calc.fixed_bits(bits);
        return false;
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "([us])(0|[1-9][0-9]*)";
        return _regex;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.angle_mode(Calculator::e_angle_mode::radians);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.angle_mode(Calculator::e_angle_mode::degrees);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
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
        calc.angle_mode(Calculator::e_angle_mode::gradians);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct Help : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"help"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: help [cmd]\n"
            "\n"
            "    prints a list of available commands with no argument\n"
            "    or more information for `cmd` specified\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return calc.run_help();
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string fn = match[1];
        return calc.run_help(fn);
    }
    virtual const std::string_view regex() const final
    {
        static constexpr auto _regex{"help\\s+([^\\s]+)"};
        return _regex;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

} // namespace function
} // namespace smrty

register_calc_fn(Exit);
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
register_calc_fn(Help);
