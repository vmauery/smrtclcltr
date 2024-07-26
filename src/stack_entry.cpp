/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <stack_entry.hpp>

namespace smrty
{

void stack_entry::store_value(numeric&& v)
{
    _value = reduce_numeric(v, precision);
    if (mpz* v = std::get_if<mpz>(&_value); fixed_bits && v != nullptr)
    {
        execution_flags dummy{};
        emulate_int_types(*v, dummy);
    }
    else
    {
        std::visit(
            [](const auto& v) -> std::tuple<bool, bool> {
                using v_type = std::remove_cvref_t<decltype(v)>;
                if constexpr (is_real_v<v_type>)
                {
                    return {v == decltype(v){0}, v < decltype(v){0}};
                }
                else
                {
                    lg::debug("v is not real; z, s = false\n");
                    return {false, false};
                }
            },
            _value);
    }
}

void stack_entry::store_value(numeric&& v, execution_flags& flags)
{
    _value = reduce_numeric(v, precision);
    if (mpz* v = std::get_if<mpz>(&_value); fixed_bits && v != nullptr)
    {
        emulate_int_types(*v, flags);
    }
    else
    {
        const auto& [z, s] = std::visit(
            [](const auto& v) -> std::tuple<bool, bool> {
                using v_type = std::remove_cvref_t<decltype(v)>;
                if constexpr (is_real_v<v_type>)
                {
                    return {v == decltype(v){0}, v < decltype(v){0}};
                }
                else
                {
                    lg::debug("v is not real; z, s = false\n");
                    return {false, false};
                }
            },
            _value);
        flags.zero = z;
        flags.sign = s;
    }
    lg::debug("store_numeric flags: z({}) c({}) o({}) s({})\n", flags.zero,
              flags.carry, flags.overflow, flags.sign);
}

void stack_entry::emulate_int_types(mpz& v, execution_flags& flags)
{
    if (fixed_bits == 0)
    {
        return;
    }
#if 0
// 4-bit integers, for simplicity sake
// binary, unsigned interpretation, signed interpretation
// aligned for unsigned
0000 0001 0010 0011 0100 0101 0110 0111 1000 1001 1010 1011 1100 1101 1110 1111
   0    1    2    3    4    5    6    7    8    9   10   11   12   13   14   15
   0    1    2    3    4    5    6    7   -8   -7   -6   -5   -4   -3   -2   -1
// aligned for signed
1000 1001 1010 1011 1100 1101 1110 1111 0000 0001 0010 0011 0100 0101 0110 0111
   8    9   10   11   12   13   14   15    0    1    2    3    4    5    6    7
  -8   -7   -6   -5   -4   -3   -2   -1    0    1    2    3    4    5    6    7
#endif
    mpz zero{0};
    if (is_signed)
    {
        mpz max_val = ((one << (fixed_bits - 1)) - one);
        mpz min_val = -(one << (fixed_bits - 1));
        flags.overflow = (v > max_val) || (v < min_val);
        if (flags.overflow)
        {
            mpz bits_val = (one << fixed_bits);
            mpz mask = bits_val - one;
            v &= mask;
            if (v > max_val)
            {
                v -= bits_val;
            }
        }
    }
    else // unsigned
    {
        mpz max_val = ((one << fixed_bits) - one);
        flags.carry = (v > max_val) || (v < zero);
        if (flags.carry)
        {
            v &= max_val;
            if (v < zero)
            {
                v += max_val;
            }
        }
    }
    flags.zero = (v == zero);
    flags.sign = (v < zero);
}

} // namespace smrty
