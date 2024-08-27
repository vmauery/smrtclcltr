/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/
#pragma once

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>
#include <vector>

std::string get_file_contents(const std::filesystem::path& filename);

namespace smrty
{

class Config
{
  protected:
    struct Private
    {
    };

    Config() = delete;
    Config(const Config&) = delete;
    Config(Config&&) = delete;

  public:
    Config(const Private&, std::string_view profile);
    ~Config();

    static std::unique_ptr<Config>& get(std::string_view profile = "");

    std::optional<std::string_view> readline();
    std::filesystem::path path_of(std::string_view name);

  protected:
    std::filesystem::path cfg_dir;
    std::string profile_name;
    // backing store for split-out lines
    std::string data;
    std::vector<std::string_view> lines;
    std::vector<std::string_view>::iterator next_line;
};

} // namespace smrty
