/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <deque>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <string>

struct stack_entry
{
    stack_entry() : base(10), fixed_bits(0), precision(8), is_signed(true)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s) :
        value(v), base(b), fixed_bits(f), precision(p), is_signed(s)
    {
    }
    numeric value;
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
};

class calculator
{
  public:
    calculator();
    bool run();

  protected:
    void show_stack();
    std::string get_next_token();
    bool run_one(const std::string& expr);
    template <typename Fn>
    bool two_arg_op(const Fn& fn)
    {
        if (_stack.size() < 2)
        {
            return false;
        }
        stack_entry a = _stack.front();
        _stack.pop_front();
        stack_entry b = _stack.front();
        _stack.pop_front();
        numeric cv = operate(fn, a.value, b.value);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    bool two_arg_op(const std::function<numeric(const auto&, const auto&)>& fn);
    bool debug();
    bool base();
    bool fixed_bits();
    bool precision();
    bool unsigned_mode();
    bool add();
    bool subtract();
    bool multiply();
    bool divide();

    bool _debug = false;
    bool _running = true;
    int _base = 10;
    int _fixed_bits = 0;
    bool _is_signed = true;
    int _precision = 8;
    std::deque<stack_entry> _stack;
    std::map<std::string, std::function<bool()>> _operations;
};
