/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <algorithm>
#include <calculator.hpp>
#include <exception>
#include <format>
#include <function_library.hpp>
#include <map>
#include <parser.hpp>

extern const struct smrty::CalcFunction* __start_calc_functions;
extern const struct smrty::CalcFunction* __stop_calc_functions;

namespace smrty
{

namespace
{

size_t op_names_max_strlen;
std::map<std::string_view, CalcFunction::ptr> operations;
std::vector<std::string_view> function_names;
std::map<CalcFunction::ptr, std::string_view> reops;
std::vector<CalcFunction::ptr> user_functions;

} // namespace


CalcFunction::ptr fn_get_fn_ptr_by_name(std::string_view name)
{
    auto fn = operations.find(name);
    if (fn != operations.end())
    {
        return fn->second;
    }
    return nullptr;
}

std::string_view fn_get_name(CalcFunction::ptr p)
{
    if (p)
    {
        return p->name();
    }
    return "<unknown-function>";
}

std::span<std::string_view> fn_get_all_names()
{
    return {function_names.begin(), function_names.end()};
}

struct DoNotDelete
{
    void operator()(auto*)
    {
    }
};

static std::vector<CalcFunction::ptr> builtin_functions;
void initialize_builtin_function_vector() __attribute__((constructor));
void initialize_builtin_function_vector()
{
    DoNotDelete undeleter;
    for (auto iter = &__start_calc_functions; iter < &__stop_calc_functions;
         iter++)
    {
        builtin_functions.emplace_back(
            std::shared_ptr<const CalcFunction>(*iter, undeleter));
    }
}

void setup_catalog()
{
    operations.clear();
    // add the functions in the __functions__ section
    for (const auto& fn : builtin_functions)
    {
        operations[fn->name()] = fn;
    }
    op_names_max_strlen = 1;
    function_names.clear();
    std::transform(operations.begin(), operations.end(),
                   std::back_inserter(function_names), [](const auto& kv) {
                       size_t sz = kv.first.size();
                       if (sz > op_names_max_strlen)
                       {
                           op_names_max_strlen = sz;
                       }
                       return kv.first;
                   });
    std::sort(function_names.begin(), function_names.end());

    std::vector<std::tuple<CalcFunction::ptr, std::string_view>> reop_list{};
    reops.clear();
    for (const auto& [k, v] : operations)
    {
        auto re = v->regex();
        if (re.size())
        {
            reops.emplace(v, re);
            reop_list.emplace_back(std::make_tuple(v, re));
        }
    }
    parser::set_function_lists(function_names, reop_list);
}

std::span<std::string_view> fn_list_all_starts_with(std::string_view start)
{
    std::vector<std::string_view>::iterator first = function_names.end();
    for (auto iter = function_names.begin(); iter != function_names.end();
         iter++)
    {
        if (iter->starts_with(start))
        {
            first = iter++;
            for (; iter != function_names.end(); iter++)
            {
                if (!(iter->starts_with(start)))
                {
                    break;
                }
            }
            return {first, iter};
        }
    }
    return {};
}

} // namespace smrty
