/*
Copyright © 2023 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <cstdint>
#include <debug.hpp>
#include <exception>
#include <format>

template <typename T>
struct basic_matrix
{
    using element_type = T;

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
    }
    basic_matrix(size_t cols, size_t rows) : cols(cols), rows(rows)
    {
        // pad or truncate, based on size
        values.resize(cols * rows);
    }
    basic_matrix(size_t cols, size_t rows, const std::span<T>& init) :
        cols(cols), rows(rows)
    {
        values.insert(values.begin(), init.begin(), init.end());
        // pad or truncate, based on size
        values.resize(cols * rows);
    }
    basic_matrix(size_t cols, size_t rows, std::vector<T>&& init) :
        cols(cols), rows(rows), values(std::forward<std::vector<T>>(init))
    {
        // pad or truncate, based on size
        values.resize(cols * rows);
    }

    // basic_matrix(const basic_matrix<T>& o) = default;
    // basic_matrix(basic_matrix<T>&& o) = default;

    static basic_matrix<T> I(size_t n)
    {
        basic_matrix<T> m(n, n);
        for (size_t i = 0; i < n; i++)
        {
            m.values[i + i * n] = 1;
        }
        return m;
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

    auto operator<=>(const basic_matrix<T>&) const = default;

    basic_matrix<T> operator+(const basic_matrix<T>& r) const
    {
        if (r.size() != size())
        {
            throw std::invalid_argument("matrix size mismatch for addition");
        }
        basic_matrix<T> s = *this;
        for (size_t i = 0; i < values.size(); i++)
        {
            s.values[i] += r.values[i];
        }
        return s;
    }

    basic_matrix<T> operator-(const basic_matrix<T>& r) const
    {
        if (r.size() != size())
        {
            throw std::invalid_argument("matrix size mismatch for subtraction");
        }
        basic_matrix<T> s = *this;
        for (size_t i = 0; i < values.size(); i++)
        {
            s.values[i] -= r.values[i];
        }
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
                    *out_iter += (*col_iter) * (*row_iter);
                    col_iter += r.cols;
                    row_iter += 1;
                }
                out_iter++;
            }
        }
        return p;
    }

    basic_matrix<T> operator*(const T& v) const
    {
        // special case for unitary multiplication
        if (v == T{1})
        {
            return *this;
        }
        basic_matrix<T> r(cols, rows);
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = *in_iter * v;
            out_iter++;
            in_iter++;
        }
        return r;
    }

    basic_matrix<T> operator/(const T& v) const
    {
        basic_matrix<T> r(cols, rows);
        auto out_iter = r.values.begin();
        auto in_iter = values.begin();
        for (size_t j = 0; j < size(); j++)
        {
            *out_iter = *in_iter / v;
            out_iter++;
            in_iter++;
        }
        return r;
    }

    basic_matrix<T>& operator*=(const T& v)
    {
        for (auto& iter : values)
        {
            iter *= v;
        }
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
        T sign = 1;
        for (size_t c = 0; c < cols; c++)
        {
            d += sign * values[c] * minor(c, 0).det();
            sign *= -1;
        }
        return d;
    }

    basic_matrix<T> adjoint() const
    {
        basic_matrix<T> a(cols, rows);
        auto in = values.begin();
        for (auto i = 0; i < cols; i++)
        {
            auto out = a.values.begin() + i;
            for (auto j = 0; j < rows; j++)
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
            *iter /= v;
            iter++;
        }
    }

    void row_op(size_t dst, size_t src, T factor)
    {
        if (factor == 0)
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
            *out_iter -= (*in_iter * factor);
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
            if (*v == 0)
            {
                size_t j = c;
                for (; j < cols; j++)
                {
                    if (m.values[j + cols * j] != 0)
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

        return i;
    }
};

template <typename T>
struct std::formatter<basic_matrix<T>>
{
    std::__format::_Spec<char> spec{};

    // Parses format like the standard int parser
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) -> decltype(ctx.begin())
    {
        // this is a simplification of integer format parsing from format
        auto begin = ctx.begin(), end = ctx.end();
        if (begin == end)
        {
            return begin;
        }
        begin = spec._M_parse_width(begin, end, ctx);
        if (begin == end)
        {
            return begin;
        }
        // offer a one-line format?
        /*
        switch (*begin)
        {
            case 'b':
                spec._M_type = std::__format::_Pres_b;
                ++begin;
                break;
            case 'd':
                spec._M_type = std::__format::_Pres_d;
                ++begin;
                break;
            case 'o':
                spec._M_type = std::__format::_Pres_o;
                ++begin;
                break;
            case 'x':
                spec._M_type = std::__format::_Pres_x;
                ++begin;
                break;
            default:
                // throw something
                break;
        }
        */
        if (begin == end)
        {
            return begin;
        }
        return begin;
    }

    template <typename FormatContext>
    auto format(const basic_matrix<T>& m, FormatContext& ctx) const
        -> decltype(ctx.out())
    {
        auto out = ctx.out();
        int pad = 1 + spec._M_get_width(ctx);
        *out++ = '[';
        auto iter = m.values.begin();
        // FIXME: use column width code?
        for (size_t r = 0; r < m.rows; r++)
        {
            if (r != 0)
            {
                *out++ = '\n';
                for (int i = 0; i < pad; i++)
                {
                    *out++ = ' ';
                }
            }
            *out++ = '[';
            for (size_t c = 0; c < m.cols; c++)
            {
                out = std::format_to(out, "{:f}", *iter++);
                if ((c + 1) < m.cols)
                {
                    *out++ = ' ';
                }
            }
            *out++ = ']';
        }
        *out++ = ']';
        return out;
    }
};
