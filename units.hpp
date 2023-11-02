/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <boost/bimap.hpp>
#include <exception>
#include <functions/common.hpp>
#include <numeric.hpp>
#include <ostream>

namespace smrty
{
namespace units
{

class bad_conversion : public std::runtime_error
{
  public:
    bad_conversion() : std::runtime_error("bad conversion")
    {
    }
    ~bad_conversion() = default;
};
class unit_parse_error : public std::runtime_error
{
  public:
    unit_parse_error() : std::runtime_error("unit parse error")
    {
    }
    ~unit_parse_error() = default;
};

template <typename L, typename R>
boost::bimap<L, R> make_bimap(
    std::initializer_list<typename boost::bimap<L, R>::value_type> list)
{
    return boost::bimap<L, R>(list.begin(), list.end());
}

using Scale = mpq;
using Id = mpq;

const Id id_None(1, 1);
const Id id_s(2, 1);
const Id id_m(3, 1);
const Id id_kg(5, 1);
const Id id_A(7, 1);
const Id id_K(11, 1);
const Id id_mol(13, 1);
const Id id_cd(17, 1);
const Id id_rad(23, 1);
const Id id_deg(29, 1);
const Id id_grad(31, 1);
const Id id_degC(37, 1);
const Id id_degF(41, 1);
const Id id_next(43, 1);

struct unit
{
    Id id = id_None;   // some composite of primes that specifies num/denom ids
    Scale exp{1, 1};   // SI prefixes (only for base-10 scaling)
    Scale scale{1, 1}; // scaling from SI units to non-SI units

    unit() = default;

    explicit unit(const Id& id) : id(id), exp(1, 1), scale(1, 1)
    {
    }

    unit(const Id& id, const Scale& exp) : id(id), exp(exp), scale(1, 1)
    {
    }

    unit(const Id& id, const Scale& exp, const Scale& scale) :
        id(id), exp(exp), scale(scale)
    {
    }

    unit(const unit& o) : id(o.id), exp(o.exp), scale(o.scale)
    {
    }
    unit(unit&& o) : id(o.id), exp(o.exp), scale(o.scale)
    {
    }
    unit& operator=(unit const& o)
    {
        id = o.id;
        exp = o.exp;
        scale = o.scale;
        return *this;
    }
    unit& operator=(unit&& o)
    {
        id = o.id;
        exp = o.exp;
        scale = o.scale;
        return *this;
    }

    explicit unit(std::string_view u);

    // handle conversions between units
    // change this into o, and modify the accompanying
    // numeric to match a change in scale and exp
    numeric conv(unit& o, const numeric& v) const;

    // compatible operator
    bool compat(const unit& o) const
    {
        return (id == o.id);
    }

    // Other units
    unit operator*(const unit& o) const
    {
        return unit(id * o.id, exp * o.exp, scale * o.scale);
    }
    unit operator/(const unit& o) const
    {
        return unit(id / o.id, exp / o.exp, scale / o.scale);
    }
    unit operator+(const unit& o) const
    {
        if (!compat(o))
        {
            throw std::invalid_argument("units do not match");
        }
        // what about scale and exp?
        return *this;
    }
    unit operator-(const unit& o) const
    {
        if (!compat(o))
        {
            throw std::invalid_argument("units do not match");
        }
        // what about scale and exp?
        return *this;
    }

    // exp scaling
    unit operator*(const int& o) const
    {
        return unit(id, exp * mpq(o, 1), scale);
    }
    unit operator/(const int& o) const
    {
        return unit(id, exp / mpq(o, 1), scale);
    }

    // foreign unit scaling
    unit operator*(const Scale& o) const
    {
        return unit(id, exp, scale * o);
    }
    unit operator/(const Scale& o) const
    {
        return unit(id, exp, scale / o);
    }

