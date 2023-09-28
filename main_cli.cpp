/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <calculator.hpp>
#include <iostream>
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
        std::cerr << "uncaught exception: " << e.what()
                  << "\nPress Enter to continue...\n";
        std::string nextline;
        std::getline(std::cin, nextline);
    }
    return 0;
}
