/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace smrty
{
namespace function
{

struct drop : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"drop"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: drop\n"
            "\n"
            "    Removes the bottom item on the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // single arg using num_args
        calc.stack.pop_front();
        return true;
    }
    int num_args() const final
    {
        return 1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct dropn : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"dropn"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x dropn\n"
            "           drop[1-9][0-9]*\n"
            "\n"
            "    Removes the x bottom items on the stack\n"
            "    regex variant supports drop2 or drop18\n"
            // clang-format on
        };
        return _help;
    }
    bool drop(Calculator& calc, size_t count) const
    {
        if (calc.stack.size() < count)
        {
            throw insufficient_args();
        }
        for (size_t i = 0; i < count; i++)
        {
            calc.stack.pop_front();
        }
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // single are using num_args
        stack_entry& n = calc.stack.front();
        if (n.unit() != units::unit())
        {
            throw units_prohibited();
        }
        size_t count = 0;
        if (auto np = std::get_if<mpz>(&n.value()); np != nullptr)
        {
            count = static_cast<size_t>(*np);
        }
        else if (count == 0)
        {
            throw std::invalid_argument("count cannot be zero");
        }
        calc.stack.pop_front();
        return drop(calc, count);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string_view n = match[1];
        size_t count{};
        if (std::from_chars(n.data(), n.data() + n.size(), count).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        return drop(calc, count);
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "drop([1-9][0-9]*)";
        return _regex;
    }
    int num_args() const final
    {
        return -1;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct dup : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"dup"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x dup\n"
            "\n"
            "    Duplicates the bottom item on the stack (x x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // single arg using num_args
        stack_entry a = calc.stack.front();
        calc.stack.push_front(a);
        return true;
    }
    int num_args() const final
    {
        return 1;
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

struct dupn : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"dupn"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x0 x1..xn n dupn\n"
            "           x0 x1..xn dup[1-9][0-9]*\n"
            "\n"
            "    Duplicates the bottom n items on the stack\n"
            "    regex variant allows for dup2 or dup18\n"
            // clang-format on
        };
        return _help;
    }
    bool dup(Calculator& calc, size_t count) const
    {
        if (calc.stack.size() < count)
        {
            throw insufficient_args();
        }
        for (size_t i = 0; i < count; i++)
        {
            calc.stack.push_front(calc.stack[count - 1]);
        }
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry& n = calc.stack.front();
        size_t count = 0;
        if (auto np = std::get_if<mpz>(&n.value()); np != nullptr)
        {
            count = static_cast<size_t>(*np);
        }
        else if (count == 0)
        {
            throw std::invalid_argument("count cannot be zero");
        }
        // remove N
        calc.stack.pop_front();
        return dup(calc, count);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string_view n = match[1];
        size_t count{};
        if (std::from_chars(n.data(), n.data() + n.size(), count).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        return dup(calc, count);
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "dup([1-9][0-9]*)";
        return _regex;
    }
    int num_args() const final
    {
        return -2;
    }
    int num_resp() const final
    {
        return -1;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct over : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"over"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
    "\n"
    "    Usage: x over\n"
    "\n"
    "    Pushes the second to bottom item onto the stack as the bottom\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry a = calc.stack[1];
        calc.stack.push_front(a);
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
        return symbolic_op::none;
    }
};

struct swap : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"swap"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x y swap\n"
            "\n"
            "    Swaps the bottom two items on the stack (y x)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry a = calc.stack.front();
        calc.stack.pop_front();
        stack_entry b = calc.stack.front();
        calc.stack.pop_front();
        calc.stack.push_front(std::move(a));
        calc.stack.push_front(std::move(b));
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct clear : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"clear"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: clear\n"
            "\n"
            "    Removes all items from the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        calc.stack.clear();
        return true;
    }
    int num_args() const final
    {
        return 0;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct depth : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"depth"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: depth\n"
            "\n"
            "    Returns the number of items on the stack\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        stack_entry e(mpz(calc.stack.size()), calc.config.base,
                      calc.config.fixed_bits, calc.config.precision,
                      calc.config.is_signed, calc.flags);
        calc.stack.push_front(std::move(e));
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
        return symbolic_op::none;
    }
};

