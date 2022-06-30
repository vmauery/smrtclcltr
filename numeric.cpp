/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <chrono>
#include <cmath>
#include <functions/common.hpp>
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

mpq make_quotient(const std::string& s)
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
    mpz den = pow_fn(mpz(10), parts[3].str().size());
    mpq val(sign * (whole * den + num), den);
    if (parts[5].matched)
    {
        int exp = std::stoi(parts[5].str());
        // FIXME: check to see if this will get too big? Then what?
        if (exp < 0)
        {
            val /= to_mpq(function::util::pow(mpz(10), -exp));
        }
        else
        {
            val *= to_mpq(function::util::pow(mpz(10), exp));
        }
    }
    return val;
}

/*
 * make_quotient:
 * original source: https://www.ics.uci.edu/~eppstein/numth/frap.c
 *
 * find rational approximation to given real number
 * David Eppstein / UC Irvine / 8 Aug 1993
 *
 * With corrections from Arno Formella, May 2008
 *
 * based on the theory of continued fractions
 * if x = a1 + 1/(a2 + 1/(a3 + 1/(a4 + ...)))
 * then best approximation is found by truncating this series
 * (with some adjustments in the last term).
 *
 * Note the fraction can be recovered as the first column of the matrix
 *  ( a1 1 ) ( a2 1 ) ( a3 1 ) ...
 *  ( 1  0 ) ( 1  0 ) ( 1  0 )
 * Instead of keeping the sequence of continued fraction terms,
 * we just keep the last partial product of these matrices.
 */

mpq make_quotient(const mpf& f, int digits)
{
    int orig_prec = default_precision;
    set_default_precision(orig_prec + 2);
    const mpf one(1.0l);
    // require error of at least original precision
    const mpf max_error = pow_fn(mpf(10.0l), -orig_prec);
    // std::cerr << "looking for max_error < " << max_error << "\n";

    mpz m[2][2];
    mpf x(f);
    mpz ai;
    mpz maxden = pow_fn(mpz(10), digits);

    /* initialize matrix */
    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    /* loop finding terms until denom gets too big */
    while (m[1][0] * (ai = to_mpz(x)) + m[1][1] <= maxden)
    {
        mpz t;
        t = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;
        if (x == to_mpf(ai))
            break; // AF: division by zero
        x = one / (x - to_mpf(ai));
    }

    mpq result(m[0][0], m[1][0]);
    // throw something; not a perfect representation
    mpf error = abs(f - to_mpf(result));
    // std::cerr << result << ", error = " << error << "\n";

    /* now try other possibility */
    ai = (maxden - m[1][1]) / m[1][0];
    m[0][0] = m[0][0] * ai + m[0][1];
    m[1][0] = m[1][0] * ai + m[1][1];
    mpq result2(m[0][0], m[1][0]);
    mpf error2 = abs(f - to_mpf(result2));
    // std::cerr << result2 << ", error2 = " << error2 << "\n";
    set_default_precision(orig_prec);
    if (error > max_error && error2 > max_error)
    {
        // std::cerr << "lossy mpf->mpq; not into it.\n";
        throw std::invalid_argument("Unable to convert mpf to mpq");
    }
    if (error <= error2)
    {
        return result;
    }
    return result2;
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
#ifdef USE_BASIC_TYPES
                    std::string s = parts[i + 1].str();
                    char* end = nullptr;
                    complex_parts[i] = std::strtold(s.c_str(), &end);
                    if (*end)
                    {
                        throw std::invalid_argument(
                            "input is not a complex number");
                    }
#else
                    std::string s = parts[i + 1].str();
                    complex_parts[i] = mpf(s);
#endif
                }
            }
        }
    }
    return mpc(complex_parts[0], complex_parts[1]);
}

time_ parse_time(const std::string& s)
{
    static std::regex time_literal("("           // start of value capture
                                   "[-+.eE\\d]+" // number bits
                                   ")"           // end of value capture
                                   "("           // start of units capture
                                   "ns|"         // nanoseconds
                                   "us|"         // microseconds
                                   "ms|"         // milliseconds
                                   "s|"          // seconds
                                   "m|"          // minutes
                                   "h|"          // hours
                                   "d"           // days
                                   ")"           // end of units capture
                                   ,
                                   std::regex::ECMAScript);
    std::smatch parts;
    if (!std::regex_match(s, parts, time_literal))
    {
        throw std::invalid_argument("input failed to match time literal regex");
    }
    // [0] entire number
    // [1] value
    // [2] units
    mpq value = parse_mpf(parts[1].str());
    std::string units = parts[2].str();
    if (units == "ns")
    {
        // no-op
    }
    else if (units == "us")
    {
        value *= mpq(1'000ull, 1);
    }
    else if (units == "ms")
    {
        value *= mpq(1'000'000ull, 1);
    }
    else if (units == "s")
    {
        value *= mpq(1'000'000'000ull, 1);
    }
    else if (units == "m")
    {
        value *= mpq(60ull * 1'000'000'000ull, 1);
    }
    else if (units == "h")
    {
        value *= mpq(60ull * 60ull * 1'000'000'000ull, 1);
    }
    else if (units == "d")
    {
        value *= mpq(24ull * 60ull * 60ull * 1'000'000'000ull, 1);
    }
    // all values are in terms of nanoseconds
    value /= mpq(1'000'000'000ull, 1);
    return time_(value, false);
}

numeric reduce_numeric(const numeric& n, int precision)
{
    if (precision == 0)
    {
        precision = default_precision;
    }
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
#ifndef USE_BASIC_TYPES
        // multiprecision mpq does not reduce internally
        mpz c = gcd_fn(helper::numerator(*q), helper::denominator(*q));
        if (c > 1)
        {
            if (c == helper::denominator(*q))
            {
                return helper::numerator(*q) / c;
            }
            return mpq(helper::numerator(*q) / c, helper::denominator(*q) / c);
        }
#endif // !USE_BASIC_TYPES
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
            return reduce_numeric(make_quotient(*f, precision / 5), precision);
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
            return reduce_numeric(c->real(), precision);
        }
        return n;
    }
    return n;
}
