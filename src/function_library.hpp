/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace smrty
{

class Calculator;
enum class symbolic_op;

struct CalcFunction
{
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
};

// All functions will register by adding an object to the __functions__ section
#define register_calc_fn(__cls)                                                \
    const static smrty::function::__cls __calc_fn_impl__##__cls;               \
    const static smrty::CalcFunction* __calc_fn_##__cls                        \
        __attribute((__section__("calc_functions"))) __attribute((__used__)) = \
            &__calc_fn_impl__##__cls;

size_t fn_id_by_name(std::string_view name);

std::string_view fn_name_by_id(size_t id);

const CalcFunction* fn_get_fn_ptr_by_name(std::string_view name);
const CalcFunction* fn_get_fn_ptr_by_id(size_t id);

std::span<std::string_view> fn_get_all_names();

void setup_catalog();

std::span<std::string_view> fn_list_all_starts_with(std::string_view start);

} // namespace smrty
