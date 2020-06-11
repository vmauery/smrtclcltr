/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <deque>
#include <functional>
#include <map>
#include <numeric.hpp>
#include <string>
#include <tuple>
#include <type_traits>

constexpr bool const_or(bool b0)
{
    return b0;
}

template <typename B0, typename... BN>
constexpr bool const_or(B0 b0, BN... bN)
{
    return b0 | const_or(bN...);
}

template <typename... Ttypes, typename... Vtypes>
bool variant_holds_type(const std::variant<Vtypes...>& v)
{
    return const_or(std::holds_alternative<Ttypes>(v)...);
};

template <typename T, typename Vtype>
struct variant_has_member;

template <typename T, typename... Vtypes>
struct variant_has_member<T, std::variant<Vtypes...>>
    : public std::disjunction<std::is_same<T, Vtypes>...>
{
};

template <typename Vin, typename Vout>
struct reduce
{
    constexpr static size_t A_size = std::variant_size<Vin>::value;

    Vin& _vin;
    Vout& _vout;

    reduce(Vin& vin, Vout& vout) : _vin(vin), _vout(vout)
    {
    }

    bool extract_I(std::integral_constant<size_t, A_size>)
    {
        return false;
    }

    template <size_t I>
    bool extract_I(
        std::integral_constant<size_t, I> = std::integral_constant<size_t, 0>())
    {
        if constexpr (variant_has_member<std::variant_alternative_t<I, Vin>,
                                         Vout>::value)
        {
            auto p = std::get_if<I>(&_vin);
            if (p)
            {
                _vout = *p;
                return true;
            }
        }
        return extract_I(std::integral_constant<size_t, I + 1>());
    }

    bool operator()()
    {
        return extract_I<0>();
    }
};

struct stack_entry
{
    stack_entry() : base(10), fixed_bits(0), precision(8), is_signed(true)
    {
    }
    stack_entry(numeric&& v, int b, int f, int p, bool s) :
        value(v), base(b), fixed_bits(f), precision(p), is_signed(s)
    {
    }
    numeric value;
    int base;
    int fixed_bits;
    int precision;
    bool is_signed;
};

template <typename TypeOut, typename TypeIn>
TypeOut coerce_variant(const TypeIn& in)
{
    return TypeOut(in);
}

template <std::size_t N, class... Args>
using list_type_t = std::tuple_element_t<N, std::tuple<Args...>>;

template <typename... I>
struct conversion
{
    static void op(numeric&)
    {
    }
};

template <typename... Ti, typename... To>
struct conversion<std::tuple<Ti...>, std::tuple<To...>>
{
    constexpr static size_t I_size = sizeof...(Ti);

    static void op(numeric&, std::integral_constant<size_t, I_size>)
    {
    }
    template <size_t I>
    static void op(numeric& v, std::integral_constant<size_t, I>)
    {
        if (std::holds_alternative<list_type_t<I, Ti...>>(v))
        {
            v = coerce_variant<list_type_t<I, To...>>(
                std::get<list_type_t<I, Ti...>>(v));
        }
        op(v, std::integral_constant<size_t, I + 1>());
    }
    static void op(numeric& v)
    {
        static_assert(I_size >= 1);
        static_assert(sizeof...(Ti) == sizeof...(To));
        op(v, std::integral_constant<size_t, 0>());
    }
};

class calculator
{
  public:
    calculator();
    bool run();

  protected:
    void make_grammar();
    void make_functions();

    void show_stack();
    std::string get_next_token();
    bool run_one(const std::string& expr);