struct roll : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"roll"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: roll\n"
            "\n"
            "    Rolls the stack up (top item becomes new bottom)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry a = calc.stack.back();
        calc.stack.pop_back();
        calc.stack.push_front(a);
        return true;
    }
    int num_args() const final
    {
        return 2;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct rolln : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rolln"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x rolln\n"
            "           x0 x1..xn roll[1-9][0-9]*\n"
            "\n"
            "    Rolls the bottom n stack items up\n"
            "    regex variant supports roll2 or roll18\n"
            // clang-format on
        };
        return _help;
    }
    bool roll(Calculator& calc, size_t count) const
    {
        if (calc.stack.size() < count)
        {
            throw insufficient_args();
        }
        // pick count and push it at front, removing it from original location
        stack_entry a = calc.stack[count - 1];
        calc.stack.erase(calc.stack.begin() + count - 1);
        calc.stack.push_front(a);
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // three args using num_args
        stack_entry& n = calc.stack.front();
        if (n.unit() != units::unit())
        {
            throw units_prohibited();
        }
        size_t count = 0;
        if (auto np = std::get_if<mpz>(&n.value()); np != nullptr)
        {
            count = static_cast<size_t>(*np);
        }
        else if (count == 0)
        {
            throw std::invalid_argument("count cannot be zero");
        }
        // remove N
        calc.stack.pop_front();
        return roll(calc, count);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string_view n = match[1];
        size_t count{};
        if (std::from_chars(n.data(), n.data() + n.size(), count).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        return roll(calc, count);
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "roll([1-9][0-9]*)";
        return _regex;
    }
    int num_args() const final
    {
        return 3;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct rolld : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rolld"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rolld\n"
            "\n"
            "    Rolls the stack down (bottom item becomes new top)\n"
            // clang-format on
        };
        return _help;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry a = calc.stack.front();
        calc.stack.pop_front();
        calc.stack.push_back(a);
        return true;
    }
    int num_args() const final
    {
        return -2;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct rolldn : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"rolldn"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: rolldn\n"
            "           x0 x1..xn rolld[1-9][0-9]*\n"
            "\n"
            "    Rolls the bottom n stack items down\n"
            "    regex variant supports rolld2 or rolld18\n"
            // clang-format on
        };
        return _help;
    }
    bool rolld(Calculator& calc, size_t count) const
    {
        if (calc.stack.size() < count)
        {
            throw insufficient_args();
        }
        // pop bottom and push it at count
        stack_entry a = calc.stack.front();
        calc.stack.pop_front();
        calc.stack.insert(calc.stack.begin() + count - 1, std::move(a));
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // three args using num_args
        stack_entry& n = calc.stack.front();
        size_t count = 0;
        if (auto np = std::get_if<mpz>(&n.value()); np != nullptr)
        {
            count = static_cast<size_t>(*np);
        }
        else if (count == 0)
        {
            throw std::invalid_argument("count cannot be zero");
        }
        // remove N
        calc.stack.pop_front();
        return rolld(calc, count);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string_view n = match[1];
        size_t count{};
        if (std::from_chars(n.data(), n.data() + n.size(), count).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        return rolld(calc, count);
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "rolld([1-9][0-9]*)";
        return _regex;
    }
    int num_args() const final
    {
        return -3;
    }
    int num_resp() const final
    {
        return 0;
    }
    symbolic_op symbolic_usage() const final
    {
        return symbolic_op::none;
    }
};

struct pick : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"pick"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x pick\n"
            "           x0 x1..xn pick[1-9][0-9]*\n"
            "\n"
            "    Returns the item x entries up the stack\n"
            "    regex variant supports pick2 or pick18\n"
            // clang-format on
        };
        return _help;
    }
    bool pickN(Calculator& calc, size_t count) const
    {
        // duplicate item N
        calc.stack.push_front(calc.stack[count - 1]);
        return true;
    }
    virtual bool op(Calculator& calc) const final
    {
        // two args using num_args
        stack_entry& n = calc.stack.front();
        size_t count = 0;
        if (auto np = std::get_if<mpz>(&n.value()); np != nullptr)
        {
            count = static_cast<size_t>(*np);
        }
        else if (count == 0)
        {
            throw std::invalid_argument("count cannot be zero");
        }
        // remove N
        calc.stack.pop_front();
        return pickN(calc, count);
    }
    virtual bool reop(Calculator& calc,
                      const std::vector<std::string>& match) const final
    {
        std::string_view n = match[1];
        size_t count{};
        if (std::from_chars(n.data(), n.data() + n.size(), count).ec !=
            std::errc{})
        {
            throw std::invalid_argument("failed to parse integer from string");
        }
        return pickN(calc, count);
    }
    virtual const std::string_view regex() const final
    {
        static const auto _regex = "pick([1-9][0-9]*)";
        return _regex;
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

} // namespace function
} // namespace smrty

register_calc_fn(drop);
register_calc_fn(dropn);
register_calc_fn(dup);
register_calc_fn(dupn);
register_calc_fn(over);
register_calc_fn(swap);
register_calc_fn(clear);
register_calc_fn(depth);
register_calc_fn(roll);
register_calc_fn(rolln);
register_calc_fn(rolld);
register_calc_fn(rolldn);
register_calc_fn(pick);
