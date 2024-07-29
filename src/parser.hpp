/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <climits>
#include <debug.hpp>
#include <optional>
#include <string_view>

namespace smrty
{

struct program;
struct CalcFunction;

namespace parser
{

using diagnostic_function = std::function<void(std::string_view)>;

void set_current_base(int b);

void set_function_lists(
    std::vector<std::string_view>&,
    const std::vector<
        std::tuple<std::shared_ptr<const CalcFunction>, std::string_view>>&);

std::optional<program> parse_user_input(
    std::string_view str,
    diagnostic_function errors_callback = diagnostic_function());

} // namespace parser

} // namespace smrty
