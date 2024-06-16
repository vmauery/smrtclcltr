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
struct basic_matrix
{
    using element_type = T;
    using default_element_type = typename variant0_or_single<T>::type;

    size_t cols;
    size_t rows;
    std::vector<T> values;

    basic_matrix() : cols(0), rows(0), values()
    {
    }
    basic_matrix(size_t cols, size_t rows, std::initializer_list<T> init) :
        cols(cols), rows(rows), values(init)
    {
        // pad or truncate, based on size
        values.resize(cols * rows);
        reduce();
    }
    basic_matrix(size_t cols, size_t rows) : cols(cols), rows(rows)
    {
        // pad or truncate, based on size
        values.resize(cols * rows);
    }
    basic_matrix(size_t cols, size_t rows, const std::vector<T>& init) :
        cols(cols), rows(rows)
    {
        values = init;
        // pad or truncate, based on size
        values.resize(cols * rows);
        reduce();
    }
    basic_matrix(size_t cols, size_t rows, std::vector<T>&& init) :
        cols(cols), rows(rows), values(std::forward<std::vector<T>>(init))
    {
        // pad or truncate, based on size
        values.resize(cols * rows);
        reduce();
    }

    // basic_matrix(const basic_matrix<T>& o) = default;
    // basic_matrix(basic_matrix<T>&& o) = default;

