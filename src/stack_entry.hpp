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
        is_signed(true)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s,
                execution_flags& flags) :
        _unit(), base(b), fixed_bits(f), precision(p), is_signed(s)
    {
        store_value(std::move(v), flags);
    }

    stack_entry(numeric&& v, const smrty::units::unit& u, int b, int f, int p,
                bool s, execution_flags& flags) :
        _unit(u), base(b), fixed_bits(f), precision(p), is_signed(s)
    {
        store_value(std::move(v), flags);
    }

    const numeric& value() const
    {
        return _value;
    }

    void value(const numeric& n, execution_flags& flags)
    {
        store_value(numeric{n}, flags);
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
    void store_value(numeric&& v, execution_flags& flags);

    void emulate_int_types(mpz& v, execution_flags& flags);

  protected:
    numeric _value;
    smrty::units::unit _unit;

  public:
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
};

} // namespace smrty
