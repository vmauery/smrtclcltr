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

auto constexpr help = "TEMPLATE of first item on the stack";

} // namespace TEMPLATE
} // namespace function

namespace functions
{

CalcFunction TEMPLATE = {function::TEMPLATE::help, function::TEMPLATE::impl};

}
