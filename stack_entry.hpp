/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <numeric.hpp>
#include <units.hpp>

struct stack_entry
{
    stack_entry() :
        _value(), _unit(), base(10), fixed_bits(0), precision(8),
        is_signed(true)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s) :
        _unit(), base(b), fixed_bits(f), precision(p), is_signed(s)
    {
        _value = reduce_numeric(v, precision);
    }

    stack_entry(numeric&& v, const units::unit& u, int b, int f, int p,
                bool s) :
        _unit(u),
        base(b), fixed_bits(f), precision(p), is_signed(s)
    {
        _value = reduce_numeric(v, precision);
    }

    const numeric& value() const
    {
        return _value;
    }
    void value(const numeric& n)
    {
        std::visit(
            [](const auto& v) {
                lg::debug("value(): type(n) = {}, n = {}\n", DEBUG_TYPE(v), v);
            },
            n);
        _value = reduce_numeric(n, precision);
    }

    units::unit& unit()
    {
        return _unit;
    }
    void unit(const units::unit& u)
    {
        _unit = u;
    }
    void unit(const std::string& u)
    {
        _unit = units::unit(u);
    }

  protected:
    numeric _value;
    units::unit _unit;

  public:
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
};
