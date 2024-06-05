#pragma once

#include <cstdio>
#include <format>

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
    vprint(f, format.get(), std::make_format_args(args...));
}

template <class... Args>
static inline void print(std::format_string<Args...> format, Args&&... args)
{
    vprint(stdout, format.get(), std::make_format_args(args...));
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
