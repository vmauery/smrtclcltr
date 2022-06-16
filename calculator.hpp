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
#include <string>
#include <tuple>

class Calculator;

using CalcFunction = std::tuple<std::string, std::function<bool(Calculator&)>>;

struct stack_entry
{
    stack_entry() :
        _value(), base(10), fixed_bits(0), precision(8), is_signed(true)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s) :
        base(b), fixed_bits(f), precision(p), is_signed(s)
    {
        _value = reduce(v);
    }
    numeric& value()
    {
        return _value;
    }
    void value(const numeric& n)
    {
        _value = reduce(n);
    }
    numeric reduce(const numeric& n)
    {
        // std::visit([](auto& a) { std::cerr << "reduce(" << a << ")\n"; }, n);
        /*
         * may be lossy if precision is low... mpf to mpq/mpz might be a lie
         * mpc -> mpf for imaginary = 0
         * mpf -> mpz if no fractional part
         * mpf -> mpq for perfect fractions?
         * mpq -> mpz for denominator = 1
         */
        if (auto q = std::get_if<mpq>(&n); q)
        {
            if (helper::denominator(*q) == 1)
            {
                return helper::numerator(*q);
            }
            return n;
        }
        else if (auto f = std::get_if<mpf>(&n); f)
        {
            if (*f == mpf(0.0))
            {
                return mpz(0);
            }
            // internally, make_quotient will do calculations
            // with a higher precision than the current precision
            // but we will limit the size of the denominator to
            // a reasonable size to keep irrationals from getting
            // turned into rationals
            try
            {
                // make_quotient might return a reducible q
                // so call reduce again
                return reduce(make_quotient(*f, precision / 5));
            }
            catch (const std::exception& e)
            {
                return n;
            }
        }
        else if (auto c = std::get_if<mpc>(&n); c)
        {
            if (c->imag() == mpf(0.0))
            {
                return reduce(c->real());
            }
            return n;
        }
        return n;
    }

  protected:
    numeric _value;

  public:
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
};

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

  protected:
    std::deque<Stack> saved_stacks;

    void make_grammar();
    void make_functions();

    void show_stack();
    std::optional<std::string> get_input();
    std::string get_next_token();
    bool run_one(const std::string& expr);
    bool undo();

    bool debug();
    bool base();
    bool cbase();
    bool fixed_bits();
    bool precision();
    bool unsigned_mode();
    bool angle_mode(e_angle_mode);
    bool mpq_mode(e_mpq_mode);
    bool _running = true;

    std::map<std::string, CalcFunction> _operations;
    std::vector<std::string> _op_names;
    size_t _op_names_max_strlen;
};
