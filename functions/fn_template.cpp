/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{
namespace TEMPLATE
{

bool impl(Calculator& calc)
{
    (void)calc.stack.front();
    return true;
}

auto constexpr help =
    "\n"
    "    Usage: x TEMPLATE\n"
    "\n"
    "    Returns the TEMPLATE of the bottom item on the stack (x)\n";

} // namespace TEMPLATE
} // namespace function

namespace functions
{

CalcFunction TEMPLATE = {function::TEMPLATE::help, function::TEMPLATE::impl};

}
