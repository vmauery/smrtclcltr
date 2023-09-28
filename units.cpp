/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <algorithm>
#include <charconv>
#include <functions/common.hpp>
#include <units.hpp>
#include <vector>

namespace smrty
{
namespace units
{

// "kg*m^2/(A*s^3)" -- not possible now
// "kg*m^2*A^-1*s^-3" -- not working now
// "kg*m/s/s" -- works fine
namespace
{
const unit unitless(id_None);                        // -
const unit s(id_s);                                  // second
const unit min = s * Scale(60, 1);                   // minute
const unit hr = min * Scale(60, 1);                  // hour
const unit d = hr * Scale(24, 1);                    // day
const unit m(id_m);                                  // meter
const unit kg(id_kg);                                // kilogram
const unit g = kg / 1000;                            // gram
const unit A(id_A);                                  // ampere
const unit K(id_K);                                  // kelvin
const unit mol(id_mol);                              // 1/mol
const unit cd(id_cd);                                // candela
const unit rad(id_rad);                              // radians
const unit deg(id_deg);                              // degrees
const unit grad(id_grad);                            // gradians
const unit degC(id_degC);                            // deg Celsius
const unit dm = m / 10;                              // decimeter
const unit cm = m / 100;                             // centimeter
const unit mm = m / 1000;                            // millimeter
const unit L = dm * dm * dm;                         // liter
const unit mL = L / 1000;                            // milliliter
const unit N = kg * m / (s * s);                     // Newton
const unit Pa = kg / (m * s * s);                    // Pascal
const unit J = kg * m * m / (s * s);                 // Joule
const unit W = kg * m * m / (s * s * s);             // Watt
const unit C = s * A;                                // Coulomb
const unit V = kg * m * m / (s * s * s * A);         // Volt
const unit F = s * s * s * s * A * A / (kg * m * m); // Farad
const unit Ohm = kg * m * m / (s * s * s * A * A);   // Ohm
const unit S = s * s * s * A * A / (kg * m * m);     // siemens
const unit Wb = kg * m * m / (s * s * A);            // weber
const unit T = kg / (s * s * A);                     // tesla
const unit H = kg * m * m / (s * s * A * A);         // henry
const unit lm = cd;                                  // lumen
const unit lx = cd / (m * m);                        // lux
const unit Hz(id_None / id_s);                       // Hertz

// imperial units; ick
const unit in = m * Scale(254, 10000);                  // inch
const unit ft = in * Scale(12, 1);                      // foot
const unit yd = ft * Scale(3, 1);                       // yard
const unit mi = ft * Scale(5280, 1);                    // mile
const unit acre = mi * mi / Scale(640, 1);              // acre
const unit oz = kg * Scale(17636981, 500000);           // ounce
const unit lb = oz * Scale(16, 1);                      // pound
const unit ton = lb * Scale(2000, 1);                   // ton
const unit fl_oz = L * Scale(2957352965, 100000000000); // fluid ounce
const unit tbsp = fl_oz * Scale(1, 2);                  // tablespoon
const unit tsp = tbsp * Scale(1, 3);                    // teaspoon
const unit cup = fl_oz * Scale(8, 1);                   // cup
const unit pt = cup * Scale(2, 1);                      // pint
const unit qt = pt * Scale(2, 1);                       // quart
const unit gal = qt * Scale(4, 1);                      // gallon
const unit degF(id_degF);                               // deg Farenheit
const unit mph = mi / hr;                               // mph
const unit hp = W * Scale(746, 1);                      // horsepower

} // namespace

static boost::bimap<std::string_view, unit> units_map =
    make_bimap<std::string_view, unit>({// si units
                                        {"", unitless},
                                        {"s", s},
                                        {"m", m},
                                        {"m", m},
                                        {"dm", dm},
                                        {"cm", cm},
                                        {"mm", mm},
                                        {"kg", kg},
                                        {"g", g},
                                        {"A", A},
                                        {"K", K},
                                        {"mol", mol},
                                        {"cd", cd},
                                        {"rad", rad},
                                        {"deg", deg},
                                        {"grad", grad},
                                        {"degC", degC},
                                        {"degF", degF},
                                        {"N", N},
                                        {"Pa", Pa},
                                        {"J", J},
                                        {"W", W},
                                        {"C", C},
                                        {"V", V},
                                        {"F", F},
                                        {"Ohm", Ohm},
                                        {"S", S},
                                        {"Wb", Wb},
                                        {"T", T},
                                        {"H", H},
                                        {"lm", lm},
                                        {"lx", lx},
                                        {"Hz", Hz},
                                        // other scaled units
                                        {"min", min},
                                        {"hr", hr},
                                        // imperial units
                                        {"in", in},
                                        {"ft", ft},
                                        {"yd", yd},
                                        {"mi", mi},
                                        {"acre", acre},
                                        {"oz", oz},
                                        {"lb", lb},
                                        {"ton", ton},
                                        {"floz", fl_oz},
                                        {"tbsp", tbsp},
                                        {"tsp", tsp},
                                        {"cup", cup},
                                        {"pt", pt},
                                        {"qt", qt},
                                        {"gal", gal},
                                        {"degF", degF},
                                        {"mph", mph},
                                        {"hp", hp}});

unit::unit(std::string_view u) : id(1, 1), exp(1, 1), scale(1, 1)
{
    constexpr const std::array<char, 2> tokens = {'*', '/'};
    // parse unit string to turn it into an id
    // break up on "*" and "/"
    auto start = u.cbegin();
    char op = '*';
    char next_op = '*';
    while (start != u.cend())
    {
        auto next =
            std::find_first_of(start, u.cend(), tokens.cbegin(), tokens.cend());
        std::string_view ustr;
        if (next != u.cend())
        {
            next_op = *next;
            ustr = std::string_view{start, next};
            start = next + 1;
        }
        else
        {
            ustr = std::string_view{start, next};
            start = next;
        }
        int pval = 1;
        // further break down the unit kg^2 or m^-1
        auto p = ustr.find('^');
        if (p != std::string::npos)
        {
            ustr = ustr.substr(0, p);
            auto expview = ustr.substr(p + 1);
            std::from_chars(expview.data(), expview.data() + expview.size(),
                            pval);
        }
        auto units_it = units_map.left.find(ustr);
        if (units_it == units_map.left.end())
        {
            // failed to parse; a unit with id 0 is an error
            return;
        }
        unit uval = units_it->second;
        if (pval < 0)
        {
            if (op == '*')
            {
                op = '/';
            }
            else
            {
                op = '*';
            }
            pval = -pval;
        }
        if (op == '*')
        {
            id *= uval.id;
            exp *= uval.exp * pval;
            scale *= uval.scale;
        }
        else
        {
            id /= uval.id;
            exp /= uval.exp * pval;
            scale /= uval.scale;
        }
        op = next_op;
    }
}

numeric unit::conv(unit& o, const numeric& v) const
{
    if (id == o.id)
    {
        std::visit(
            [this, &o](const auto& n) {
                lg::debug(
                    "conv: type(v) = {}, v = {}, exp = {}, scale = {}, o.exp = "
                    "{}, o.scale = {}\n",
                    DEBUG_TYPE(n), n, exp, scale, o.exp, o.scale);
            },
            v);

        numeric vc = std::visit(
            [&o, this](const auto& n) -> numeric {
                lg::debug("type(v) = {}, v = {}, (mpf={})\n", DEBUG_TYPE(n), n,
                          DEBUG_TYPE(mpf{}));
                if constexpr (std::is_same_v<decltype(n), const mpf&>)
                {
                    return n * to_mpf((o.exp / exp) * (o.scale / scale));
                }
                else
                {
                    return n * (o.exp / exp) * (o.scale / scale);
                }
            },
            v);
        o = *this;
        return vc;
    }
    // special case for K/degF/degC
    else if ((id == id_K || id == id_degF || id == id_degC) &&
             (o.id == id_K || o.id == id_degF || o.id == id_degC))
    {
        // FIXME: what if exp or scale are set? They should not be....
        if (o.id == id_K)
        {
            if (id == id_degC)
            {
                // K->C
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return to_mpq(n) - mpq(5463, 20);
                    },
                    v);
                o = *this;
                return vc;
            }
            else // if (id == id_degF)
            {
                // K->F
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return (to_mpq(n) - mpq(5463, 20)) * mpq(9, 5) +
                               mpq(32, 1);
                    },
                    v);
                o = *this;
                return vc;
            }
        }
        else if (o.id == id_degF)
        {
            if (id == id_K)
            {
                // F->K
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return (to_mpq(n) - mpq(32, 1)) * mpq(5, 9) +
                               mpq(5463, 20);
                    },
                    v);
                o = *this;
                return vc;
            }
            else // if (id == id_degC)
            {
                // F->C
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return (to_mpq(n) - mpq(32, 1)) * mpq(5, 9);
                    },
                    v);
                o = *this;
                return vc;
            }
        }
        else // if (o.id == id_degC)
        {
            if (id == id_K)
            {
                // C->K
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return to_mpq(n) - mpq(5463, 20);
                    },
                    v);
                o = *this;
                return vc;
            }
            else // if (id == id_degF)
            {
                // C->F
                numeric vc = std::visit(
                    [](const auto& n) -> numeric {
                        return to_mpq(n) * mpq(9, 5) + mpq(32, 1);
                    },
                    v);
                o = *this;
                return vc;
            }
        }
    }
    throw bad_conversion();
}

