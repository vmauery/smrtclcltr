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
std::vector<std::tuple<size_t, std::string_view>> regex_operations = {
    {36, "drop([1-9][0-9]*)"},   {39, "dup([1-9][0-9]*)"},
    {44, "([su])([1-9][0-9]*)"}, {79, "roll([1-9][0-9]*)"},
    {80, "rolld([1-9][0-9]*)"},
};

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

int main(int argc, char* argv[])
{
    lg::debug_level = lg::level::debug;
    auto input = Input::make_shared(true, auto_complete);
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
