/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <functional>
#include <string>
#include <tuple>

class Calculator;

using CalcFunction = std::tuple<std::string, std::function<bool(Calculator&)>>;

namespace functions
{

extern CalcFunction add;
extern CalcFunction subtract;
extern CalcFunction multiply;
extern CalcFunction divide;
extern CalcFunction range;
extern CalcFunction sum;
extern CalcFunction product;
extern CalcFunction sqrt;
extern CalcFunction sin;
extern CalcFunction cos;
extern CalcFunction tan;
extern CalcFunction asin;
extern CalcFunction acos;
extern CalcFunction atan;
extern CalcFunction log;
extern CalcFunction ln;
extern CalcFunction factorial;
extern CalcFunction negate;
extern CalcFunction divmod;
extern CalcFunction bitwise_and;
extern CalcFunction bitwise_or;
extern CalcFunction bitwise_xor;
extern CalcFunction bitwise_inv;
extern CalcFunction power;
extern CalcFunction drop;
extern CalcFunction dup;
extern CalcFunction swap;
extern CalcFunction clear;
extern CalcFunction depth;

} // namespace functions

namespace constants
{

extern CalcFunction e;
extern CalcFunction pi;
extern CalcFunction i;

} // namespace constants
