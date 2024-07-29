/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <input.hpp>
#include <optional>
#include <parser.hpp>
#include <print.hpp>
#include <program.hpp>
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
std::vector<std::string_view> sym_ok = {
    "abs",   "acos", "acosh", "asin", "asinh", "atan", "atanh",
    "ceil",  "comb", "cos",   "cosh", "floor", "gcd",  "inv",
    "lcm",   "ln",   "log",   "log2", "neg",   "perm", "pi",
    "round", "sin",  "sinh",  "sqr",  "sqrt",  "tan",  "tanh",
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
        virtual const std::string& name() const final
        {
            static const std::string _name{"OK"};
            return _name;
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
            return symbolic_op::paren;
        }
    };
    static CalcFunction::ptr ok = std::make_shared<OK>();
    const auto& fn = std::find(sym_ok.begin(), sym_ok.end(), name);
    if (fn != operations.end())
    {
        return ok;
    }
    return nullptr;
}
std::string_view fn_get_name(CalcFunction::ptr p)
{
    if (p)
    {
        return p->name();
    }
    return "<unknown-function>";
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

int main(int argc, char* argv[])
{
    lg::debug_level = lg::level::debug;
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
