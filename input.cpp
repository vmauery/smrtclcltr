/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <input.hpp>

namespace input
{
char* generator_linkage(
    const char* text, int state,
    std::optional<
        std::function<std::optional<std::string_view>(std::string_view, int)>>
        set_generator)
{
    static std::function<std::optional<std::string_view>(const std::string_view,
                                                         int)>
        _input_generator;
    if (set_generator)
    {
        _input_generator = *set_generator;
        return nullptr;
    }
    if (!_input_generator)
    {
        return nullptr;
    }
    std::optional<std::string_view> next = _input_generator(text, state);
    if (next)
    {
        // need to strdup the string_view, but don't want to copy to string
        size_t sz = (*next).size();
        auto copy = reinterpret_cast<char*>(malloc(sz + 1));
        if (copy != nullptr)
        {
            (*next).copy(copy, sz);
            copy[sz] = 0;
            return copy;
        }
    }
    return nullptr;
}

extern "C" {

char* c_input_generator(const char* text, int state)
{
    return input::generator_linkage(text, state, std::nullopt);
}

char** c_input_completion(const char* text, int, int)
{
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, c_input_generator);
}

} // extern "C"
} // namespace input

std::optional<std::string> Input::readline()
{
    if (interactive)
    {
        // for interactive, use readline
#if HAVE_READLINE == 1
        std::shared_ptr<char> buf =
            std::shared_ptr<char>(::readline("> "), free);
        std::string nextline;
        if (buf)
        {
            nextline = buf.get();
            if (nextline.size())
            {
                add_history(buf.get());
                return nextline;
            }
            return "";
        }
#else  // ! HAVE_READLINE
        ui::get()->out("> ")->flush();
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            if (nextline.size())
            {
                return nextline;
            }
            return "";
        }
#endif // HAVE_READLINE
        return std::nullopt;
    }
    else
    {
        // for non-interactive, std::getline() works fine
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            if (nextline.size())
            {
                return nextline;
            }
            return "";
        }
        return std::nullopt;
    }
}
