/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#ifdef USE_BASIC_TYPES
#include <numeric>
#else
#include <boost/integer/common_factor_rt.hpp>
#endif
#include <function.hpp>

namespace function
{
namespace factor
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

bool impl(Calculator& calc)
{
    stack_entry e = calc.stack.front();
    mpz* v = std::get_if<mpz>(&e.value());
    if (!v)
    {
        return false;
    }
    calc.stack.pop_front();
    std::vector<mpz> facts = factor_mpz(*v);
    for (const auto& f : facts)
    {
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
    }
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x factor\n"
    "\n"
    "    Returns the factors of the bottom item on the stack\n";

} // namespace factor

namespace prime_factor
{
bool impl(Calculator& calc)
{
    stack_entry e = calc.stack.front();
    mpz* v = std::get_if<mpz>(&e.value());
    if (!v)
    {
        return false;
    }
    calc.stack.pop_front();
    std::vector<mpz> facts = factor::prime_factor(*v);
    for (const auto& f : facts)
    {
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed);
    }
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x prime_factor\n"
    "\n"
    "    Returns the prime factors of the bottom item on the stack\n";

} // namespace prime_factor

namespace gcd
{

#ifdef USE_BASIC_TYPES
#define gcd_fn std::gcd
#else
#define gcd_fn boost::math::gcd
#endif

bool impl(Calculator& calc)
{
    stack_entry e1 = calc.stack[1];
    stack_entry e0 = calc.stack[0];
    mpz* v = std::get_if<mpz>(&e1.value());
    mpz* u = std::get_if<mpz>(&e0.value());
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

auto constexpr help = "\n"
                      "    Usage: x y gcd\n"
                      "\n"
                      "    Returns the greatest common divisor (GCD) of the "
                      "bottom two items on the stack: GCD(x,y)\n";

} // namespace gcd

namespace lcm
{

#ifdef USE_BASIC_TYPES
#define lcm_fn std::lcm
#else
#define lcm_fn boost::math::lcm
#endif

bool impl(Calculator& calc)
{
    stack_entry e1 = calc.stack[1];
    stack_entry e0 = calc.stack[0];
    mpz* v = std::get_if<mpz>(&e1.value());
    mpz* u = std::get_if<mpz>(&e0.value());
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

auto constexpr help = "\n"
                      "    Usage: x y lcd\n"
                      "\n"
                      "    Returns the least common multiple (LCD) of the "
                      "bottom two items on the stack: LCD(x,y)\n";

} // namespace lcm
} // namespace function

namespace functions
{

CalcFunction factor = {function::factor::help, function::factor::impl};
CalcFunction prime_factor = {function::prime_factor::help,
                             function::prime_factor::impl};
CalcFunction gcd = {function::gcd::help, function::gcd::impl};
CalcFunction lcm = {function::lcm::help, function::lcm::impl};

} // namespace functions
