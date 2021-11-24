/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <calculator.hpp>
#include <functional>
#include <string>
#include <tuple>

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
extern CalcFunction inverse;
extern CalcFunction divmod;
extern CalcFunction bitwise_and;
extern CalcFunction bitwise_or;
extern CalcFunction bitwise_xor;
extern CalcFunction bitwise_inv;
extern CalcFunction power;
extern CalcFunction modexp;
extern CalcFunction drop;
extern CalcFunction dup;
extern CalcFunction over;
extern CalcFunction swap;
extern CalcFunction clear;
extern CalcFunction depth;
extern CalcFunction factor;
extern CalcFunction gcd;
extern CalcFunction lcm;
extern CalcFunction combination;
extern CalcFunction permutation;
extern CalcFunction split;

} // namespace functions

namespace constants
{

extern CalcFunction e;
extern CalcFunction pi;
extern CalcFunction i;

} // namespace constants
