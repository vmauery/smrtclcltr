/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <chrono>
#include <cmath>
#include <iostream>
#include <numeric.hpp>
#include <variant>

std::ostream& operator<<(std::ostream& out, const numeric& n)
{
    return std::visit(
        [&out](const auto& nn) -> std::ostream& { return out << nn; }, n);
}

mpz make_fixed(const mpz& v, int bits, bool is_signed)
{
    mpz one(1);
    mpz max_half = one << (bits - 1);
    mpz max_signed_value = max_half - 1;
    mpz max_mask = (one << bits) - 1;

    mpz s1;
    if (is_signed)
    {
        s1 = v & max_mask;
        if (s1 > max_signed_value)
        {
            s1 = -(max_half + ((~(v & max_mask)) + 1) % max_half);
        }
    }
    else
    {
        s1 = v & max_mask;
    }
    return s1;
}
