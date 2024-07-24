/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "debug.hpp"
#include "main.hpp"

#include <calculator.hpp>
#include <string>
#include <vector>

int usage(std::span<std::string_view> args)
{
    std::print(stderr, "Usage: {} [-v [n]]\n", args[0]);
    return 1;
}

int cpp_main(std::span<std::string_view> args)
{
    size_t i = 1;
    for (; i < args.size(); i++)
    {
        std::string_view arg = args[i];
        if (arg[0] != '-')
        {
            break;
        }
        if (arg == "-v")
        {
            if ((i + 1) < args.size())
            {
                arg = args[++i];
                int v{};
                const auto& [ptr, ec] =
                    std::from_chars(arg.begin(), arg.end(), v);
                if (ec != std::error_code{} || ptr != arg.end())
                {
                    return usage(args);
                }
                lg::debug_level = static_cast<lg::level>(v);
            }
        }
    }
    std::span<std::string_view> pargs{};
    if (i < args.size())
    {
        pargs = args.subspan(i);
    }
    // need to hold a reference to UI so we don't crash on shutdown
    // if messages print
    auto out = ui::get();

    smrty::Calculator& calc = smrty::Calculator::get();
    try
    {
        calc.run(std::format("{: }", pargs));
    }
    catch (const std::exception& e)
    {
        lg::emergency("uncaught exception: {}\n\nPress Enter to continue...\n",
                      e.what());
        std::string nextline;
        std::getline(std::cin, nextline);
    }
    out.reset();
    return 0;
}
