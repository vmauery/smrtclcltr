/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/seed_seq.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <function.hpp>
#include <functions/common.hpp>
#include <random>

namespace smrty
{
namespace function
{
namespace util
{

mpz comb(const mpz& x, const mpz& y)
{
    return factorial(x) / (factorial(y) * factorial(x - y));
}

mpz perm(const mpz& x, const mpz& y)
{
    return factorial(x) / factorial(x - y);
}

// private namespace for generator used by random functions
namespace
{
boost::random::mt19937& gen(void)
{
    static std::shared_ptr<boost::random::mt19937> _gen;
    if (!_gen)
    {
        std::random_device rd;
        _gen = std::make_shared<boost::random::mt19937>(rd());
    }
    return *_gen;
}
} // namespace

#ifdef USE_BASIC_TYPES
mpz rand_dist(const mpz& low, const mpz& high)
{
    using sub_type = mpz::value_type;
    boost::random::uniform_int_distribution<sub_type> dist(low.value,
                                                           high.value);
    return mpz{dist(gen())};
}

mpf rand_dist(const mpf& low, const mpf& high)
{
    using sub_type = mpf::value_type;
    boost::random::uniform_real_distribution<sub_type> dist(low.value,
                                                            high.value);
    return mpf{dist(gen())};
}

#else // USE_BASIC_TYPES
mpz rand_dist(const mpz& low, const mpz& high)
{
    boost::random::uniform_int_distribution<mpz> dist(low, high);
    return dist(gen());
}

mpf rand_dist(const mpf& low, const mpf& high)
{
    boost::random::uniform_real_distribution<mpf> dist(low, high);
    return dist(gen());
}

#endif // USE_BASIC_TYPES

mpq rand(unsigned long bits)
{
    mpz high{1};
    high <<= bits;
    return mpq(rand_dist(zero, high), high);
}

} // namespace util

struct combination : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"comb"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y comb\n"
            "\n"
            "    Return the statistical combination of the bottom "
            "two items on the stack\n"
            "\n"
            "    Use when order doesn't matter in the choice.\n"
            "\n"
            "    No repetition, use: x y comb\n"
            "    / x \\       x!\n"
            "    |    | = --------\n"
            "    \\ y /    y!(x-y)!\n"
            "\n"
            "    With repetition, use: x y swap over + 1 - swap comb\n"
            "                  or use: x y 1 - over + swap 1 - comb\n"
            "\n"
            "    / x+y-1 \\     / x+y-1 \\     (x+y-1)!\n"
            "    |        | =  |        | =  --------\n"
            "    \\   y   /     \\  x-1  /     y!(x-y)!\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args provided by num_args
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        if (e0.unit() != units::unit() || e1.unit() != units::unit())
        {
            throw units_prohibited();
        }
        const mpz* x = std::get_if<mpz>(&e1.value());
        const mpz* y = std::get_if<mpz>(&e0.value());
        if (!x || !y || (*y > *x))
        {
            throw std::invalid_argument("requires integers such that x < y");
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = util::comb(*x, *y);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct permutation : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"perm"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "    Usage: x y perm\n"
            "\n"
            "    Return the statistical permutation of the bottom "
            "two items on the stack\n"
            "\n"
            "    Use when order matters in the choice.\n"
            "\n"
            "    No repetition, use: x y perm\n"
            "                                        x!\n"
            "    order y things from x available = ------\n"
            "                                      (x-y)!\n"
            "\n"
            "    With repetition, use: x y ^\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args provided by num_args
        stack_entry e1 = calc.stack[1];
        stack_entry e0 = calc.stack[0];
        if (e0.unit() != units::unit() || e1.unit() != units::unit())
        {
            throw units_prohibited();
        }
        const mpz* x = std::get_if<mpz>(&e1.value());
        const mpz* y = std::get_if<mpz>(&e0.value());
        if (!x || !y || (*y > *x))
        {
            throw std::invalid_argument("requires integers such that x < y");
        }
        calc.stack.pop_front();
        calc.stack.pop_front();
        mpz f = util::perm(*x, *y);
        calc.stack.emplace_front(f, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct mean : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"mean"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x1 x2... xn n mean\n"
            "           { x1 x2... xn } mean\n"
            "\n"
            "    Returns the mean of the bottom n items on the stack\n"
            "    or the bottom single item that is a list\n"
            // clang-format on
        };
        return _help;
    }
    bool mean_from_stack(Calculator& calc, const mpz& v) const
    {
        size_t count = static_cast<size_t>(v) - 1;
        auto n = calc.stack.front();
        calc.stack.pop_front();
        for (; count > 0; count--)
        {
            // no need to check the return;
            // it is either true or it throws an exception
            util::add_from_stack(calc);
        }
        calc.stack.push_front(n);
        return util::divide_from_stack(calc);
    }
    bool mean_from_list(Calculator& calc, const list& lst) const
    {
        list::element_type sum{mpz{0}};
        for (const auto& v : lst.values)
        {
            sum = sum + v;
        }
        auto mean = sum / list::element_type{mpz{lst.size()}};
        calc.stack.pop_front();
        std::visit(
            [&](auto&& v) {
                calc.stack.emplace_front(
                    numeric{v}, calc.config.base, calc.config.fixed_bits,
                    calc.config.precision, calc.config.is_signed, calc.flags);
            },
            std::move(mean));
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // required entry provided by num_args
        stack_entry& e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            throw units_prohibited();
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (v && (*v < static_cast<mpz>(calc.stack.size())))
        {
            return mean_from_stack(calc, *v);
        }
        if (auto lp = std::get_if<list>(&e.value()); lp)
        {
            return mean_from_list(calc, *lp);
        }
        throw std::invalid_argument("Invalid aruments for mean");
    }
    int num_args() const final
    {
        return -1;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct median : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"median"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x1 x2... xn n median\n"
            "\n"
            "    Returns the median of the bottom n items on the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // first two arguments provided by num_args
        stack_entry e = calc.stack.front();
        if (e.unit() != units::unit())
        {
            throw units_prohibited();
        }
        const mpz* v = std::get_if<mpz>(&e.value());
        if (!v || (*v > 1000000000) || (*v <= mpz{0}) ||
            (*v >= static_cast<mpz>(calc.stack.size())))
        {
            throw std::invalid_argument(
                "n must be an integer greater than 0 and less the stack depth");
        }
        units::unit first_unit = calc.stack.front().unit();
        size_t count = static_cast<size_t>(*v);
        if (calc.stack.size() < (count + 1))
        {
            throw std::invalid_argument("Insufficient arguments");
        }
        calc.stack.pop_front();
        std::vector<std::variant<mpz, mpq, mpf>> items{};
        for (; count > 0; count--)
        {
            stack_entry e = calc.stack.front();
            if (e.unit() != first_unit)
            {
                throw units_mismatch();
            }
            auto& v = e.value();
            if (auto zp = std::get_if<mpz>(&v); zp)
            {
                items.emplace_back(*zp);
            }
            else if (auto qp = std::get_if<mpq>(&v); qp)
            {
                items.emplace_back(*qp);
            }
            else if (auto fp = std::get_if<mpf>(&v); fp)
            {
                items.emplace_back(*fp);
            }
            else
            {
                throw std::invalid_argument(
                    "items must all be integer, rational, or real");
            }
            calc.stack.pop_front();
        }
        std::sort(items.begin(), items.end());
        auto stack_inserter = [&](auto&& v) {
            calc.stack.emplace_front(numeric{std::move(v)}, first_unit,
                                     calc.config.base, calc.config.fixed_bits,
                                     calc.config.precision,
                                     calc.config.is_signed, calc.flags);
        };
        if (items.size() % 2)
        {
            auto& item = items[items.size() / 2];
            std::visit(stack_inserter, item);
            return true;
        }
        else
        {
            auto& item1 = items[(items.size() / 2) - 1];
            std::visit(stack_inserter, item1);
            auto& item2 = items[items.size() / 2];
            std::visit(stack_inserter, item2);
            calc.stack.emplace_front(
                two, calc.config.base, calc.config.fixed_bits,
                calc.config.precision, calc.config.is_signed, calc.flags);
            mean mean_fn{};
            return mean_fn.op(calc);
        }
    }
    int num_args() const final
    {
        return -2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct rand : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rand"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rand\n"
            "\n"
            "    Returns a uniformly distributed random float between 0 and 1\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        constexpr double log2_10 = 3.325;
        mpq r = util::rand(
            static_cast<long long>(std::ceil(calc.config.precision * log2_10)));
        calc.stack.emplace_front(r, calc.config.base, calc.config.fixed_bits,
                                 calc.config.precision, calc.config.is_signed,
                                 calc.flags);
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};

struct rand_dist : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rand_dist"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y rand_dist\n"
            "\n"
            "    Returns a uniformly distributed random number in the range of [x, y]\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        return two_arg_conv<ITypes<mpq>, OTypes<mpf>, LTypes<mpz, mpf>>::op(
            calc,
            [](const auto& a, const auto& b, const units::unit& ua,
               const units::unit& ub) -> std::tuple<numeric, units::unit> {
                if (ua != units::unit{} || ua != ub)
                {
                    throw units_prohibited();
                }
                if (b <= a)
                {
                    throw std::invalid_argument("y must be > x");
                }
                if constexpr (!same_type_v<decltype(a), decltype(b)>)
                {
                    if constexpr (same_type_v<decltype(a), mpf>)
                    {
                        return {util::rand_dist(a, mpf{b}), ua};
                    }
                    else
                    {
                        return {util::rand_dist(mpf{a}, b), ua};
                    }
                }
                else
                {
                    return {util::rand_dist(a, b), ua};
                }
            });
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::paren;
    }
};
} // namespace function
} // namespace smrty

register_calc_fn(combination);
register_calc_fn(permutation);
register_calc_fn(mean);
register_calc_fn(median);
register_calc_fn(rand);
register_calc_fn(rand_dist);
