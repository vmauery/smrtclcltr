/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <format>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace smrty
{

class Calculator;
struct program;
enum class symbolic_op;

struct CalcFunction
{
    using ptr = std::shared_ptr<const CalcFunction>;
    virtual ~CalcFunction() = default;
    virtual const std::string& name() const = 0;
    virtual const std::string& help() const = 0;
    virtual bool op(Calculator&) const = 0;
    virtual bool reop(Calculator&, const std::vector<std::string>&) const
    {
        return false;
    };
    virtual const std::string_view regex() const
    {
        static std::string_view _regex{};
        return _regex;
    }
    // if number of args is known, each function should set that
    // if number of args is < 0, it is variable, with |n| as the min
    virtual int num_args() const = 0;
    virtual int num_resp() const = 0;
    virtual symbolic_op symbolic_usage() const = 0;

    static CalcFunction::ptr create(const std::string&, program&&);
};

// All functions will register by adding an object to the __functions__ section
#define register_calc_fn(__cls)                                                \
    const static smrty::function::__cls __calc_fn_impl__##__cls;               \
    const static smrty::CalcFunction* __calc_fn_##__cls                        \
        __attribute((__section__("calc_functions"))) __attribute((__used__)) = \
            &__calc_fn_impl__##__cls;

std::string_view fn_get_name(CalcFunction::ptr);

CalcFunction::ptr fn_get_fn_ptr_by_name(std::string_view name);

std::span<std::string_view> fn_get_all_names();

void setup_catalog();
void register_user_function(const std::string& name, program&& function);
void unregister_user_function(const std::string& name);
bool is_user_function(const std::string& name);
const std::vector<CalcFunction::ptr>& fn_get_all_user();

std::span<std::string_view> fn_list_all_starts_with(std::string_view start);

} // namespace smrty

template <>
struct std::formatter<smrty::CalcFunction::ptr>
{
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // TODO: at some point, a pretty vs oneline option might be nice
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const smrty::CalcFunction::ptr& b, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        out = std::format_to(out, "CalcFunction({})", fn_get_name(b));
        return out;
    }
};
