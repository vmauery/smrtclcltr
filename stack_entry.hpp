/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <numeric.hpp>
#include <units.hpp>

namespace smrty
{

class stack_entry
{
  public:
    stack_entry() :
        _value(), _unit(), base(10), fixed_bits(0), precision(8),
        is_signed(true), overflow(false), sign(false), carry(false), zero(false)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s) :
        _unit(), base(b), fixed_bits(f), precision(p), is_signed(s),
        overflow(false), sign(false), carry(false), zero(false)
    {
        _value = reduce_numeric(v, precision);
        if (mpz* v = std::get_if<mpz>(&_value); v != nullptr)
        {
            emulate_int_types(*v);
        }
    }

    stack_entry(numeric&& v, const smrty::units::unit& u, int b, int f, int p,
                bool s) :
        _unit(u),
        base(b), fixed_bits(f), precision(p), is_signed(s), overflow(false),
        sign(false), carry(false), zero(false)
    {
        _value = reduce_numeric(v, precision);
        if (mpz* v = std::get_if<mpz>(&_value); v != nullptr)
        {
            emulate_int_types(*v);
        }
    }

    const numeric& value() const
    {
        return _value;
    }
    void value(const numeric& n)
    {
        /*
          std::visit(
              [](const auto& v) {
                  lg::debug("value(): type(n) = {}, n = {}\n", DEBUG_TYPE(v),
          v);
              },
              n);
        */
        _value = reduce_numeric(n, precision);
        if (mpz* v = std::get_if<mpz>(&_value); v != nullptr)
        {
            emulate_int_types(*v);
        }
    }

    smrty::units::unit& unit()
    {
        return _unit;
    }
    void unit(const smrty::units::unit& u)
    {
        _unit = u;
    }
    void unit(std::string_view u)
    {
        _unit = smrty::units::unit(u);
    }

  protected:
    void emulate_int_types(mpz& v)
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
            mpz max_val = ((mpz{1} << (fixed_bits - 1)) - 1);
            mpz min_val = -(mpz{1} << fixed_bits - 1);
            overflow = (v > max_val) || (v < min_val);
            if (overflow)
            {
                mpz bits_val = (mpz{1} << fixed_bits);
                mpz mask = bits_val - 1;
                v &= mask;
                if (v > max_val)
                {
                    v -= bits_val;
                }
            }
        }
        else // unsigned
        {
            mpz max_val = ((mpz{1} << fixed_bits) - 1);
            carry = (v > max_val) || (v < zero);
            if (carry)
            {
                v &= max_val;
                if (v < zero)
                {
                    v += max_val;
                }
            }
        }
        zero = (v == zero);
        sign = (v < zero);
    }

  protected:
    numeric _value;
    smrty::units::unit _unit;

  public:
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
    bool overflow;
    bool sign;
    bool carry;
    bool zero;
};

} // namespace smrty