    // comparison
    bool operator==(const unit& o) const
    {
        return id == o.id && exp == o.exp && scale == o.scale;
    }
    bool operator!=(const unit& o) const
    {
        return !(id == o.id && exp == o.exp && scale == o.scale);
    }
    std::strong_ordering operator<=>(const unit& o) const = default;
};

static inline bool are_temp_units(const unit& a, const unit& b)
{
    return ((a.id == id_degC || a.id == id_degF || a.id == id_K) &&
            (b.id == id_degC || b.id == id_degF || b.id == id_K));
}

template <typename T>
static inline numeric scale_temp_units(const T& v, const unit& ua,
                                       const unit& ub)
{
    if (ua.id == id_degC)
    {
        if (ub.id == id_degF)
        {
            return v * mpq(9, 5);
        }
        return v;
    }
    else if (ua.id == id_degF)
    {
        if (ub.id != id_degF)
        {
            return v * mpq(5, 9);
        }
        return v;
    }
    else // if (ua.id == id_K)
    {
        if (ub.id == id_degF)
        {
            return v * mpq(9, 5);
        }
        return v;
    }
    throw std::invalid_argument("not temperature units");
}

// helper function that makes unit conversions more readable
// Convert value vb from units ub to units ua
static inline numeric convert(const numeric& vb, unit& ub, unit& ua)
{
    return ua.conv(ub, vb);
}

// unit op functions
static auto remove_units = [](const auto...) { return unit(); };
static auto retain_units = [](const unit& a, const auto...) { return a; };
/*
static bool is_unitless(void)
{
    return true;
}
*/
template <typename Arg, typename... Args>
static bool is_unitless(const Arg& first, Args&&... remaining)
{
    return first == unit() && is_unitless(remaining...);
}
static auto require_unitless = [](const auto... various_units) {
    return is_unitless(various_units...);
};
static auto require_match_2 = [](const unit& a, const unit& b) {
    if (a != b)
    {
        throw std::invalid_argument("units do not match");
    }
    return a;
};
static auto require_match_3 = [](const unit& a, const unit& b, const unit& c) {
    if (a != b || a != c)
    {
        throw std::invalid_argument("units do not match");
    }
    return a;
};

unit pow(const unit& u, const mpf& p);

const boost::bimap<std::string_view, unit>& get_units_map();

} // namespace units
} // namespace smrty

template <>
struct std::formatter<smrty::units::unit>
{
    std::__format::_Spec<char> spec;

    // Parses format like the standard int formatter
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_fill_and_align(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_sign(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_zero_fill(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        // default to basic
        spec._M_type = std::__format::_Pres_b;
        switch (*begin)
        {
            case 'b': // unit basic
                spec._M_type = std::__format::_Pres_b;
                ++begin;
                break;
            case 'd': // unit debug
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    // Formats the point p using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const smrty::units::unit& u, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        bool debug = spec._M_type == std::__format::_Pres_d;
        if (u.id == smrty::units::id_None)
        {
            if (debug)
            {
                std::format_to(out, "_<>({}, {}, {})", u.id, u.exp, u.scale);
            }
            return out;
        }
        auto units_map = smrty::units::get_units_map();
        auto units_it = units_map.right.find(u);
        if (units_it != units_map.right.end())
        {
            std::format_to(out, "_{}", units_it->second);
            if (debug)
            {
                std::format_to(out, "({}, {}, {})", u.id, u.exp, u.scale);
            }
            return out;
        }
        // factor numerator and denominator of id
        std::vector<mpz> num_factors =
            smrty::function::util::prime_factor(helper::numerator(u.id));
        if (num_factors.size() == 0)
        {
            num_factors.push_back(helper::numerator(u.id));
        }
        std::vector<mpz> den_factors =
            smrty::function::util::prime_factor(helper::denominator(u.id));
        bool first = true;
        for (const auto& f : num_factors)
        {
            if (first)
            {
                *out++ = '_';
                first = false;
            }
            else
            {
                *out++ = '*';
            }
            units_it = units_map.right.find(smrty::units::unit(mpq{f, 1}));
            if (units_it != units_map.right.end())
            {
                std::format_to(out, "{}", units_it->second);
            }
            else
            {
                *out++ = '?';
            }
        }
        if (first)
        {
            *out++ = '1';
        }
        first = true;
        for (const auto& f : den_factors)
        {
            if (first)
            {
                *out++ = '/';
                first = false;
            }
            else
            {
                *out++ = '*';
            }
            units_it = units_map.right.find(smrty::units::unit(mpq{f, 1}));
            if (units_it != units_map.right.end())
            {
                std::format_to(out, "{}", units_it->second);
            }
            else
            {
                *out++ = '?';
            }
        }
        if (debug)
        {
            std::format_to(out, "({}, {}, {})", u.id, u.exp, u.scale);
        }
        return out;
    }
};
