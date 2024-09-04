/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once
#if NEED_NUMERIC_TYPE_FORMATTERS != 1
#error "Do not include this file directly"
#endif

#include <std_container_format.hpp>

namespace std
{

// list and matrix do not format at compile time, so calls to std::format
// will fail. This offers a runtime format mechanism until the c++26
// std::runtime_format() mechanism will force std::format to parse and
// format at runtime.
template <class... Args>
static inline std::string format_runtime(std::string_view f, Args&&... args)
{
    return vformat(f, std::make_format_args(args...));
}

} // namespace std

// This assumes that mpz, mpf, mpc, mpq are already fully defined
// These formatters are common for both boost types and basic types

template <>
struct std::formatter<mpz>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard int parser
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
        begin = spec._M_parse_alternate_form(begin, end);
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
        switch (*begin)
        {
            case 'b':
                spec._M_type = std::__format::_Pres_b;
                ++begin;
                break;
            case 'd':
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            case 'o':
                spec._M_type = std::__format::_Pres_o;
                ++begin;
                break;
            case 'x':
                spec._M_type = std::__format::_Pres_x;
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

    template <typename FormatContext>
    auto format(const mpz& z, FormatContext& ctx) const -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        std::streamsize width = spec._M_get_width(ctx);
        std::string s{};
        if (spec._M_type == std::__format::_Pres_b)
        {
            // manually make the binary version; mpz does not do it
            s = mpz_to_bin_string(z, width);
        }
        else
        {
            // hex/dec/oct are all handled by mpz
            std::ios_base::fmtflags f{};
            if (spec._M_alt)
            {
                f |= std::ios::showbase;
            }
            if (spec._M_type == std::__format::_Pres_o)
            {
                f |= std::ios::oct;
            }
            else if (spec._M_type == std::__format::_Pres_x)
            {
                f |= std::ios::hex;
            }
            else // if (spec._M_type == std::__format::_Pres_d)
            {
                f |= std::ios::dec;
            }
            s = z.str(width, f);
        }
        auto out = ctx.out();
        if (z >= zero && spec._M_sign == std::__format::_Sign_plus)
        {
            *out++ = '+';
        }
        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct std::formatter<mpf>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard float parser
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
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'f':
                spec._M_type = std::__format::_Pres_f;
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

    template <typename FormatContext>
    auto format(const mpf& f, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1.0)
        {
            precision = default_precision;
        }
        auto out = ctx.out();
        if (f >= 0.0 && spec._M_sign == std::__format::_Sign_plus)
        {
            *out++ = '+';
        }
        auto s = f.str(precision);
        return std::copy(s.begin(), s.end(), out);
    }
};

template <>
struct std::formatter<mpq>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard float parser
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
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'q': // quotient
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            case 'f': // fraction
                spec._M_type = std::__format::_Pres_f;
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

    template <typename FormatContext>
    auto format(const mpq& q, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1)
        {
            precision = default_precision;
        }
        if (spec._M_type == std::__format::_Pres_f)
        {
            mpf f = static_cast<mpf>(helper::numerator(q)) /
                    static_cast<mpf>(helper::denominator(q));
#ifdef USE_BASIC_TYPES
            return std::format_to(ctx.out(), "{0:{1}f}", f, precision);
#else
            auto s = f.str(precision);
            return std::copy(s.begin(), s.end(), ctx.out());
#endif
        }
        else // if (spec._M_type == std::__format::_Pres_d)
        {
            // q form is chosen by 'q' presentation type
            mpz num = helper::numerator(q);
            mpz den = helper::denominator(q);
            if (den == 1)
            {
                return std::format_to(ctx.out(), "{}", num);
            }
            return std::format_to(ctx.out(), "{}/{}", num, den);
        }
    }
};

template <>
struct std::formatter<mpc>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard int parser
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
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        switch (*begin)
        {
            case 'i': // i or j notation
                spec._M_type = std::__format::_Pres_g;
                ++begin;
                break;
            case 'p': // polar
                spec._M_type = std::__format::_Pres_p;
                ++begin;
                break;
            case 'r': // rectangular
                spec._M_type = std::__format::_Pres_f;
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

    template <typename FormatContext>
    auto format(const mpc& c, FormatContext& ctx) const -> decltype(ctx.out())
    {
        ssize_t precision = spec._M_get_precision(ctx);
        if (precision < 1)
        {
            precision = default_precision;
        }
        // ctx.out() is an output iterator to write to.
        if (spec._M_type == std::__format::_Pres_f)
        {
            return std::format_to(ctx.out(), "({0:.{2}},{1:.{2}})", c.real(),
                                  c.imag(), precision);
        }
        else if (spec._M_type == std::__format::_Pres_p)
        {
            return std::format_to(ctx.out(), "({0:.{2}},<{1:.{2}})", abs(c),
                                  atan2(c.real(), c.imag()), precision);
        }
        else // if (spec._M_type == std::__format::_Pres_g)
        {
            return std::format_to(ctx.out(), "{0:.{2}}{1:+.{2}}i", c.real(),
                                  c.imag(), precision);
        }
    }
};

