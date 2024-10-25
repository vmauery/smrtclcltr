/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>
#include <type_helpers.hpp>

template <typename T>
struct basic_list
{
    using element_type = T;
    using default_element_type = typename variant0_or_single<T>::type;
    using iterator = std::vector<T>::iterator;

    std::vector<T> values;

    basic_list() : values()
    {
    }
    explicit basic_list(size_t n) : values()
    {
        values.resize(n);
    }
    basic_list(std::initializer_list<T> init) : values(init)
    {
        reduce();
    }
    basic_list(const std::vector<T>& init)
    {
        values.insert(values.begin(), init.begin(), init.end());
        reduce();
    }
    basic_list(std::vector<T>&& init) :
        values(std::forward<std::vector<T>>(init))
    {
        reduce();
    }

    basic_list(const basic_list<T>& l) : values(l.values)
    {
    }

    template <typename O>
        requires(!same_type_v<O, T>)
    basic_list(const basic_list<O>& l) : values()
    {
        // convert from a list of type O to a list of type T, if possible
        values.reserve(l.size());
        for (const auto& v : l.values)
        {
            if constexpr (is_variant_v<O> && is_variant_v<T>)
            {
                T vp{};
                if (!::reduce(v, vp)())
                {
                    throw std::invalid_argument("failed to convert list");
                }
                values.emplace_back(std::move(vp));
            }
            else if constexpr (is_variant_v<O> && !is_variant_v<T>)
            {
                if (auto vp = std::get_if<T>(&v); vp)
                {
                    values.emplace_back(*vp);
                }
                else
                {
                    throw std::invalid_argument("failed to convert list");
                }
            }
            else if constexpr ((!is_variant_v<O> && is_variant_v<T>) &&
                               variant_has_member<O, T>::value)
            {
                values.push_back(v);
            }
            else /* neither element type is variant; but are different types */
            {
                // currently don't have a way to deal with this
                throw std::invalid_argument("failed to convert list");
            }
        }
    }

    basic_list<T>& operator=(const basic_list<T>& l)
    {
        values = l.values;
        return *this;
    }

    iterator begin()
    {
        return values.begin();
    }
    iterator end()
    {
        return values.end();
    }

    void reduce()
    {
        if constexpr (is_variant_v<T>)
        {
            for (auto iter = values.begin(); iter != values.end(); iter++)
            {
                *iter = reduce_numeric(*iter);
            }
        }
    }

    basic_list<T> sub(size_t pos, size_t count) const
    {
        if ((pos + count) >= values.size())
        {
            throw std::out_of_range("out of bounds sub list");
        }
        std::vector<T> d{};
        d.reserve(count);
        d.insert(d.begin(), values.begin() + pos, values.begin() + pos + count);
        return basic_list(std::move(d));
    }

    size_t size() const
    {
        return values.size();
    }

    void reverse()
    {
        std::reverse(values.begin(), values.end());
    }

    basic_list<T> operator+(const basic_list<T>& r) const
    {
        basic_list<T> s{};
        s.values.reserve(size() + r.size());
        s.values.insert(s.values.end(), values.begin(), values.end());
        s.values.insert(s.values.end(), r.values.begin(), r.values.end());
        return s;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_list<T> operator+(const Type& v) const
    {
        // special case for zero addition
        if (v == decltype(v){0})
        {
            return *this;
        }
        T tv{v};
        basic_list<T> r(size());
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = (*in_iter) + tv;
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_list<T> operator-(const Type& v) const
    {
        // special case for zero subtraction
        if (v == decltype(v){0})
        {
            return *this;
        }
        T tv{v};
        basic_list<T> r(size());
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = (*in_iter) - tv;
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_list<T> operator*(const Type& v) const
    {
        // special case for unitary multiplication
        if (v == decltype(v){1})
        {
            return *this;
        }
        T tv{v};
        basic_list<T> r(size());
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = (*in_iter) * tv;
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_list<T> operator/(const Type& v) const
    {
        // special case for unitary division
        if (v == decltype(v){1})
        {
            return *this;
        }
        T tv{v};
        basic_list<T> r(size());
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = (*in_iter) / tv;
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }
};