unit pow(const unit& u, const mpf& p)
{
    // calculating the 'power' is more accurate if done
    // separately on the numerators and denominators
    mpq new_id(to_mpq(pow_fn(to_mpf(helper::numerator(u.id)), p)) /
               to_mpq(pow_fn(to_mpf(helper::denominator(u.id)), p)));
    return unit(new_id, u.exp, u.scale);
}

} // namespace units
} // namespace smrty

std::ostream& operator<<(std::ostream& out, const smrty::units::unit& n)
{
    bool debug = smrty::Calculator::get().config.debug;
    if (n.id == smrty::units::id_None)
    {
        if (debug)
        {
            out << "_<>(" << n.id << ", " << n.exp << ", " << n.scale << ")";
        }
        return out;
    }
    auto units_it = smrty::units::units_map.right.find(n);
    if (units_it != smrty::units::units_map.right.end())
    {
        out << "_" << units_it->second;
        if (debug)
        {
            out << "(" << n.id << ", " << n.exp << ", " << n.scale << ")";
        }
        return out;
    }
    // factor numerator and denominator of id
    std::vector<mpz> num_factors =
        smrty::function::util::prime_factor(helper::numerator(n.id));
    if (num_factors.size() == 0)
    {
        num_factors.push_back(helper::numerator(n.id));
    }
    std::vector<mpz> den_factors =
        smrty::function::util::prime_factor(helper::denominator(n.id));
    bool first = true;
    for (const auto& f : num_factors)
    {
        if (first)
        {
            out << "_";
            first = false;
        }
        else
        {
            out << "*";
        }
        units_it =
            smrty::units::units_map.right.find(smrty::units::unit(mpq{f, 1}));
        if (units_it != smrty::units::units_map.right.end())
        {
            out << units_it->second;
        }
        else
        {
            out << "?";
        }
    }
    if (first)
    {
        out << "1";
    }
    first = true;
    for (const auto& f : den_factors)
    {
        if (first)
        {
            out << "/";
            first = false;
        }
        else
        {
            out << "*";
        }
        units_it =
            smrty::units::units_map.right.find(smrty::units::unit(mpq{f, 1}));
        if (units_it != smrty::units::units_map.right.end())
        {
            out << units_it->second;
        }
        else
        {
            out << "?";
        }
    }
    if (debug)
    {
        out << "(" << n.id << ", " << n.exp << ", " << n.scale << ")";
    }
    return out;
}

/*

Reimagine units....

* each 'units' object contains a vector of 'unit' object
* each unit object has:
  * mpq id - one of the base units defined in this file
  * mpq exp - for si scaling
  * mpq scale - for non-si scaling
  * mpq power - for kg^-1 or kg^2 or kg^(1/2) and the like
* when printing a units, just loop through the units vector and
  print the unit (* or / based on sign of power)
* multiply or divide is just adding a new thing to the vector
* units::pow() will adjust the power of each item in the vector
* compat means that all the ids and powers are the same
  (can be expressed in terms of the other)
* reduce() will express vector in lowest terms, combining like
  terms by adding powers, replacing groups of terms with single
  composite terms (like kg*m/s^2 replaced with N, starting with
  searching for more complex terms and finishing with base SI)

*/
