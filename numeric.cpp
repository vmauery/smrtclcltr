/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric.hpp>
#include <regex>
#include <variant>

int default_precision = 100;

std::ostream& operator<<(std::ostream& out, const numeric& n)
{
    return std::visit(
        [&out](const auto& nn) -> std::ostream& { return out << nn; }, n);
}

mpz make_fixed(const mpz& v, int bits, bool is_signed)
{
    mpz one(1);
    mpz max_half = one << (bits - 1);
    mpz max_signed_value = max_half - 1;
    mpz max_mask = (one << bits) - 1;

    mpz s1;
    if (is_signed)
    {
        s1 = v & max_mask;
        if (s1 > max_signed_value)
        {
            s1 = -(max_half + ((~(v & max_mask)) + 1) % max_half);
        }
    }
    else
    {
        s1 = v & max_mask;
    }
    return s1;
}

// accept the form 3.2e7+4.3e6i or (3.2e7,4.3e6)
const std::regex cmplx_regex[] = {
    std::regex("^(?=[ij.\\d+-])([-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]"
               "?\\d+)?(?![ij.\\d]))?([+-]?(?:(?:\\d+(?:\\.\\d*)?|\\.\\d+)("
               "?:[eE][+-]?\\d+)?)?)?[ij]$",
               std::regex::ECMAScript),
    std::regex("^[(]([-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?),(["
               "-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?)[)]$",
               std::regex::ECMAScript),
};
#ifdef USE_BASIC_TYPES
mpc parse_mpc(const std::string& s)
{
    mpf complex_parts[2]{};
    for (int j = 0; j < 2; j++)
    {
        std::smatch parts;
        if (std::regex_search(s, parts, cmplx_regex[j]))
        {
            for (unsigned long i = 0; i < 2; i++)
            {
                if (parts[i + 1].matched)
                {
                    std::string s = parts[i + 1].str();
                    char* end = nullptr;
                    complex_parts[i] = std::strtold(s.c_str(), &end);
                    if (*end)
                    {
                        throw std::invalid_argument(
                            "input is not a complex number");
                    }
                }
            }
        }
    }
    return mpc(complex_parts[0], complex_parts[1]);
}
#else
mpc parse_mpc(const std::string& s)
{
    mpf complex_parts[2]{};
    for (int j = 0; j < 2; j++)
    {
        std::smatch parts;
        if (std::regex_search(s, parts, cmplx_regex[j]))
        {
            for (unsigned long i = 0; i < 2; i++)
            {
                if (parts[i + 1].matched)
                {
                    std::string s = parts[i + 1].str();
                    complex_parts[i] = mpf(s);
                }
            }
        }
    }
    return mpc(complex_parts[0], complex_parts[1]);
}
#endif
