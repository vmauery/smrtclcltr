/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <cmath>
#include <numeric>
#include <ui.hpp>

column_layout find_best_layout(std::span<std::string_view> words, size_t width)
{
    constexpr size_t PADDING_SIZE = 2; // 2 spaces
    // max columns should be less than the ideal if there is minimal padding
    size_t total_chars =
        std::accumulate(words.begin(), words.end(), 0,
                        [](size_t sum, const std::string_view& w) {
                            return sum + w.size() + PADDING_SIZE;
                        });
    size_t max_columns = words.size() * width / total_chars;
    std::vector<column_layout> layouts;
    layouts.reserve(max_columns);
    for (size_t i = 0; i < max_columns; i++)
    {
        layouts.emplace_back(i + 1);
    }
    // place each word in each layout
    for (size_t idx = 0; idx < words.size(); idx++)
    {
        size_t w_len = words[idx].size() + PADDING_SIZE;
        for (auto& layout : layouts)
        {
            size_t col_count = layout.cols.size();
            size_t row_count =
                std::ceil(static_cast<double>(words.size()) / col_count);
            size_t this_col = idx / row_count;
            if (layout.valid)
            {
                if (w_len > layout.cols[this_col])
                {
                    layout.len += w_len - layout.cols[this_col];
                    layout.cols[this_col] = w_len;
                    if (layout.len > width)
                    {
                        layout.valid = false;
                    }
                }
            }
        }
    }
    // return the best layout
    for (auto l = layouts.rbegin(); l != layouts.rend(); l++)
    {
        if (l->valid)
        {
            return *l;
        }
    }
    return column_layout(1);
}
