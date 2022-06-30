/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <cmath>
#include <function.hpp>

namespace function
{
namespace util
{

std::vector<mpz> factor_mpz(const mpz& x)
{
    std::vector<mpz> facts;
    mpz maxf = to_mpz(ceil_fn(sqrt(mpf(x))));
    mpz n(2);
    while (n < maxf)
    {
        if (x % n == 0)
        {
            facts.push_back(n);
            facts.push_back(x / n);
        }
        n++;
    }
    std::sort(facts.begin(), facts.end());
    return facts;
}

mpz next_factor(const mpz& x, mpz& n)
{
    mpz maxf = to_mpz(ceil_fn(sqrt(mpf(x))));
    while (n <= maxf)
    {
        if (x % n == 0)
        {
            return x / n;
        }
        n++;
    }
    return 0;
}

std::vector<mpz> prime_factor(mpz x)
{
    std::vector<mpz> facts;
    mpz maxf = to_mpz(ceil_fn(sqrt(mpf(x))));
    mpz n(2);
    while (n <= maxf)
    {
        mpz nf = next_factor(x, n);
        if (nf != 0)
        {
            facts.push_back(n);
            x = nf;
            maxf = to_mpz(ceil_fn(sqrt(mpf(x))));
            n = 1;
        }
        else
        {
            facts.push_back(x);
        }
        n++;
    }
    std::sort(facts.begin(), facts.end());
    return facts;
}

} // namespace util

struct factor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"factor"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x factor\n"
            "\n"
            "    Returns the factors of the bottom item on the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (!v)
        {
            return false;
        }
        calc.stack.pop_front();
        std::vector<mpz> facts = util::factor_mpz(*v);
        for (const auto& f : facts)
        {
            calc.stack.emplace_front(
                f, calc.config.base, calc.config.fixed_bits,
                calc.config.precision, calc.config.is_signed);
        }
        return true;
    }
};

struct prime_factor : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"prime_factor"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x prime_factor\n"
            "\n"
            "    Returns the prime factors of the bottom item on the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (!v)
        {
            return false;
        }
        calc.stack.pop_front();
        std::vector<mpz> facts = util::prime_factor(*v);
        for (const auto& f : facts)
        {
            calc.stack.emplace_front(
                f, calc.config.base, calc.config.fixed_bits,
                calc.config.precision, calc.config.is_signed);
        }
        return true;
    }
};

struct gcd : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"gcd"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y gcd\n"
            "\n"
            "    Returns the greatest common divisor (GCD) of the "
            "bottom two items on the stack: GCD(x,y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        if (e0.unit() != units::unit() || e1.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e1.value());
        const mpz* u = std::get_if<mpz>(&e0.value());
        if (!v || !u)
        {
            return false;
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = gcd_fn(*v, *u);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
    }
};

struct lcm : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"lcm"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y lcd\n"
            "\n"
            "    Returns the least common multiple (LCD) of the "
            "bottom two items on the stack: LCD(x,y)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        if (e0.unit() != units::unit() || e1.unit() != units::unit())
        {
            return false;
        }
        const mpz* v = std::get_if<mpz>(&e1.value());
        const mpz* u = std::get_if<mpz>(&e0.value());
        if (!v || !u)
        {
            return false;
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = lcm_fn(*v, *u);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
        return true;
    }
};

} // namespace function

register_calc_fn(factor);
register_calc_fn(prime_factor);
register_calc_fn(gcd);
register_calc_fn(lcm);
