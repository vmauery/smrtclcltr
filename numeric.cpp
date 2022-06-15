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

int default_precision = builtin_default_precision;

std::ostream& operator<<(std::ostream& out, const numeric& n)
{
    return std::visit(
        [&out](const auto& nn) -> std::ostream& { return out << nn; }, n);
}

static mpq make_quotient(const std::string& s)
{
    static std::regex real{
        "([-+])?" // optional sign
        "(\\d+)?" // optional whole part
        "(?:"     // start of non-capturing group
        "\\."     // period
        "("       // start of float capture
        "[0]*"    // optional leading zeros
        "(\\d*)"  // truncated floating part
        ")"       // end of float capture
        ")?"      // end of non-capturing group
        "(?:"     // start of non-capturing group
        "[eE]"    // exponent marker
        "("       // start of exponent capture
        "[+-]?"   // optional sign
        "[0]*"    // optional leading zeros
        "\\d+"    // exponent digits
        ")"       // end of exponent capture
        ")?"      // end of non-capturing group
    };
    std::smatch parts;
    if (!std::regex_match(s, parts, real))
    {
        throw std::runtime_error("input failed to match float regex");
    }
    // [0] entire number
    // [1] sign
    // [2] whole
    // [3] float
    // [4]  float with leading zeros removed
    // [5] exponent with sign
    mpz sign = (parts[1].matched && parts[1].str() == "-") ? -1 : 1;
    mpz whole = parts[2].matched ? parse_mpz(parts[2].str()) : 0;
    mpz num = parts[4].matched ? parse_mpz(parts[4].str()) : 0;
    mpz den = pow(mpz(10), parts[3].str().size());
    mpq val(sign * (whole * den + num), den);
    if (parts[5].matched)
    {
        int exp = std::stoi(parts[5].str());
        // FIXME: check to see if this will get too big? Then what?
        if (exp < 0)
        {
            val /= pow(mpz(10), -exp);
        }
        else
        {
            val *= pow(mpz(10), exp);
        }
    }
    return val;
}

mpq parse_mpf(const std::string& s)
{
    return make_quotient(s);
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
