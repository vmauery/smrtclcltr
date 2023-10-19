/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <charconv>
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
    std::smatch parts;
    // FIXME: when regex gets its shit together and supports string_view, remove
    //        the temporary std::string below
    std::string ss{s};
    if (!std::regex_match(ss, parts, real))
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
            val /= to_mpq(smrty::function::util::pow(mpz(10), -exp));
        }
        else
        {
            val *= to_mpq(smrty::function::util::pow(mpz(10), exp));
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
    lg::debug("Q: {}, error = {}\n", result, error);

    /* now try other possibility */
    ai = (maxden - m[1][1]) / m[1][0];
    m[0][0] = m[0][0] * ai + m[0][1];
    m[1][0] = m[1][0] * ai + m[1][1];
    mpq result2(m[0][0], m[1][0]);
    mpf error2 = abs(f - to_mpf(result2));
    lg::debug("Q: {}, error2 = {}\n", result2, error2);
    set_default_precision(orig_prec);
    if (error > max_error && error2 > max_error)
    {
        lg::info("lossy mpf->mpq; not into it.\n");
        throw std::invalid_argument("Unable to convert mpf to mpq");
    }
    if (error <= error2)
    {
        return result;
    }
    return result2;
}

mpq parse_mpf(std::string_view s)
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
        ret *= mpz(pow_fn(mpf(10), exp));
        return ret;
    }
    return mpz(s);
}
#else  // USE_BASIC_TYPES
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
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), ret, base);
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
            auto [ptr, ec] = std::from_chars(exps.begin(), exps.end(), exp);
            if (ptr != exps.end())
            {
                throw std::invalid_argument("input has an invalid exponent");
            }
            exp = pow_fn(10.0l, static_cast<mpf>(exp));
            mpf retf = ret * exp;
            if (exp == HUGE_VALL ||
                ((retf) > mpf(std::numeric_limits<mpz>::max())))
            {
                throw std::overflow_error("overflow with exponent");
            }
            ret = mpz(retf);
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
               "?:[eE][+-]?\\d+)?)?)?[ij]$",
               std::regex::ECMAScript),
    std::regex("^[(]([-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?),(["
               "-]?(?:\\d+(?:\\.\\d*)?|\\.\\d+)(?:[eE][+-]?\\d+)?)[)]$",
               std::regex::ECMAScript),
};
mpc parse_mpc(std::string_view s)
{
    mpf complex_parts[2]{};
    for (int j = 0; j < 2; j++)
    {
        std::smatch parts;
        // FIXME: when regex gets its shit together and supports string_view,
        // remove the temporary std::string below
        std::string ss{s};
        if (std::regex_search(ss, parts, cmplx_regex[j]))
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

std::string time_::str() const
{
    if (absolute)
    {
        long long nanos = static_cast<long long>(
            (helper::numerator(value) * mpz(1'000'000'000ll)) /
            helper::denominator(value));
        // lg::debug("value={}, nanos={}\n", value, nanos);
        std::chrono::duration d = std::chrono::nanoseconds(nanos);
        std::chrono::time_point<std::chrono::system_clock> tp(d);
        return std::format("{:%F %T}", tp);
        // const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
        // return std::strftime(std::localtime(&t_c), "%F %T");
    }

    // duration
    static const mpq one_day{86400, 1};
    static const mpq one_hour{3600, 1};
    static const mpq one_minute{60, 1};
    static const mpq one_second{1, 1};
    static const mpq one_ms{1, 1000};
    static const mpq one_us{1, 1000000};
    static const mpq one_ns{1, 1000000000};

    auto pval = abs(value);
    if (pval >= one_day)
    {
        return std::format("{:f}d", value / one_day);
    }
    if (pval >= one_hour)
    {
        return std::format("{:f}h", value / one_hour);
    }
    if (pval >= one_minute)
    {
        return std::format("{:f}m", value / one_minute);
    }
    if (pval >= one_second)
    {
        return std::format("{:f}s", value / one_second);
    }
    if (pval >= one_ms)
    {
        return std::format("{:f}ms", value / one_ms);
    }
    if (pval >= one_us)
    {
        return std::format("{:f}us", value / one_us);
    }
    return std::format("{:f}ns", value / one_ns);
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
    std::smatch parts;
    // FIXME: when regex gets its shit together and supports string_view,
    // remove the temporary std::string below
    std::string ss{s};
    if (std::regex_match(ss, parts, time_literal))
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
    // FIXME: when regex gets its shit together and supports string_view,
    // remove the temporary std::string below
    else if (std::regex_match(ss, parts, iso_8601))
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
        std::stringstream strs(ss);
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
    else if (std::regex_match(ss, parts, dmmyyyy))
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

numeric reduce_numeric(const numeric& n, int precision)
{
    if (precision == 0)
    {
        precision = default_precision;
    }
    std::visit(
        [](const auto& v) {
            lg::debug("reduce({} (type {}))\n", v, DEBUG_TYPE(v));
        },
        n);
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
    char fill = v < 0 ? '1' : '0';
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
