/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <deque>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <optional>
#include <stack_entry.hpp>
#include <string>
#include <tuple>

class Calculator;

struct CalcFunction
{
    virtual ~CalcFunction() = default;
    virtual const std::string& name() const = 0;
    virtual const std::string& help() const = 0;
    virtual bool op(Calculator&) const = 0;
};

// All functions will register by adding an object to the __functions__ section
#define register_calc_fn(__cls)                                                \
    const static function::__cls __calc_fn_impl__##__cls;                      \
    const static CalcFunction* __calc_fn_##__cls                               \
        __attribute((__section__("calc_functions"))) __attribute((__used__)) = \
            &__calc_fn_impl__##__cls;

class Calculator
{
  public:
    enum class e_angle_mode
    {
        rad,
        deg,
        grad
    };
    enum class e_mpq_mode
    {
        f,
        q
    };
    struct Config
    {
        bool interactive = true;
        bool debug = false;
        int base = 10;
        int fixed_bits = 0;
        bool is_signed = true;
        int precision = 8;
        e_angle_mode angle_mode = e_angle_mode::rad;
        e_mpq_mode mpq_mode = e_mpq_mode::f;
    };
    using Stack = std::deque<stack_entry>;

    Config config;
    Stack stack;

    Calculator();
    bool run();

    // used by functions
    bool undo();
    bool debug();
    bool base();
    bool cbase();
    bool fixed_bits();
    bool precision();
    bool unsigned_mode();
    bool angle_mode(e_angle_mode);
    bool mpq_mode(e_mpq_mode);

  protected:
    std::deque<Stack> saved_stacks;

    void make_grammar();
    void make_functions();

    void show_stack();
    std::optional<std::string> get_input();
    std::string get_next_token();
    bool run_one(std::string expr);

    bool _running = true;

    std::map<std::string, const CalcFunction*> _operations;
    std::vector<std::string> _op_names;
    size_t _op_names_max_strlen;
};