    template <typename Fn>
    bool one_arg_op(const Fn& fn)
    {
        if (_stack.size() < 1)
        {
            return false;
        }
        stack_entry a = _stack.front();
        _stack.pop_front();
        numeric cv;
        cv = std::visit([&fn](const auto& a) { return operate(fn, a); },
                        a.value);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename Fn, typename... Itypes, typename... Otypes,
              typename... Ltypes>
    bool one_arg_conv_op(const Fn& fn, const std::tuple<Itypes...>& /* in */,
                         const std::tuple<Otypes...>& /* out */,
                         const std::tuple<Ltypes...>& /*limit*/)
    {
        if (_stack.size() < 1)
        {
            return false;
        }
        stack_entry a = _stack.front();
        numeric& ca = a.value;
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        std::variant<Ltypes...> lca;
        if (!reduce(ca, lca)())
        {
            std::cerr << "wtf, we converted and now it doesn't reduce? Eat "
                         "shit c++.\n";
            return false;
        }
        _stack.pop_front();
        numeric cv =
            std::visit([&fn](const auto& a) { return operate(fn, a); }, lca);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename... AllowedTypes, typename Fn>
    bool one_arg_limited_op(const Fn& fn)
    {
        if (_stack.size() < 1)
        {
            return false;
        }
        stack_entry a = _stack.front();
        std::variant<AllowedTypes...> la;
        if (!variant_holds_type<AllowedTypes...>(a.value))
        {
            return false;
        }
        if (!reduce(a.value, la)())
        {
            return false;
        }
        _stack.pop_front();
        numeric cv =
            std::visit([&fn](const auto& a) { return operate(fn, a); }, la);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename Fn>
    bool two_arg_op(const Fn& fn)
    {
        if (_stack.size() < 2)
        {
            return false;
        }
        stack_entry a = _stack[1];
        stack_entry b = _stack[0];
        _stack.pop_front();
        _stack.pop_front();

        numeric cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            a.value, b.value);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename Fn, typename... Itypes, typename... Otypes,
              typename... Ltypes>
    bool two_arg_conv_op(const Fn& fn, const std::tuple<Itypes...>& /* in */,
                         const std::tuple<Otypes...>& /* out */,
                         const std::tuple<Ltypes...>& /*limit*/)
    {
        if (_stack.size() < 2)
        {
            return false;
        }
        stack_entry a = _stack[1];
        stack_entry b = _stack[0];

        numeric& ca = a.value;
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(ca);
        numeric& cb = b.value;
        conversion<std::tuple<Itypes...>, std::tuple<Otypes...>>::op(cb);
        std::variant<Ltypes...> lca;
        std::variant<Ltypes...> lcb;
        if (!reduce(ca, lca)() || !reduce(cb, lcb)())
        {
            std::cerr << "wtf, we converted and now it doesn't reduce? Eat "
                         "shit c++.\n";
            return false;
        }
        _stack.pop_front();
        _stack.pop_front();

        numeric cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            lca, lcb);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename... AllowedTypes, typename Fn>
    bool two_arg_limited_op(const Fn& fn)
    {
        if (_stack.size() < 2)
        {
            return false;
        }

        stack_entry a = _stack[1];
        stack_entry b = _stack[0];

        if (!variant_holds_type<AllowedTypes...>(a.value) ||
            !variant_holds_type<AllowedTypes...>(b.value))
        {
            return false;
        }
        std::variant<AllowedTypes...> la;
        std::variant<AllowedTypes...> lb;
        if (!reduce(a.value, la)() || !reduce(b.value, lb)())
        {
            return false;
        }
        _stack.pop_front();
        _stack.pop_front();

        numeric cv = std::visit(
            [&fn](const auto& a, const auto& b) { return operate(fn, a, b); },
            la, lb);
        _stack.emplace_front(std::move(cv), _base, _fixed_bits, _precision,
                             _is_signed);
        return true;
    }

    template <typename Fn>
    auto scaled_trig_op(auto a, const Fn& fn)
    {
        if (_angle_mode == e_angle_mode::deg)
        {
            a *= boost::math::constants::pi<mpf>() / 180;
        }
        else if (_angle_mode == e_angle_mode::grad)
        {
            a *= boost::math::constants::pi<mpf>() / 50;
        }
        return fn(a);
    }

    template <typename Fn>
    auto scaled_trig_op_inv(const auto& a, const Fn& fn)
    {
        auto b = fn(a);
        if (_angle_mode == e_angle_mode::deg)
        {
            b *= 180 / boost::math::constants::pi<mpf>();
        }
        else if (_angle_mode == e_angle_mode::grad)
        {
            b *= 50 / boost::math::constants::pi<mpf>();
        }
        return b;
    }

    enum class e_angle_mode
    {
        rad,
        deg,
        grad
    };

    bool debug();
    bool base();
    bool fixed_bits();
    bool precision();
    bool unsigned_mode();
    bool angle_mode(e_angle_mode);

    bool _debug = false;
    bool _running = true;
    int _base = 10;
    int _fixed_bits = 0;
    bool _is_signed = true;
    int _precision = 8;
    e_angle_mode _angle_mode = e_angle_mode::rad;
    std::deque<stack_entry> _stack;
    std::map<std::string, std::function<bool()>> _operations;
};
