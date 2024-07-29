/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <function_library.hpp>
#include <program.hpp>

namespace smrty
{

// UserFunction is a special subclass wrapping all user-defined functions
struct UserFunction : public CalcFunction
{
    UserFunction(const std::string& name, program&& prog);
    const std::string& name() const final;
    const std::string& help() const final;
    bool op(Calculator& calc) const final;
    int num_args() const final;
    int num_resp() const final;
    symbolic_op symbolic_usage() const final;

    static CalcFunction::ptr create(const std::string& name, program&& prog);

    std::string _help;
    std::string _name;
    program function;
};

} // namespace smrty
