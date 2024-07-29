/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>
#include <function_library.hpp>
#include <user_function.hpp>

namespace smrty
{
namespace function
{

struct store : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sto"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x 'y' sto\n"
            "\n"
            "    Store the second to bottom item on the stack (x)\n"
            "    in a variable named y\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        if (calc.stack.size() < 2)
        {
            throw std::invalid_argument("Requires 2 arguments");
        }
        stack_entry& a = calc.stack[1];
        stack_entry& b = calc.stack[0];

        auto& val = a.value();
        auto var = std::get_if<symbolic>(&b.value());
        if (var)
        {
            if (auto n = std::get_if<std::string>(&(*(*var)).left); n)
            {
                // make sure there is not a user-defined function with this name
                if (is_user_function(*n))
                {
                    throw std::invalid_argument(
                        "User-defined function already exists with this name");
                }
                calc.variables[*n] = val;
                calc.stack.pop_front();
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument("'y' must be a string for variable name");
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

struct define : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"def"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: $(.x.) 'y' def\n"
            "\n"
            "    Define a new user-defined function named y with contents x\n"
            "\n"
            "    After defining the function, it can be run just as any\n"
            "    other built-in function, i.e. without quotes\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // first two args provided by num_args
        stack_entry& a = calc.stack[1];
        stack_entry& b = calc.stack[0];

        auto prog = std::get_if<program>(&a.value());
        if (!prog)
        {
            throw std::invalid_argument("'x' must be a program");
        }
        auto var = std::get_if<symbolic>(&b.value());
        if (var)
        {
            if (auto n = std::get_if<std::string>(&(*(*var)).left); n)
            {
                if (is_user_function(*n))
                {
                    throw std::invalid_argument(
                        "User-defined function already exists with this name");
                }
                register_user_function(*n, program{*prog});
                calc.stack.pop_front();
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument("'y' must be a string for function name");
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

struct rm : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rm"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: 'x' rm\n"
            "\n"
            "    Delete user-defined function or variable named x\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // arg provided by num_args
        stack_entry& a = calc.stack[0];

        auto var = std::get_if<symbolic>(&a.value());
        if (var)
        {
            if (auto n = std::get_if<std::string>(&(*(*var)).left); n)
            {
                // check if n is a variable
                if (auto v = calc.variables.find(*n); v != calc.variables.end())
                {
                    calc.variables.erase(v, ++v);
                }
                else if (is_user_function(*n))
                {
                    unregister_user_function(*n);
                }
                else
                {
                    throw std::invalid_argument(std::format(
                        "Failed to find variable or function named '{}'", *n));
                }
                calc.stack.pop_front();
                return true;
            }
        }
        throw std::invalid_argument(
            "'x' must be a string for variable or user-defined function name");
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

struct listing : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"ls"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: ls\n"
            "           ls -l\n"
            "\n"
            "    List the variables and user-defined functions available,\n"
            "    optionally with their values as well (-l)\n"
            // clang-format on
        };
        return _help;
    }
    bool list(Calculator& calc) const
    {
        auto ui = ui::get();
        auto size = ui->size();
        auto& tcols = std::get<1>(size);

        auto display = [&ui, &tcols](std::span<std::string_view> var_names) {
            // limit listing to screen width
            column_layout l = find_best_layout(var_names, tcols);
            size_t col_count = l.cols.size();
            size_t row_count =
                std::ceil(static_cast<double>(var_names.size()) / col_count);
            for (size_t i = 0; i < row_count; i++)
            {
                for (size_t j = 0; j < col_count; j++)
                {
                    size_t idx = row_count * j + i;
                    if (idx >= var_names.size())
                    {
                        break;
                    }
                    ui->out("{0: <{1}}", var_names[idx], l.cols[j]);
                }
                ui->out("\n");
            }
        };

        if (calc.variables.size() != 0)
        {
            std::vector<std::string_view> var_names;
            var_names.reserve(calc.variables.size());
            for (const auto& [var, val] : calc.variables)
            {
                var_names.push_back(var);
            }
            ui->out("Variables:\n");
            display(var_names);
            ui->out("\n");
        }

        auto user_fns = fn_get_all_user();
        if (user_fns.size() != 0)
        {
            std::vector<std::string_view> fn_names;
            fn_names.reserve(user_fns.size());
            for (const auto& ptr : user_fns)
            {
                fn_names.push_back(ptr->name());
            }
            ui->out("User-defined functions:\n");
            display(fn_names);
            ui->out("\n");
        }
        return false;
    }
    bool show(Calculator& calc) const
    {
        // limit listing to screen width
        auto ui = ui::get();
        auto size = ui->size();
        auto& tcols = std::get<1>(size);

        if (calc.variables.size() != 0)
        {
            ui->out("Variables:\n");
            size_t max_var_len = 0;
            for (const auto& [var, val] : calc.variables)
            {
                max_var_len = std::max(max_var_len, var.size());
            }

            for (const auto& [var, val] : calc.variables)
            {
                std::string vo = std::format("{}", val);
                vo.resize(tcols - max_var_len - 2);
                ui->out("{0: <{1}}: {2}\n", var, max_var_len, vo);
            }
            ui->out("\n");
        }

        auto user_fns = fn_get_all_user();
        if (user_fns.size() != 0)
        {
            ui->out("User-defined functions:\n");
            size_t max_var_len = 0;
            for (const auto& ptr : user_fns)
            {
                max_var_len = std::max(max_var_len, ptr->name().size());
            }

            for (const auto& ptr : user_fns)
            {
                auto fn = dynamic_pointer_cast<const UserFunction>(ptr);
                std::string vo = std::format("{}", fn->function);
                vo.resize(tcols - max_var_len - 2);
                ui->out("{0: <{1}}: {2}\n", ptr->name(), max_var_len, vo);
            }
            ui->out("\n");
        }
        return false;
    }
    virtual bool op(Calculator& calc) const final
    {
        return list(calc);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string fn = match[1];
        if (fn == "-l")
        {
            return show(calc);
        }
        throw std::invalid_argument("invalid argument to ls");
    }
    virtual const std::string_view regex() const final
    {
        static constexpr auto _regex{"ls\\s+(-l)"};
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

register_calc_fn(store);
register_calc_fn(define);
register_calc_fn(rm);
register_calc_fn(listing);
