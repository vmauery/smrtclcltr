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
#include <program.hpp>
#include <user_function.hpp>

extern const struct smrty::CalcFunction* __start_calc_functions;
extern const struct smrty::CalcFunction* __stop_calc_functions;

namespace smrty
{

namespace
{

size_t op_names_max_strlen;
std::map<std::string_view, CalcFunction::ptr> operations;
std::vector<std::string_view> function_names;
std::vector<std::string_view> auto_complete_words;
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
    // add the user-defined functions
    for (const auto& fn : user_functions)
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

    auto_complete_words = function_names;
    auto_complete_words.insert(auto_complete_words.end(),
                               {"break", "continue", "do", "done", "elif",
                                "else", "endif", "for", "if", "in", "then",
                                "while"});
    std::sort(auto_complete_words.begin(), auto_complete_words.end());

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

void register_user_function_early(const std::string& name, program&& function)
{
    user_functions.push_back(UserFunction::create(name, std::move(function)));
}

void register_user_function(const std::string& name, program&& function)
{
    register_user_function_early(name, std::move(function));
    setup_catalog();
}

void unregister_user_function(const std::string& name)
{
    std::erase_if(user_functions,
                  [&name](CalcFunction::ptr& p) { return p->name() == name; });
    setup_catalog();
}

bool is_user_function(const std::string& name)
{
    auto f = std::find_if(
        user_functions.begin(), user_functions.end(),
        [&name](CalcFunction::ptr& p) { return p->name() == name; });
    return f != user_functions.end();
}

const std::vector<CalcFunction::ptr>& fn_get_all_user()
{
    return user_functions;
}

std::span<std::string_view> fn_list_all_starts_with(std::string_view start)
{
    std::vector<std::string_view>::iterator first = auto_complete_words.end();
    for (auto iter = auto_complete_words.begin();
         iter != auto_complete_words.end(); iter++)
    {
        if (iter->starts_with(start))
        {
            first = iter++;
            for (; iter != auto_complete_words.end(); iter++)
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
