/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <config.hpp>
#include <deque>
#include <functional>
#include <input.hpp>
#include <map>
#include <numeric.hpp>
#include <optional>
#include <regex>
#include <stack_entry.hpp>
#include <string>
#include <tuple>

namespace smrty
{

class Calculator
{
  public:
    enum class e_angle_mode
    {
        radians,
        degrees,
        gradians
    };
    enum class e_mpq_mode
    {
        floating,
        quotient
    };
    enum class e_mpc_mode
    {
        rectangular,
        polar,
        ij
    };
    struct Settings
    {
        static constexpr int default_base = 10;
        static constexpr int default_fixed_bits = 0;
        static constexpr bool default_is_signed = true;

        bool interactive = true;
        bool debug = false;
        int base = default_base;
        int fixed_bits = default_fixed_bits;
        bool is_signed = default_is_signed;
        int precision = builtin_default_precision;
        e_angle_mode angle_mode = e_angle_mode::radians;
        e_mpq_mode mpq_mode = e_mpq_mode::floating;
        e_mpc_mode mpc_mode = e_mpc_mode::rectangular;
        bool save_stack = false;
        bool local_time = true;
    };
    using Stack = std::deque<stack_entry>;

    Settings config;
    Stack stack;
    execution_flags flags;

    // enforce singleton
  protected:
    struct no_touchy
    {
    };

  public:
    Calculator(const no_touchy&) : Calculator()
    {
    }
    ~Calculator();
    void save_state(const std::filesystem::path& filename);
    // to require access via get(); no copies
    Calculator(const Calculator&) = delete;
    Calculator(Calculator&&) = delete;
    Calculator& operator=(const Calculator&) = delete;
    Calculator& operator=(Calculator&&) = delete;
    static Calculator& get()
    {
        static std::unique_ptr<Calculator> _this{};
        if (!_this)
        {
            _this = std::make_unique<Calculator>(no_touchy{});
        }
        return *_this;
    }
    bool run(std::string_view);
    bool run_help(std::string_view fn = {});
    void stop()
    {
        _running = false;
    }

    // used by functions
    bool undo();
    bool debug();
    bool base(unsigned int);
    bool cbase();
    bool fixed_bits(unsigned int);
    bool precision(unsigned int);
    bool signed_mode(bool);
    bool tz_mode();
    bool angle_mode(e_angle_mode);
    bool mpq_mode(e_mpq_mode);
    bool mpc_mode(e_mpc_mode);
    bool run_one(const simple_instruction& itm);

    // methods to access scoped variables
    void var_scope_enter();
    void var_scope_exit();
    std::vector<std::string_view> get_var_names();
    std::optional<numeric> get_var(std::string_view name);
    void set_var(std::string_view name, const numeric& value);
    void unset_var(std::string_view name);

  protected:
    Calculator();

    std::deque<Stack> saved_stacks;

    // a stack of variables to allow for scope
    std::deque<std::map<std::string, numeric>> variables;

    void make_functions();
    std::optional<std::string_view> auto_complete(std::string_view in,
                                                  int state);

    std::string format_stack_entry(const stack_entry& e, size_t first_col);
    void show_stack();

    bool _running = true;

    std::shared_ptr<Input> input;
};

} // namespace smrty
