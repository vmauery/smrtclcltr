/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <stdio.h>
#include <sys/ioctl.h>

#include <memory>
#include <print>
#include <span>
#include <string_view>
#include <vector>

struct ui : public std::enable_shared_from_this<ui>
{
  private:
    struct Private
    {
    };

    ui()
    {
    }

    ui(ui&&) = delete;
    ui(const ui&) = delete;

  public:
    ui(const Private&) : ui()
    {
    }

    static std::shared_ptr<ui> get()
    {
        static std::shared_ptr<ui> self{};
        if (!self)
        {
            self = std::make_shared<ui>(Private{});
        }
        return self;
    }

    std::shared_ptr<ui> flush()
    {
        // stderr is non-buffering, so just flush stdout
        fflush(stdout);
        return shared_from_this();
    }

    std::pair<int, int> size()
    {
        struct winsize w;
        if (ioctl(0, TIOCGWINSZ, &w) < 0)
        {
            return {25, 80};
        }
        return {w.ws_row, w.ws_col};
    }

    std::shared_ptr<ui> err(const std::string& m)
    {
        fputs(m.c_str(), stderr);
        return shared_from_this();
    }

    std::shared_ptr<ui> out(const std::string& m)
    {
        fputs(m.c_str(), stdout);
        return shared_from_this();
    }

    template <typename... Args>
    std::shared_ptr<ui> out(std::string_view f, Args... args)
    {
        std::string msg = std::vformat(f, std::make_format_args(args...));
        return out(msg);
    }
};

struct column_layout
{
    explicit column_layout(size_t n) : cols(n), len(0), valid(true)
    {
    }
    std::vector<size_t> cols;
    size_t len;
    bool valid;
};

column_layout find_best_layout(std::span<std::string_view> words, size_t width);
