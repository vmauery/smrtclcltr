#pragma once

#include <cstdio>
#include <format>
#include <span>
#include <variant>
#include <vector>

namespace std
{

static inline void write(std::FILE* f, std::string_view str)
{
    fwrite(str.data(), 1, str.size(), f);
}

static inline void vprint(std::FILE* f, std::string_view format,
                          std::format_args args)
{
    std::string str = std::vformat(format, args);
    write(f, str);
}

template <class... Args>
static inline void print(std::FILE* f, std::format_string<Args...> format,
                         Args&&... args)
{
    vprint(f, format.get(), std::make_format_args(std::forward<Args>(args)...));
}

template <class... Args>
static inline void print(std::format_string<Args...> format, Args&&... args)
{
    vprint(stdout, format.get(),
           std::make_format_args(std::forward<Args>(args)...));
}

// degenerate cases (no formatting required)
static inline void print(const std::string& str)
{
    write(stdout, str);
}

static inline void print(std::FILE* f, const std::string& str)
{
    write(f, str);
}

static inline void print(std::string_view str)
{
    write(stdout, str);
}

static inline void print(const char* str)
{
    write(stdout, str);
}

static inline void print(std::FILE* f, std::string_view str)
{
    write(f, str);
}

} // namespace std

template <typename T>
struct std::formatter<std::span<T>>
{
    std::string_view delimiter{};
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        auto begin = ctx.begin(), end = ctx.end();
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
    auto format(const std::span<T>& m, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        auto mi = m.begin();
        for (size_t i = 0; i < m.size(); i++)
        {
            if (i != 0)
            {
                out = std::copy(delimiter.begin(), delimiter.end(), out);
            }
            std::format_to(out, "{}", *mi++);
        }
        return out;
    }
};

template <typename T>
struct std::formatter<std::vector<T>>
{
    std::string_view delimiter{};
    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        auto begin = ctx.begin(), end = ctx.end();
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
    auto format(const std::vector<T>& m, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        auto mi = m.begin();
        for (size_t i = 0; i < m.size(); i++)
        {
            if (i != 0)
            {
                out = std::copy(delimiter.begin(), delimiter.end(), out);
            }
            std::format_to(out, "{}", *mi++);
        }
        return out;
    }
};

template <typename... Types>
struct std::formatter<std::variant<Types...>>
{
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // nothing to parse
        auto begin = ctx.begin();
        return begin;
    }

    template <typename FormatContext>
    auto format(const std::variant<Types...>& t, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        // ctx.out() is an output iterator to write to.
        return std::visit(
            [&ctx](const auto& v) {
                return std::format_to(ctx.out(), "{}", v);
            },
            t);
    }
};
