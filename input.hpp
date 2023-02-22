/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#if HAVE_READLINE == 1
#include <readline/history.h>
#include <readline/readline.h>
#endif // HAVE_READLINE

#include <functional>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace input
{
char* generator_linkage(
    const char* text, int state,
    std::optional<
        std::function<std::optional<std::string_view>(std::string_view, int)>>
        set_generator);
extern "C" char** c_input_completion(const char* text, int, int);
} // namespace input

class Input
{
  protected:
    struct Private
    {
    };

    Input()
    {
    }

    template <typename T>
    Input(bool interactive, T&& auto_complete_cb) :
        interactive(interactive),
        auto_complete(std::forward<T>(auto_complete_cb))
    {
        if (auto_complete)
        {
            rl_attempted_completion_function = input::c_input_completion;
            input::generator_linkage(nullptr, 0, auto_complete);
        }
    }

  public:
    template <typename T>
    Input(bool interactive, T&& auto_complete, const Private&) :
        Input(interactive, std::forward<T>(auto_complete))
    {
    }
    template <typename T>
    static std::shared_ptr<Input> make_shared(bool interactive,
                                              T&& auto_complete)
    {
        return std::make_shared<Input>(
            interactive, std::forward<T>(auto_complete), Private{});
    }

    std::optional<std::string> readline();

  protected:
    bool interactive;
    std::function<std::optional<std::string_view>(std::string_view, int)>
        auto_complete;
};
