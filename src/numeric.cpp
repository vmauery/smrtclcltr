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
#include <type_helpers.hpp>
#include <variant>

int default_precision = builtin_default_precision;

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

mpq parse_mpf(const smrty::single_number_parts& n)
{
    static std::regex real{
        "([,\\d]+)?" // optional whole part
        "(?:"        // start of non-capturing group
        "\\."        // period
        "("          // start of float capture
        "[0]*"       // optional leading zeros
        "(\\d*)"     // truncated floating part
        ")"          // end of float capture
        ")?"         // end of non-capturing group
    };
    std::smatch parts;
    if (!std::regex_match(n.mantissa, parts, real))
    {
        throw std::runtime_error("input failed to match float regex");
    }
    // [0] entire number
    // [1] whole
    // [2] float
    // [3]  float with leading zeros removed
    mpz sign = n.mantissa_sign;
    mpz whole = parts[1].matched ? parse_mpz(parts[1].str()) : zero;
    mpz num = parts[3].matched ? parse_mpz(parts[3].str()) : zero;
    mpz den = powul_fn(ten, parts[2].str().size());
    mpq val(sign * (whole * den + num), den);
    if (n.exponent.size())
    {
        int exp{std::stoi(n.exponent)};
        exp *= n.exponent_sign;
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

mpq parse_mpq(const smrty::two_number_parts& np)
{
    lg::debug("parse_mpq({})\n", np);
    mpq num{}, den{};
    if (np.first.base)
    {
        numeric n = parse_mpz(np.first);
        if (auto np = std::get_if<mpz>(&n); np)
        {
            num = *np;
        }
        else if (auto np = std::get_if<mpq>(&n); np)
        {
            num = *np;
        }
        else
        {
            throw std::invalid_argument("invalid numerator");
        }
    }
    else
    {
        num = parse_mpf(np.first);
    }
    if (np.second.base)
    {
        numeric d = parse_mpz(np.second);
        if (auto dp = std::get_if<mpz>(&d); dp)
        {
            den = *dp;
        }
        else if (auto dp = std::get_if<mpq>(&d); dp)
        {
            den = *dp;
        }
        else
        {
            throw std::invalid_argument("invalid denominator");
        }
    }
    else
    {
        den = parse_mpf(np.second);
    }
    return num / den;
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

std::string mutate_bin_to_hex(std::string_view s)
{
    if (s.starts_with("0b"))
    {
        s = s.substr(2);
    }
    size_t remaining = s.size();
    size_t done = 0;
    std::string hex_val{};
    hex_val.reserve(2 + (remaining + 3) / 4);
    hex_val += "0x";

    while (remaining)
    {
        size_t bits = remaining % 4;
        if (bits == 0)
        {
            bits = 4;
        }
        int nibble = 0;
        for (size_t i = 0; i < bits; i++)
        {
            nibble <<= 1;
            if (s[done + i] == '1')
            {
                nibble |= 1;
            }
        }
        constexpr char hex_to_ascii[] = "0123456789abcdef";
        hex_val += hex_to_ascii[nibble];
        remaining -= bits;
        done += bits;
    }
    return hex_val;
}

mpz parse_mpz(std::string_view s, int base)
{
    /*
    std::string nc{};
    // remove any commas
    if (s.find(",") != std::string::npos)
    {
        nc = s;
        nc.erase(std::remove(nc.begin(), nc.end(), ','), nc.end());
        s = nc;
    }
    */
    lg::debug("parse_mpz({}, {})\n", s, base);
    if (base == 2)
    {
        // do we need to turn it into hex first?
        return mpz{mutate_bin_to_hex(s)};
    }
    else if (base == 8 && !s.starts_with("0"))
    {
        return mpz{std::string("0") + std::string{s}};
    }
    else if (base == 16 && !s.starts_with("0x"))
    {
        return mpz{std::string("0x") + std::string{s}};
    }
    else if (base == 10 && s.starts_with("0d"))
    {
        return mpz{s.substr(2)};
    }

    return mpz{s};
}

numeric parse_mpz(const smrty::single_number_parts& num)
{
    mpz exponent{1};
    if (num.exponent.size())
    {
        auto exp = static_cast<int>(mpz{num.exponent});
        exponent = mpz{powul_fn(mpz{10}, static_cast<int>(exp))};
    }

    mpz value = mpz(num.mantissa_sign) * parse_mpz(num.mantissa, num.base);
    if (num.exponent_sign < 0)
    {
        return mpq{value, parse_mpz(num.exponent)};
    }
    return value * exponent;
}

#ifdef USE_BASIC_TYPES

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

#endif // USE_BASIC_TYPES

mpc parse_mpc(const smrty::two_number_parts& num)
{
    mpq real = parse_mpf(num.first);
    mpq imag = parse_mpf(num.second);

    return mpc{mpf{real}, mpf{imag}};
}

matrix parse_matrix(const smrty::compound_parts& num)
{
    size_t rows = 0;
    size_t cols = num.cols;
    size_t rows_cols = 0;
    std::vector<matrix::element_type> values{};
    for (const auto& itm : num.items)
    {
        lg::debug("next is '{}'\n", itm);
        numeric nval = make_numeric(itm);
        matrix::element_type val;
        if (!reduce(nval, val)())
        {
            throw std::runtime_error("Invalid matrix type");
        }
        values.emplace_back(std::move(val));
        rows_cols++;
        if ((rows_cols % cols) == 0)
        {
            rows++;
            lg::debug("end of row {}: cols={}, cows_cols={}\n", rows, cols,
                      rows_cols);
        }
    }
    if ((cols != rows_cols) && ((rows == 2) && ((rows_cols % cols) != 0)))
    {
        throw std::invalid_argument(
            "Failed to parse matrix: column count is inconsistent");
    }
    return matrix(cols, rows, std::move(values));
}

list parse_list(const smrty::compound_parts& num)
{
    std::vector<list::element_type> values{};
    for (const auto& itm : num.items)
    {
        lg::debug("next is '{}'\n", itm);
        numeric nval = make_numeric(itm);
        list::element_type val;
        if (!reduce(nval, val)())
        {
            throw std::runtime_error("Invalid list type");
        }
        values.emplace_back(std::move(val));
    }
    return list(std::move(values));
}

template <typename T>
std::string_view from_chars(std::string_view s, T& t, int base = 10)
{
    auto endp = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(s.data(), endp, t, base);
    if (ec != std::error_code{})
    {
        return s;
    }
    return {ptr, endp};
}

time_ parse_time(const smrty::time_parts& num)
{
    if (!num.absolute)
    {
        mpq value = parse_mpf(num.duration);
        std::string_view units = num.suffix;
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
    else
    {
        std::tm tm{};
        try
        {
            size_t pos{};
            tm.tm_year = std::stoi(num.year, &pos) - 1900;

            tm.tm_mon = std::stoi(num.month, &pos) - 1;
            tm.tm_mday = std::stoi(num.day, &pos);
            if (num.h.size())
            {
                tm.tm_hour = std::stoi(num.h, &pos);
                tm.tm_min = std::stoi(num.m, &pos);
                tm.tm_sec = std::stoi(num.s, &pos);
            }
            tm.tm_isdst = -1;
        }
        catch (const std::exception& e)
        {
            throw std::invalid_argument("Failed to parse ISO 8601 date");
        }
        time_ subsecond{};
        if (num.sub.size())
        {
            smrty::time_parts sub;
            sub.absolute = false;
            sub.duration.base = 0;
            sub.duration.mantissa = num.sub;
            sub.suffix = "s";
            subsecond = parse_time(sub);
        }
        // adjust for tz
        if (num.tz.size())
        {
            std::string_view tzs{num.tz};
            int tzh{0}, tzm{0};
            if (tzs.size() >= 2)
            {
                tzs = from_chars(tzs, tzh);
                if (tzs.size() && tzs[0] == ':')
                {
                    tzs = tzs.substr(1);
                }
                else
                {
                    throw std::invalid_argument(
                        "Failed to parse ISO 8601 timezone");
                }
            }
            if (tzs.size() == 2)
            {
                tzs = from_chars(tzs, tzm);
            }
            if (!tzs.empty())
            {
                throw std::invalid_argument(
                    "Failed to parse ISO 8601 timezone");
            }
            int tzsign = tzh < 0 ? -1 : 1;
            tzh *= tzsign;
            int tzo = tzsign * (60 * tzh + tzm);
            tm.tm_min += tzo;
        }
        auto tp = std::chrono::system_clock::from_time_t(std::mktime(&tm));
        // std::mktime may adjust tm based on invalid values in fields
        lg::debug("tm{{ sec:{}, min:{}, hour:{}, mday:{}, mon:{}, "
                  "year:{}, wday:{}, yday:{}, dst:{} }}\n",
                  tm.tm_sec, tm.tm_min, tm.tm_hour, tm.tm_mday, tm.tm_mon,
                  tm.tm_year, tm.tm_wday, tm.tm_yday, tm.tm_isdst);
        time_ t(tp);
        return t + subsecond;
    }
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

mpx make_mpx(const smrty::number_parts& num)
{
    numeric nval = make_numeric(itm);
    mpx val;
    if (!reduce(nval, val)())
    {
        return {};
    }
    return val;
}

numeric make_numeric(const smrty::number_parts& num)
{
    if (const auto n = std::get_if<smrty::single_number_parts>(&num); n)
    {
        return make_numeric(*n);
    }
    else if (const auto n = std::get_if<smrty::two_number_parts>(&num); n)
    {
        return make_numeric(*n);
    }
    return {};
}

numeric make_numeric(const smrty::single_number_parts& num)
{
    if (num.base == 0)
    {
        return parse_mpf(num);
    }
    else
    {
        return parse_mpz(num);
    }
}

numeric make_numeric(const smrty::two_number_parts& num)
{
    if (num.number_type == smrty::two_number_parts::type::cmplx)
    {
        return parse_mpc(num);
    }
    else if (num.number_type == smrty::two_number_parts::type::rtnl)
    {
        return parse_mpq(num);
    }
    return {};
}

numeric make_numeric(const smrty::time_parts& num)
{
    return parse_time(num);
}

numeric make_numeric(const smrty::compound_parts& num)
{
    if (num.cols)
    {
        return parse_matrix(num);
    }
    return parse_list(num);
}
