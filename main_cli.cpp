/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include "debug.hpp"

#include <calculator.hpp>
#include <string>
#include <vector>

int main()
{
    smrty::Calculator& calc = smrty::Calculator::get();
    try
    {
        calc.run();
    }
    catch (const std::exception& e)
    {
        lg::error("uncaught exception: {}\n\nPress Enter to continue...\n",
                  e.what());
        std::string nextline;
        std::getline(std::cin, nextline);
    }
    return 0;
}
