/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>

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
            "    List the variables stored in memory\n"
            "    optionally with their values as well (-l)\n"
            // clang-format on
        };
        return _help;
    }
    bool list_vars(Calculator& calc) const
    {
        if (calc.variables.size() == 0)
        {
            return false;
        }
        // limit listing to screen width
        std::vector<std::string_view> var_names;
        var_names.reserve(calc.variables.size());
        for (const auto& [var, val] : calc.variables)
        {
            var_names.push_back(var);
        }
        auto ui = ui::get();
        auto size = ui->size();
        auto& tcols = std::get<1>(size);
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
        return false;
    }
    bool show_vars(Calculator& calc) const
    {
        if (calc.variables.size() == 0)
        {
            return false;
        }
        // limit listing to screen width
        auto ui = ui::get();
        auto size = ui->size();
        auto& tcols = std::get<1>(size);
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
        return false;
    }
    virtual bool op(Calculator& calc) const final
    {
        return list_vars(calc);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string fn = match[1];
        if (fn == "-l")
        {
            return show_vars(calc);
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
register_calc_fn(listing);
