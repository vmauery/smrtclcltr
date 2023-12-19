/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <charconv>
#include <chrono>
#include <cmath>
#include <functions/common.hpp>
#include <iostream>
#include <numeric.hpp>
#include <regex>
#include <variant>

int default_precision = builtin_default_precision;

mpq make_quotient(std::string_view s)
{
    static std::regex real{
        "([-+])?"    // optional sign
        "([,\\d]+)?" // optional whole part
        "(?:"        // start of non-capturing group
        "\\."        // period
        "("          // start of float capture
        "[0]*"       // optional leading zeros
        "(\\d*)"     // truncated floating part
        ")"          // end of float capture
        ")?"         // end of non-capturing group
        "(?:"        // start of non-capturing group
        "[eE]"       // exponent marker
        "("          // start of exponent capture
        "[+-]?"      // optional sign
        "[0]*"       // optional leading zeros
        "\\d+"       // exponent digits
        ")"          // end of exponent capture
        ")?"         // end of non-capturing group
    };
    std::cmatch parts;
    if (!std::regex_match(s.begin(), s.end(), parts, real))
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
    mpz whole = parts[2].matched ? parse_mpz(parts[2].str()) : zero;
    mpz num = parts[4].matched ? parse_mpz(parts[4].str()) : zero;
    mpz den = powul_fn(ten, parts[3].str().size());
    mpq val(sign * (whole * den + num), den);
    if (parts[5].matched)
    {
        int exp{std::stoi(parts[5].str())};
        // FIXME: check to see if this will get too big? Then what?
        if (exp < zero)
        {
            val /= static_cast<mpq>(powul_fn(ten, -exp));
        }
        else
        {
            val *= static_cast<mpq>(powul_fn(ten, exp));
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
static std::tuple<mpq, mpf> calculate_quotient(const mpf& f, int digits)
{
    int orig_prec = default_precision;
    set_default_precision(orig_prec + 2);
    const mpf one(1.0l);

    mpz m[2][2];
    mpf x(f);
    mpz ai;
    mpz maxden = powul_fn(ten, digits);

    /* initialize matrix */
    m[0][0] = m[1][1] = 1;
    m[0][1] = m[1][0] = 0;

    /* loop finding terms until denom gets too big */
    while (m[1][0] * (ai = static_cast<mpz>(x)) + m[1][1] <= maxden)
    {
        mpz t;
        t = m[0][0] * ai + m[0][1];
        m[0][1] = m[0][0];
        m[0][0] = t;
        t = m[1][0] * ai + m[1][1];
        m[1][1] = m[1][0];
        m[1][0] = t;
        if (x == static_cast<mpf>(ai))
            break; // AF: division by zero
        x = one / (x - static_cast<mpf>(ai));
    }

    mpq result(m[0][0], m[1][0]);
    mpf error = abs_fn(f - static_cast<mpf>((result)));
    lg::debug("Q: {}, error = {}\n", result, error);

    /* now try other possibility */
    ai = (maxden - m[1][1]) / m[1][0];
    m[0][0] = m[0][0] * ai + m[0][1];
    m[1][0] = m[1][0] * ai + m[1][1];
    mpq result2(m[0][0], m[1][0]);
    mpf error2 = abs_fn(f - static_cast<mpf>(result2));
    lg::debug("Q: {}, error2 = {}\n", result2, error2);
    set_default_precision(orig_prec);
    if (error <= error2)
    {
        return {result, error};
    }
    return {result2, error2};
}

/* attempt making a rational, but maybe fail if the losses are too great */
mpq make_quotient(const mpf& f, int digits)
{
    // require error of at least original precision
    const mpf max_error = pow_fn(
        mpf(10.0l), static_cast<mpf>(static_cast<mpz>(-default_precision)));

    auto&& [q, error] = calculate_quotient(f, digits);
    if (error > max_error)
    {
        // throw something; not a perfect representation
        lg::info("lossy mpf->mpq; not into it.\n");
        throw std::invalid_argument("Unable to convert mpf to mpq");
    }
    return q;
}

/* force a float to a rational, dropping losses */
mpq make_quotient(const mpf& f)
{
    auto&& [q, _] = calculate_quotient(f, default_precision);
    return q;
}

mpq parse_mpf(std::string_view s)
{
    return make_quotient(s);
}

mpz make_fixed(const mpz& v, int bits, bool is_signed)
{
    mpz bitz{bits};
    mpz max_half = one << static_cast<int>(bitz - one);
    mpz max_signed_value = max_half - one;
    mpz max_mask = (one << static_cast<int>(bitz)) - one;

    mpz s1;
    if (is_signed)
    {
        s1 = v & max_mask;
        if (s1 > max_signed_value)
        {
            s1 = -(max_half + ((~(v & max_mask)) + one) % max_half);
        }
    }
    else
    {
        s1 = v & max_mask;
    }
    return s1;
}

#ifndef USE_BASIC_TYPES
mpz parse_mpz(std::string_view s, int base)
{
    std::string nc{};
    // remove any commas
    if (s.find(",") != std::string::npos)
    {
        nc = s;
        nc.erase(std::remove(nc.begin(), nc.end(), ','), nc.end());
        s = nc;
    }

    // if s is base 10 (does not start with 0), and has an e
    // peel off the e and handle that separately.
    if (size_t epos{}; base == 10 && (epos = s.find('e')) != std::string::npos)
    {
        mpz ret(s.substr(0, epos));
        auto exps = s.substr(epos + 1);
        unsigned int exp{};
        auto [ptr, ec] =
            std::from_chars(exps.data(), exps.data() + exps.size(), exp);
        if (ec != std::errc() || ptr != (exps.data() + exps.size()))
        {
            throw std::invalid_argument("input has an invalid exponent");
        }
        ret *= mpz(powul_fn(mpf(10), static_cast<int>(exp)));
        return ret;
    }
    return mpz(s);
}
#else  // USE_BASIC_TYPES

namespace smrty
{
mpz powul(const mpz& base, int exponent)
{
    mpz b{base}, e{exponent};
    lg::debug("powul(base={}, exponent={})\n", base, exponent);
    lg::debug("powul(b={}, e={})\n", b, e);
    if (e < zero)
    {
        throw std::runtime_error("invalid negative exponent");
    }
    mpz result{1};
    while (e > zero)
    {
        lg::debug("powul: r={}, b={}, e={})\n", result, b, e);
        if (e & one)
        {
            result *= b;
        }
        e = e >> 1;
        b *= b;
    }
    return result;
}
std::tuple<mpz, mpz> powl(const mpz& base, int exponent)
{
    mpz result = powul(base, std::abs(exponent));
    if (exponent < zero)
    {
        return {one, result};
    }
    return {result, one};
}

} // namespace smrty

mpz parse_mpz(std::string_view s, int base)
{
    std::string nc{};
    // remove any commas
    if (s.find(",") != std::string::npos)
    {
        nc = s;
        nc.erase(std::remove(nc.begin(), nc.end(), ','), nc.end());
        s = nc;
    }
    if (base != 10)
    {
        // skip the prefix
        s = s.substr(2);
    }
    mpz ret{};
    auto [ptr, ec] =
        std::from_chars(s.data(), s.data() + s.size(), ret.value, base);
    if (ec != std::errc{})
    {
        throw std::invalid_argument(std::error_condition(ec).message());
    }
    // possible exponent?
    if (ptr != s.end())
    {
        size_t end = ptr - s.begin();
        if (s[end] == 'e')
        {
            auto exps = s.substr(end + 1);
            end = 0;
            mpz exp{};
            auto [ptr, ec] =
                std::from_chars(exps.begin(), exps.end(), exp.value);
            if (ptr != exps.end())
            {
                throw std::invalid_argument("input has an invalid exponent");
            }
            ret *= powul_fn(ten, exp);
        }
        else
        {
            throw std::invalid_argument("input is not an integer");
        }
    }
    return ret;
}
#endif // USE_BASIC_TYPES

// accept the form 3.2e7+4.3e6i, (3.2e7,4.3e6), or (3.2e7,<4.3e6)
const std::regex cmplx_regex[] = {
    std::regex("^(?=[ij.\\d+-])([-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]"
               "?\\d+)?(?![ij.\\d]))?([+-]?(?:(?:\\d+(?:\\.\\d*)?|\\.\\d+)("
               "?:[eE][+-]?\\d+)?)?)[ij]$",
               std::regex::ECMAScript),
    std::regex("^[(]([-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?),(["
               "-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?)[)]$",
               std::regex::ECMAScript),
};
mpc parse_mpc(std::string_view s)
{
    mpf real{};
    mpf imag{1};
    for (int j = 0; j < 2; j++)
    {
        std::cmatch parts;
        if (std::regex_search(s.begin(), s.end(), parts, cmplx_regex[j]))
        {
            if (parts[1].matched)
            {
                std::string s = parts[1].str();
                lg::debug("match[1] = '{}'\n", s);
                real = mpf{s};
            }
            if (parts[2].matched)
            {
                std::string s = parts[2].str();
                lg::debug("match[2] = '{}'\n", s);
                if (s[0] == '+')
                {
                    s = s.substr(1);
                    lg::debug("match[2] = '{}'\n", s);
                }
                else if (s == "-")
                {
                    s += '1';
                    lg::debug("match[2] = '{}'\n", s);
                }
                if (s.size())
                {
                    imag = mpf{s};
                }
            }
        }
    }
    return mpc{real, imag};
}

matrix parse_matrix(std::string_view s)
{
    smrty::Calculator& calc = smrty::Calculator::get();
    Parser& parser = calc.get_parser();
    // strip off the braces and parse the values
    auto next_not_brace = [](std::string_view sv) -> std::string_view {
        auto next_pos = sv.find_first_not_of("[]");
        if (next_pos == std::string_view::npos)
        {
            return {};
        }
        return sv.substr(next_pos);
    };
    lg::debug("s is '{}'\n", s);
    std::string_view next = next_not_brace(s);
    size_t rows = 0;
    size_t cols = 0;
    size_t rows_cols = 0;
    std::vector<matrix::element_type> values{};
    do
    {
        lg::debug("next is '{}'\n", next);
        const auto& [parse_ok, t, more] = parser.parse_next(next);
        if (parse_ok)
        {
            lg::debug("parsed: {}\n", *t);
            if (rows == 0)
            {
                cols++;
            }
            if (t->type == Parser::mpz_type)
            {
                values.emplace_back(mpz{t->value});
            }
            else if (t->type == Parser::mpf_type)
            {
                values.emplace_back(parse_mpf(t->value));
            }
            else if (t->type == Parser::mpc_type)
            {
                values.emplace_back(parse_mpc(t->value));
            }
            else if (t->type == Parser::mpq_type)
            {
                values.emplace_back(mpq{t->value});
            }
            else
            {
                throw std::invalid_argument(
                    std::format("unsupported matrix entry type {}",
                                Parser::expr_types[t->type]));
            }
            rows_cols++;
        }
        next = more;
        if (!parse_ok)
        {
            rows++;
            lg::debug("end of row {}: cols={}, cows_cols={}\n", rows, cols,
                      rows_cols);
            if ((cols != rows_cols) &&
                ((rows == 2) && ((rows_cols % cols) != 0)))
            {
                throw std::invalid_argument(
                    "Failed to parse matrix: column count is inconsistent");
            }
            rows_cols = 0;
            // end of the row
            next = next_not_brace(next);
        }
    } while (next.size());
    return matrix(cols, rows, std::move(values));
}

list parse_list(std::string_view s)
{
    smrty::Calculator& calc = smrty::Calculator::get();
    Parser& parser = calc.get_parser();
    // strip off the braces and parse the values
    auto next_not_brace = [](std::string_view sv) -> std::string_view {
        auto next_pos = sv.find_first_not_of("{}");
        if (next_pos == std::string_view::npos)
        {
            return {};
        }
        return sv.substr(next_pos);
    };
    lg::debug("s is '{}'\n", s);
    std::string_view next = next_not_brace(s);
    std::vector<list::element_type> values{};
    do
    {
        lg::debug("next is '{}'\n", next);
        const auto& [parse_ok, t, more] = parser.parse_next(next);
        if (parse_ok)
        {
            lg::debug("parsed: {}\n", *t);
            if (t->type == Parser::mpz_type)
            {
                values.emplace_back(mpz{t->value});
            }
            else if (t->type == Parser::mpf_type)
            {
                values.emplace_back(parse_mpf(t->value));
            }
            else if (t->type == Parser::mpc_type)
            {
                values.emplace_back(parse_mpc(t->value));
            }
            else if (t->type == Parser::mpq_type)
            {
                values.emplace_back(mpq{t->value});
            }
            else
            {
                throw std::invalid_argument(
                    std::format("unsupported list entry type {}",
                                Parser::expr_types[t->type]));
            }
        }
        next = more;
        if (!parse_ok)
        {
            // end of the list
            next = next_not_brace(next);
        }
    } while (next.size());
    return list(std::move(values));
}

std::optional<time_> parse_time(std::string_view s)
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
    static std::regex iso_8601("([\\d]{4})-"  // year
                               "([\\d]{2})-"  // month
                               "([\\d]{2})"   // day
                               "(?:"          // non-capturing time group
                               "T([\\d]{2}):" // hour
                               "([\\d]{2}):"  // minute
                               "([\\d]{2})"   // second
                               "([.][\\d]+)?" // optional sub-seconds
                               ")?"           // end of non-capturing time group
                               ,
                               std::regex::ECMAScript);
    static std::regex dmmyyyy("([\\d]+)-?"   // day
                              "("            // month group
                              "jan|feb|mar|" // months
                              "apr|may|jun|" // months
                              "jul|aug|sep|" // months
                              "oct|nov|dec"  // months
                              ")-?"          // end month group
                              "([\\d]{4})"   // year
                              "(?:"          // non-capturing time group
                              "T([\\d]{2}):" // hour
                              "([\\d]{2}):"  // minute
                              "([\\d]{2})"   // second
                              "([.][\\d]+)?" // optional sub-seconds
                              ")?"           // end of non-capturing time group
                              ,
                              std::regex::ECMAScript);
    std::cmatch parts;
    if (std::regex_match(s.begin(), s.end(), parts, time_literal))
    {
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
    else if (std::regex_match(s.begin(), s.end(), parts, iso_8601))
    {
        // [0] entire date
        // [1] year
        // [2] month
        // [3] day
        // [4] hour
        // [5] minute
        // [6] second
        // [7] sub-second
        std::tm tm{};
        std::stringstream strs;
        strs << s;
        time_ subsecond{};
        if (parts[4].matched)
        {
            strs >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            if (parts[7].matched)
            {
                if (std::optional<time_> s = parse_time(parts[7].str() + "s");
                    s)
                {
                    subsecond = *s;
                }
            }
        }
        else
        {
            strs >> std::get_time(&tm, "%Y-%m-%d");
        }
        if (strs.fail())
        {
            throw std::invalid_argument("Failed to parse ISO 8601 date");
        }
        lg::debug("tm{{ sec:{}, min:{}, hour:{}, mday:{}, mon:{}, "
                  "year:{}, wday:{}, yday:{}, dst:{} }}\n",
                  tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon,
                  tm.tm_year, tm.tm_wday, tm.tm_yday, tm.tm_isdst);
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        time_ t(tp);
        return t + subsecond;
    }
    // FIXME: when regex gets its shit together and supports string_view,
    // remove the temporary std::string below
    else if (std::regex_match(s.begin(), s.end(), parts, dmmyyyy))
    {
        std::tm tm{};
        std::stringstream ss;
        ss << parts[1].str() << '-' << parts[2].str() << '-' << parts[3].str();
        time_ subsecond{};
        if (parts[4].matched)
        {
            ss << 'T' << parts[4].str() << ':' << parts[5].str() << ':'
               << parts[6].str();
            ss >> std::get_time(&tm, "%d-%b-%YT%H:%M:%S");
            if (parts[7].matched)
            {
                if (std::optional<time_> s = parse_time(parts[7].str() + "s");
                    s)
                {
                    subsecond = *s;
                }
            }
        }
        else
        {
            ss >> std::get_time(&tm, "%d-%b-%Y");
        }
        if (ss.fail())
        {
            throw std::invalid_argument("Failed to parse dd-mmm-yyyy date");
        }
        lg::debug("tm{{ sec:{}, min:{}, hour:{}, mday:{}, mon:{}, "
                  "year:{}, wday:{}, yday:{}, dst:{} }}\n",
                  tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon,
                  tm.tm_year, tm.tm_wday, tm.tm_yday, tm.tm_isdst);
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        time_ t(tp);
        return t + subsecond;
    }
    lg::debug("not a date\n");
    return std::nullopt;
}

std::string mpz_to_bin_string(const mpz& v, std::streamsize width)
{
    // limit binary prints to 64k bits?
    std::streamsize bits = 0;
    mpz mask{1};
    while (mask < v)
    {
        bits++;
        mask <<= 1;
    }
    if (bits >= 64 * 1024)
    {
        return std::format("{}", v);
    }
    std::string out;
    out.reserve(bits + 3);
    out.push_back('0');
    out.push_back('b');
    char fill = v < zero ? '1' : '0';
    while (width > bits)
    {
        out.push_back(fill);
        width--;
    }
    while (mask && !(mask & v))
    {
        mask >>= 1;
    }
    if (mask)
    {
        for (; mask; mask >>= 1)
        {
            out.push_back(v & mask ? '1' : '0');
        }
    }
    else
    {
        out.push_back('0');
    }
    return out;
}
