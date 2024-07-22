/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <cxxabi.h>

#include <format>
#include <iomanip>
#include <iostream>
#include <regex>
#include <source_location>
#include <string>
#include <string_view>
#include <type_helpers.hpp>
#include <type_traits>
#include <typeinfo>
#include <ui.hpp>
#include <variant>

namespace lg
{
using source_location = std::source_location;

/** Matches a type T which is anything except one of those in Ss. */
template <typename T, typename... Ss>
concept any_but = (... && !std::convertible_to<T, Ss>);

enum class level
{
    emergency = 0,
    alert,
    critical,
    error,
    warning,
    notice,
    info,
    debug,
    verbose,
    trace,
};
extern level debug_level;

static inline size_t get_file_offset()
{
    auto here = std::source_location::current();
    std::string_view debug_header = here.file_name();
    constexpr size_t debug_hpp_len = std::string_view{"debug.hpp"}.size();
    return debug_header.size() - debug_hpp_len;
}

static inline void _log(const source_location& sl, std::string_view message)
{
    static const size_t file_offset = get_file_offset();
    std::string_view file{sl.file_name()};
    std::string msg = std::format("{:s}:{:d}: {:s}", file.substr(file_offset),
                                  sl.line(), message);
    ui::get()->err(msg);
}

template <level L = level::debug, any_but<source_location>... Args>
struct log
{
    explicit log(const source_location& loc, std::string_view f, Args... args)
    {
        if (L <= debug_level)
        {
            std::string msg = std::vformat(f, std::make_format_args(args...));
            _log(loc, msg);
        }
    }
    explicit log(std::string_view f, Args... args,
                 const source_location& loc = source_location::current()) :
        log(loc, f, args...)
    {
    }
    log() = delete;
};

// Easy access for lg::level(...) -> lg::log<level>(...)
#define MK_LOG_LVL(lvl)                                                        \
    template <typename... Ts>                                                  \
    struct lvl : public log<level::lvl, Ts...>                                 \
    {                                                                          \
        using log<level::lvl, Ts...>::log;                                     \
    };                                                                         \
                                                                               \
    template <typename... Ts>                                                  \
    explicit lvl(const char*, Ts&&...) -> lvl<Ts...>;                          \
                                                                               \
    template <typename... Ts>                                                  \
    explicit lvl(const lg::source_location&, const char*, Ts&&...)->lvl<Ts...>

// Enumerate the aliases for each log level.
MK_LOG_LVL(emergency);
MK_LOG_LVL(alert);
MK_LOG_LVL(critical);
MK_LOG_LVL(error);
MK_LOG_LVL(warning);
MK_LOG_LVL(notice);
MK_LOG_LVL(info);
MK_LOG_LVL(debug);
MK_LOG_LVL(verbose);
MK_LOG_LVL(trace);

#undef MK_LOG_LVL

} // namespace lg

template <typename T>
struct debug_type
{
    std::string name{};
    debug_type(const T&)
    {
        char* dname = 0;
        int status;
        dname = abi::__cxa_demangle(typeid(T).name(), 0, 0, &status);
        if (dname != nullptr)
        {
            name = dname;
        }
        else
        {
            name = typeid(T).name();
        }
        free(dname);
    }
};

static inline std::string DEBUG_TYPE(const auto& x)
{
    debug_type t(x);
    std::string& name = t.name;
    if constexpr (is_variant_v<typename std::decay_t<decltype(x)>>)
    {
        std::string selected = std::visit(
            [](const auto& x) {
                debug_type t(x);
                return t.name;
            },
            x);
        name = std::regex_replace(name, std::regex{selected}, "[$&]");
        return name;
    }
    else
    {
        return name;
    }
}
