/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <string_view>
#include <ui.hpp>

#if 0 // __has_builtin(__builtin_source_location)
#include <source_location>

namespace lg
{
using source_location = std::source_location;
}

#else
#include <experimental/source_location>

namespace lg
{
using source_location = std::experimental::source_location;
}

#endif

namespace lg
{
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
};
extern level debug_level;

static inline void _log(level lvl, const source_location& sl,
                        const std::string& message)
{
    if (lvl <= debug_level)
    {
        std::string msg =
            fmt::format("{:s}:{:d}: {:s}", sl.file_name(), sl.line(), message);
        ui::get()->err(msg);
    }
}

template <level L = level::debug, any_but<source_location>... Args>
struct log
{
    explicit log(const source_location& loc, std::string_view f, Args... args)
    {
        std::string msg = fmt::vformat(f, fmt::make_format_args(args...));
        _log(L, loc, msg);
    }
    explicit log(std::string_view f, Args... args,
                 const source_location& loc = source_location::current()) :
        log(loc, f, args...)
    {
    }
    log() = delete;
};

// deduction guides
template <level L = level::debug, typename... Args>
explicit log(const char*, Args&&...) -> log<L, Args...>;

template <level L = level::debug, typename... Args>
explicit log(const source_location&, const char*, Args&&...) -> log<L, Args...>;

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

#undef MK_LOG_LVL

} // namespace lg