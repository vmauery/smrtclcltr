/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <input.hpp>
#include <main.hpp>
#include <numeric.hpp>
#include <optional>
#include <parser.hpp>
#include <print.hpp>
#include <string_view>
#include <vector>

std::vector<std::string_view> operations = {
    "!",        "%",           "%ch",     "&",         "*",
    "+",        "-",           "/",       "2date",     "2matrix",
    "2unix",    "<<",          ">>",      "^",         "abs",
    "acos",     "acosh",       "adjoint", "asin",      "asinh",
    "atan",     "atanh",       "base",    "cal",       "cbase",
    "ceil",     "clear",       "comb",    "cos",       "cosh",
    "debug",    "deg",         "depth",   "det",       "drop",
    "drop2",    "dropn",       "dup",     "dup2",      "dupn",
    "e",        "eye",         "f",       "factor",    "fixed_bits",
    "floor",    "gcd",         "grad",    "help",      "i",
    "ij",       "int_type",    "inv",     "lcm",       "ln",
    "log",      "log2",        "mean",    "median",    "modexp",
    "modinv",   "neg",         "now",     "over",      "perm",
    "pi",       "pick",        "polar",   "precision", "prime_factor",
    "product",  "q",           "rad",     "rand",      "rand_dist",
    "range",    "rectangular", "roll",    "rolld",     "rolldn",
    "rolln",    "round",       "signed",  "sin",       "sinh",
    "split",    "sqr",         "sqrt",    "sum",       "swap",
    "tan",      "tanh",        "uconv",   "undo",      "unix",
    "unsigned", "verbose",     "version", "xor",       "|",
    "~",
};
std::vector<std::tuple<smrty::CalcFunction::ptr, std::string_view>>
    regex_operations;

std::optional<std::string_view> auto_complete(std::string_view in, int state)
{
    static size_t last_idx = 0;
    static std::vector<std::string_view> matches;
    if (state == 0)
    {
        last_idx = 0;
        matches.clear();
        // find all the new matches
        for (auto k : operations)
        {
            if (k.starts_with(in))
            {
                matches.emplace_back(k);
            }
        }
    }
    if (last_idx < matches.size())
    {
        return matches[last_idx++];
    }
    return std::nullopt;
}

namespace smrty
{
// this is normally defined in function_library.cpp, but we don't
// actually have any functions defined; only stubs. So define a fake here.
CalcFunction::ptr fn_get_fn_ptr_by_name(std::string_view name)
{
    struct OK : public CalcFunction
    {
        explicit OK(std::string_view name) : fake_name(name)
        {
        }
        virtual const std::string& name() const final
        {
            return fake_name;
        }
        virtual const std::string& help() const final
        {
            static const std::string _help{""};
            return _help;
        }
        virtual bool op(Calculator&) const final
        {
            return false;
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
            if (std::isalnum(fake_name[0]))
            {
                return symbolic_op::paren;
            }
            return symbolic_op::infix;
        }
        std::string fake_name;
    };
    const auto& fn = std::find(operations.begin(), operations.end(), name);
    if (fn == operations.end())
    {
        return nullptr;
    }
    static std::vector<CalcFunction::ptr> used{};
    if (auto it =
            std::find_if(used.begin(), used.end(),
                         [name](auto ptr) { return ptr->name() == name; });
        it != used.end())
    {
        return *it;
    }
    auto rq = std::make_shared<OK>(name);
    used.push_back(rq);
    return rq;
}
std::string_view fn_get_name(CalcFunction::ptr p)
{
    if (p)
    {
        return p->name();
    }
    return "<unknown-function>";
}

// for-loops need to directly access calculator, so need to stub it out here
Calculator::Calculator()
{
}
Calculator::~Calculator()
{
}
void Calculator::set_var(std::string_view, const numeric&)
{
}

} // namespace smrty

void setup_regex_ops()
{
    constexpr auto regexes =
        std::to_array<std::tuple<std::string_view, std::string_view>>(
            {{"dropn", "drop([1-9][0-9]*)"},
             {"dupn", "dup([1-9][0-9]*)"},
             {"int_type", "([su])([1-9][0-9]*)"},
             {"rolln", "roll([1-9][0-9]*)"},
             {"rolldn", "rolld([1-9][0-9]*)"}});
    for (const auto& [name, re] : regexes)
    {
        regex_operations.emplace_back(smrty::fn_get_fn_ptr_by_name(name), re);
    }
}

int usage(std::span<std::string_view> args)
{
    std::print(stderr, "Usage: {} [-v [n]]\n", args[0]);
    return 1;
}

int cpp_main(std::span<std::string_view> args)
{
    lg::debug_level = lg::level::debug;

    size_t i = 1;
    for (; i < args.size(); i++)
    {
        std::string_view arg = args[i];
        if (arg[0] != '-')
        {
            break;
        }
        if (arg == "-v")
        {
            if ((i + 1) < args.size())
            {
                arg = args[++i];
                int v{};
                const auto& [ptr, ec] =
                    std::from_chars(arg.begin(), arg.end(), v);
                if (ec != std::error_code{} || ptr != arg.end())
                {
                    return usage(args);
                }
                lg::debug_level = static_cast<lg::level>(v);
            }
            else
            {
                lg::debug_level = static_cast<lg::level>(
                    static_cast<int>(lg::debug_level) + 1);
            }
        }
    }

    auto input = Input::make_shared(true, auto_complete);
    setup_regex_ops();
    smrty::parser::set_function_lists(operations, regex_operations);

    while (true)
    {
        std::optional<std::string> s = input->readline();
        if (!s || s->size() == 0)
        {
            break;
        }
        // Parse the contents.  If there is an error, just stream it to
        // cerr.
        std::string errmsg{};
        auto stuff = smrty::parser::parse_user_input(
            *s, [&errmsg](std::string_view msg) { errmsg = msg; });
        if (!stuff)
        {
            std::print(stderr, "{}\nParse failure.\n", errmsg);
            continue;
        }

        std::print("Parse OK.\n{}\n", *stuff);
    }
    return 0;
}
