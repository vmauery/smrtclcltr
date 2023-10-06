/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <iostream>
#include <memory>
#include <string_view>

struct ui : public std::enable_shared_from_this<ui>
{
  private:
    struct Private
    {
    };

    ui()
    {
    }

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
        // cerr is non-buffering, so just flush cout
        std::cout.flush();
        return shared_from_this();
    }

    std::shared_ptr<ui> err(const std::string& m)
    {
        std::cerr << m;
        return shared_from_this();
    }

    std::shared_ptr<ui> out(const std::string& m)
    {
        std::cout << m;
        return shared_from_this();
    }

    template <typename... Args>
    std::shared_ptr<ui> out(std::string_view f, Args... args)
    {
        std::string msg = std::vformat(f, std::make_format_args(args...));
        return out(msg);
    }
};
