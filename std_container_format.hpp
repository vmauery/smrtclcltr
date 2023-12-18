#pragma once

#include <array>
#include <cstdio>
#include <format>
#include <variant>

// The ifdefs below are a bit of a hack, making it so this file
// needs to be included AFTER any types to print... Ick.
template <typename T>
struct is_formattable_container : std::false_type
{
};

#if (defined(_GLIBCXX_DEQUE) || defined(_LIBCPP_DEQUE))
template <typename T, typename... A>
struct is_formattable_container<std::deque<T, A...>> : std::true_type
{
};
#endif

#if (defined(_GLIBCXX_VECTOR) || defined(_LIBCPP_VECTOR))
template <typename T, typename... A>
struct is_formattable_container<std::vector<T, A...>> : std::true_type
{
};
#endif

#if (defined(_GLIBCXX_LIST) || defined(_LIBCPP_LIST))
template <typename T, typename... A>
struct is_formattable_container<std::list<T, A...>> : std::true_type
{
};
#endif

#if (defined(_GLIBCXX_UNORDERED_SET) || defined(_LIBCPP_UNORDERED_SET))
template <typename T, typename... A>
struct is_formattable_container<std::unordered_set<T, A...>> : std::true_type
{
};
#endif

#if (defined(_GLIBCXX_SET) || defined(_LIBCPP_SET))
template <typename T, typename... A>
struct is_formattable_container<std::set<T, A...>> : std::true_type
{
};
#endif

#if (defined(_GLIBCXX_SPAN) || defined(_LIBCPP_SPAN))
template <typename T, size_t N>
struct is_formattable_container<std::span<T, N>> : std::true_type
{
};
#endif

template <typename T, size_t N>
struct is_formattable_container<std::array<T, N>> : std::true_type
{
};

template <typename K>
concept formattable_container = is_formattable_container<K>::value;

constexpr std::string_view default_format{"{}"};
template <typename Iter>
constexpr Iter parse_sub_format(Iter begin, Iter end, std::string_view& fmt)
{
    int depth = 0;
    if (*begin == '{')
    {
        auto formend = begin;
        while (formend != end && *formend)
        {
            if (*formend == '{')
            {
                depth++;
            }
            else if (*formend == '}')
            {
                depth--;
            }
            if (depth == 0)
            {
                break;
            }
            formend++;
        }
        if (depth != 0 || formend == end || *formend != '}')
        {
            return end;
        }
        fmt = {begin, formend + 1};
        begin = formend + 1;
    }
    else
    {
        fmt = default_format;
    }
    return begin;
}

template <typename Cntnr>
    requires formattable_container<Cntnr>
struct std::formatter<Cntnr>
{
    std::string_view delimiter{};
    std::string_view item_format{"{}"};
    // format: {[name][:][{item-format}][separator]}
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        auto begin = ctx.begin(), end = ctx.end();
        std::string_view f{begin, end};
        if (begin == end)
        {
            return begin;
        }
        // check for optional per-item format string
        // outside references not available at this time
        begin = parse_sub_format(begin, end, item_format);
        if (begin == end)
        {
            return begin;
        }
        auto delend = begin;
        while (delend != end && *delend && *delend != '}')
        {
            delend++;
        }
        delimiter = {begin, delend};
        return delend;
    }

    template <typename FormatContext>
    auto format(const Cntnr& m, FormatContext& ctx) const -> decltype(ctx.out())
    {
        auto out = ctx.out();
        auto mi = m.begin();
        for (size_t i = 0; i < m.size(); i++)
        {
            if (i != 0)
            {
                out = std::copy(delimiter.begin(), delimiter.end(), out);
            }
            out =
                std::vformat_to(out, item_format, std::make_format_args(*mi++));
        }
        return out;
    }
};

template <typename... Types>
struct std::formatter<std::variant<Types...>>
{
    std::array<std::string_view, std::variant_size_v<std::variant<Types...>>>
        sub_formats;
    static constexpr std::string_view default_format{"{}"};
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // format: {[name]:[{type0-format}[{type1-format}...]]}
        //   e.g.: for std::variant<mpz,mpq,mpf,mpc>:
        //         {:{:> 3i}{:> 3.3f}{:> 3.3f}{:> 1.2i}}
        auto begin = ctx.begin();
        auto end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        for (size_t i = 0; i < sub_formats.size(); i++)
        {
            auto v = parse_sub_format(begin, end, sub_formats[i]);
            if (v == end)
            {
                break;
            }
            begin = v;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const std::variant<Types...>& t,
                FormatContext& ctx) const -> decltype(ctx.out())
    {
        const auto& fmtstr = sub_formats[t.index()];
        lg::debug("fmtstr[{}] = {}\n", t.index(), fmtstr);
        return std::visit(
            [&fmtstr, &ctx](const auto& v) {
                return std::vformat_to(ctx.out(), fmtstr,
                                       std::make_format_args(v));
            },
            t);
    }
};
