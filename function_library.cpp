/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <algorithm>
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
std::map<std::string_view, const CalcFunction*> operations;
std::vector<std::string_view> function_names;
std::map<size_t, std::string_view> reops;

} // namespace

size_t fn_id_by_name(std::string_view name)
{
    const auto& fn =
        std::find(function_names.begin(), function_names.end(), name);
    if (fn == function_names.end())
    {
        throw std::invalid_argument(
            std::format("Function '{}' does not exist", name));
    }
    return std::distance(function_names.begin(), fn);
}

std::string_view fn_name_by_id(size_t id)
{
    if (id < function_names.size())
    {
        return function_names[id];
    }
    static constexpr auto unknown_function = "<unknown-function>";
    return unknown_function;
}

const CalcFunction* fn_get_fn_ptr_by_name(std::string_view name)
{
    auto fn = operations.find(name);
    if (fn != operations.end())
    {
        return fn->second;
    }
    return nullptr;
}

std::span<std::string_view> fn_get_all_names()
{
    return {function_names.begin(), function_names.end()};
}

void setup_catalog()
{
    // add the functions in the __functions__ section
    for (auto iter = &__start_calc_functions; iter < &__stop_calc_functions;
         iter++)
    {
        operations[(*iter)->name()] = *iter;
    }
    op_names_max_strlen = 1;
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

    std::vector<std::tuple<size_t, std::string_view>> reop_list{};
    for (const auto& [k, v] : operations)
    {
        auto re = v->regex();
        if (re.size())
        {
            auto first = function_names.begin();
            auto iter = std::find(first, function_names.end(), k);
            reops.emplace(std::distance(first, iter), re);
            reop_list.emplace_back(
                std::make_tuple(std::distance(first, iter), re));
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
