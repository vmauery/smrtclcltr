/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#include <function.hpp>

namespace function
{

struct square : public CalcFunction
{
    virtual const std::string& name() const final
    {
        static const std::string _name{"sqr"};
        return _name;
    }
    virtual const std::string& help() const final
    {
        static const std::string _help{
            // clang-format off
            "\n"
            "    Usage: x sqr\n"
            "\n"
            "    Returns the square of the bottom item on the stack: "
            "x^2\n"
            // clang-format on
        };
        return _help;
    }

    virtual bool op(Calculator& calc) const final
    {
        return one_arg_op(calc,
                          [](const auto& a, const units::unit& ua)
                              -> std::tuple<numeric, units::unit> {
                              return {a * a, ua * ua};
                          });
    }
};

} // namespace function

register_calc_fn(square);
