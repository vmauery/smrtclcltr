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
        std::function<std::optional<std::string>(const std::string&, int)>>
        set_generator)
{
    static std::function<std::optional<std::string>(const std::string&, int)>
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
    std::optional<std::string> next = _input_generator(text, state);
    if (next)
    {
        return strdup(next->c_str());
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
            return "\n";
        }
#else  // ! HAVE_READLINE
        std::cout << "> ";
        std::cout.flush();
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            if (nextline.size())
            {
                return nextline;
            }
            return "\n";
        }
#endif // HAVE_READLINE
        return std::nullopt;
    }
    else
    {
        // for non-interactive, std::getline() works fine
        // std::cerr << "<waiting for input>\n";
        if (std::string nextline; std::getline(std::cin, nextline))
        {
            if (nextline.size())
            {
                return nextline;
            }
            return "\n";
        }
        return std::nullopt;
    }
}