template <typename T>
struct std::formatter<basic_matrix<T>>
{
    std::__format::_Spec<char> spec{};
    static constexpr std::string_view default_format{"{}"};
    std::string vfmt{default_format};

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // format:
        // {[name]:[[#][padding][.width][{type0-format}[{type1-format}...]]]}
        //   e.g.: for std::variant<mpz,mpq,mpf,mpc>:
        //         {:#80{:> 3i}{:> 3.3f}{:> 3.3f}{:> 1.2i}}
        //     or: {[name]:[[#][.width][{type-format}]]}
        //   e.g.: for std::variant<mpz,mpq,mpf,mpc>:
        //         {:#80{:> 3i}{:> 3.3f}{:> 3.3f}{:> 1.2i}}
        auto begin = ctx.begin();
        auto end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        // alternate form for a matrix is one-line
        begin = spec._M_parse_alternate_form(begin, end);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_precision(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        auto fmts_begin = begin;
        if constexpr (is_variant_v<T>)
        {
            std::array<std::string_view, std::variant_size_v<T>> sub_formats;
            for (size_t i = 0; i < sub_formats.size(); i++)
            {
                auto v = parse_sub_format(begin, end, sub_formats[i]);
                if (v == end)
                {
                    break;
                }
                begin = v;
            }
        }
        else
        {
            std::string_view sub_format{};
            begin = parse_sub_format(begin, end, sub_format);
        }
        if (fmts_begin != begin)
        {
            vfmt = std::format("{{:{}}}", std::string_view{fmts_begin, begin});
        }
        else
        {
            vfmt = default_format;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const basic_matrix<T>& m,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        int pad = 1 + spec._M_get_width(ctx);
        [[maybe_unused]] int width = spec._M_get_precision(ctx);
        *out++ = '[';
        auto iter = m.values.begin();
        lg::debug("matrix::vfmt = '{}'\n", vfmt);
        // FIXME: use column width code?
        for (size_t r = 0; r < m.rows; r++)
        {
            if (r != 0)
            {
                // oneline
                if (spec._M_alt)
                {
                    *out++ = ' ';
                }
                else
                {
                    *out++ = '\n';
                    for (int i = 0; i < pad; i++)
                    {
                        *out++ = ' ';
                    }
                }
            }
            *out++ = '[';
            for (size_t c = 0; c < m.cols; c++)
            {
                out = std::vformat_to(out, vfmt, std::make_format_args(*iter));
                iter++;
                if ((c + 1) < m.cols)
                {
                    *out++ = ' ';
                }
            }
            *out++ = ']';
        }
        *out++ = ']';
        return out;
    }
};

template <typename T>
struct std::formatter<basic_list<T>>
{
    std::__format::_Spec<char> spec{};
    static constexpr std::string_view default_format{"{}"};
    std::string vfmt{default_format};

    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // format:
        // {[name]:[[{type0-format}[{type1-format}...]]]}
        //   e.g.: for std::variant<mpz,mpq,mpf,mpc>:
        //         {:{:> 3i}{:> 3.3f}{:> 3.3f}{:> 1.2i}}
        auto begin = ctx.begin();
        auto end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        auto fmts_begin = begin;
        if constexpr (is_variant_v<T>)
        {
            std::array<std::string_view, std::variant_size_v<T>> sub_formats;
            for (size_t i = 0; i < sub_formats.size(); i++)
            {
                auto v = parse_sub_format(begin, end, sub_formats[i]);
                if (v == end)
                {
                    break;
                }
                begin = v;
            }
        }
        else
        {
            std::string_view sub_format{};
            begin = parse_sub_format(begin, end, sub_format);
        }
        if (fmts_begin != begin)
        {
            vfmt = std::format("{{:{}}}", std::string_view{fmts_begin, begin});
        }
        else
        {
            vfmt = default_format;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const basic_list<T>& lst,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        [[maybe_unused]] int width = spec._M_get_precision(ctx);
        *out++ = '{';
        size_t remaining = lst.size() - 1;
        for (const auto& v : lst.values)
        {
            out = std::vformat_to(out, vfmt, std::make_format_args(v));
            if (remaining--)
            {
                *out++ = ' ';
            }
        }
        *out++ = '}';
        return out;
    }
};