    static basic_matrix<T> I(size_t n)
    {
        basic_matrix<T> m(n, n);
        for (size_t i = 0; i < n; i++)
        {
            m.values[i + i * n] = default_element_type{1};
        }
        return m;
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

    std::span<const T> row(size_t r) const
    {
        if (r < rows)
        {
            std::span<const T> d(values);
            return d.subspan(r * cols, cols);
        }
        throw std::out_of_range(std::format("row {} beyond bounds", r));
    }

    basic_matrix<T> sub(size_t scol, size_t srow, size_t ncol,
                        size_t nrow) const
    {
        if (((scol + ncol) >= cols) || ((srow + nrow) >= rows))
        {
            throw std::out_of_range("out of bounds sub matrix");
        }
        std::vector<T> d{};
        d.reserve(ncol * nrow);
        for (size_t r = srow; r < (srow + nrow); r++)
        {
            auto rd = row(r);
            d.append_range({rd.begin() + scol, rd.begin() + scol + ncol});
        }
        return basic_matrix(ncol, nrow, d);
    }

    basic_matrix<T> minor(size_t scol, size_t srow) const
    {
        if ((scol >= cols) || (srow >= rows))
        {
            throw std::out_of_range("out of bounds minor matrix");
        }
        std::vector<T> d{};
        auto out = values.begin();
        d.reserve((cols - 1) * (rows - 1));
        for (size_t r = 0; r < rows; r++)
        {
            if (r == srow)
            {
                out += cols;
                continue;
            }
            for (size_t c = 0; c < cols; c++)
            {
                if (c == scol)
                {
                    out++;
                    continue;
                }

                d.push_back(*out++);
            }
        }
        return basic_matrix(cols - 1, rows - 1, d);
    }

    size_t size() const
    {
        return values.size();
    }

    void col_swap(size_t one, size_t two)
    {
        auto one_iter = values.begin() + one;
        auto two_iter = values.begin() + two;
        for (size_t i = 0; i < rows; i++)
        {
            std::swap(*one_iter, *two_iter);
            one_iter += cols;
            two_iter += cols;
        }
    }

    void row_swap(size_t one, size_t two)
    {
        if (one >= rows || two >= rows)
        {
            throw std::out_of_range("invalid row index");
        }
        auto one_iter = values.begin() + cols * one;
        auto two_iter = values.begin() + cols * two;
        for (size_t i = 0; i < cols; i++)
        {
            std::swap(*one_iter, *two_iter);
            one_iter++;
            two_iter++;
        }
    }

    basic_matrix<T> operator+(const basic_matrix<T>& r) const
    {
        if (r.size() != size())
        {
            throw std::invalid_argument("matrix size mismatch for addition");
        }
        basic_matrix<T> s(cols, rows);
        for (size_t i = 0; i < values.size(); i++)
        {
            s.values[i] = values[i] + r.values[i];
        }
        s.reduce();
        return s;
    }

    basic_matrix<T> operator-(const basic_matrix<T>& r) const
    {
        if (r.size() != size())
        {
            throw std::invalid_argument("matrix size mismatch for subtraction");
        }
        basic_matrix<T> s(cols, rows);
        for (size_t i = 0; i < values.size(); i++)
        {
            s.values[i] = values[i] - r.values[i];
        }
        s.reduce();
        return s;
    }

    basic_matrix<T> operator*(const basic_matrix<T>& r) const
    {
        lg::debug("mult: ({}x{})*({}x{})\n", cols, rows, r.cols, r.rows);
        if (r.rows != cols)
        {
            throw std::invalid_argument(
                "matrix size mismatch for multiplication");
        }
        basic_matrix<T> p(r.cols, rows);
        auto out_iter = p.values.begin();
        for (size_t i = 0; i < p.rows; i++)
        {
            for (size_t j = 0; j < p.cols; j++)
            {
                auto row_iter = values.begin() + cols * i;
                auto col_iter = r.values.begin() + j;
                for (size_t k = 0; k < cols; k++)
                {
                    *out_iter = *out_iter + (*col_iter) * (*row_iter);
                    col_iter += r.cols;
                    row_iter += 1;
                }
                out_iter++;
            }
        }
        p.reduce();
        return p;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_matrix<T> operator*(const Type& v) const
    {
        // special case for unitary multiplication
        if (v == decltype(v){1})
        {
            return *this;
        }
        basic_matrix<T> r(cols, rows);
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = std::visit([&v](const auto& a) -> T { return a * v; },
                                   *in_iter);
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_matrix<T> operator/(const Type& v) const
    {
        // special case for unitary division
        if (v == decltype(v){1})
        {
            return *this;
        }
        basic_matrix<T> r(cols, rows);
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = std::visit([&v](const auto& a) -> T { return a / v; },
                                   *in_iter);
            out_iter++;
            in_iter++;
        }
        r.reduce();
        return r;
    }

    template <typename Type>
        requires is_one_of_v<Type, T>
    basic_matrix<T>& operator*=(const Type& v)
    {
        // special case for unitary multiplication
        if (v == decltype(v){1})
        {
            return *this;
        }
        for (auto& iter : values)
        {
            iter = std::visit([&v](const auto& a) -> T { return a * v; }, iter);
        }
        reduce();
        return *this;
    }

    T det() const
    {
        if (rows != cols)
        {
            throw std::invalid_argument(
                "matrix must be square for determinant");
        }
        if (rows == 1)
        {
            return values[0];
        }
        if (rows == 2)
        {
            // ad - bc
            return values[0] * values[3] - values[1] * values[2];
        }
        // calculate recursively
        T d{};
        T sign = default_element_type{1};
        for (size_t c = 0; c < cols; c++)
        {
            d = d + sign * values[c] * minor(c, 0).det();
            sign = sign * T{default_element_type{-1}};
        }
        return d;
    }

    basic_matrix<T> adjoint() const
    {
        basic_matrix<T> a(cols, rows);
        auto in = values.begin();
        for (size_t i = 0; i < cols; i++)
        {
            auto out = a.values.begin() + i;
            for (size_t j = 0; j < rows; j++)
            {
                *out = *in;
                in++;
                out += cols;
            }
        }
        return a;
    }

    void row_div(size_t row, T v)
    {
        if (row >= rows)
        {
            throw std::out_of_range("invalid row index");
        }
        auto iter = values.begin() + row * cols;
        for (size_t c = 0; c < cols; c++)
        {
            *iter = *iter / v;
            iter++;
        }
    }

    void row_op(size_t dst, size_t src, T factor)
    {
        if (std::visit([](const auto& a) { return a == decltype(a){0}; },
                       factor))
        {
            return;
        }
        if (src >= rows || dst >= rows)
        {
            throw std::out_of_range("invalid row index");
        }
        auto in_iter = values.begin() + src * cols;
        auto out_iter = values.begin() + dst * cols;
        for (size_t i = 0; i < cols; i++)
        {
            *out_iter = *out_iter - ((*in_iter) * factor);
            out_iter++;
            in_iter++;
        }
    }

    basic_matrix<T> inv() const
    {
        if (rows != cols)
        {
            throw std::invalid_argument("cannot invert a non-square matrix");
        }
        // need a copy of this to permute
        auto m = *this;
        // inverse starts out as I but changes into inverse when m == I(n)
        auto i = basic_matrix<T>::I(rows);
        // permutation matrix to keep track of row swaps
        auto p = i;
        // clear out all other entries in m for each diagonal
        // all the while, doing the same operations on i
        for (size_t c = 0; c < cols; c++)
        {
            auto v = m.values.begin() + cols * c + c;
            // need to swap if this entry has a zero
            if (std::visit([](const auto& a) { return a == decltype(a){0}; },
                           *v))
            {
                size_t j = c;
                for (; j < cols; j++)
                {
                    if (std::visit(
                            [](const auto& a) { return a == decltype(a){0}; },
                            m.values[j + cols * j]))
                    {
                        m.row_swap(c, j);
                        i.row_swap(c, j);
                        p.row_swap(c, j);
                        break;
                    }
                }
                if (j == cols)
                {
                    throw std::invalid_argument(
                        "cannot invert a singular matrix");
                }
            }
            T dv = *v;
            // scale row to get 1 in diagonal
            m.row_div(c, dv);
            i.row_div(c, dv);
            // add multiple to other rows to zeroize this column
            for (size_t r = 0; r < rows; r++)
            {
                if (r == c)
                {
                    continue;
                }
                T sf = m.values[r * cols + c];
                m.row_op(r, c, sf);
                i.row_op(r, c, sf);
            }
        }

        i.reduce();
        return i;
    }
};
