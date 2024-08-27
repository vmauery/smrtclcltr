/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#include <fcntl.h>

#include <cerrno>
#include <chrono>
#include <config.hpp>
#include <debug.hpp>
#include <fstream>
#include <memory>
#include <ranges>
#include <std_container_format.hpp>
#include <string>
#include <thread>

std::string get_file_contents(const std::filesystem::path& filename)
{
    std::ifstream in(filename, std::ios::in | std::ios::binary);
    if (in)
    {
        std::string contents;
        in.seekg(0, std::ios::end);
        contents.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&contents[0], contents.size());
        in.close();
        return contents;
    }
    return "";
}

namespace smrty
{

Config::Config(const Private&, std::string_view profile) : profile_name(profile)
{
    const char* home = std::getenv("HOME");
    if (!home)
    {
        lg::error("HOME is not set in environment; no config available\n");
        return;
    }
    cfg_dir = std::filesystem::path(home) / ".config" / "smrtclcltr";
    std::error_code ec{};
    auto cfgdir_status = std::filesystem::status(cfg_dir, ec);
    if (!ec && !std::filesystem::exists(cfgdir_status))
    {
        std::filesystem::create_directories(cfg_dir, ec);
        if (ec)
        {
            lg::error("failed to create directory for configuration ({}): {}\n",
                      cfg_dir, ec);
        }
    }
    // load config
    std::filesystem::path cfg_path = path_of("config");
    data = get_file_contents(cfg_path);
    constexpr std::string_view nl{"\n"};
    for (const auto line : std::views::split(data, nl))
    {
        lines.emplace_back(std::string_view{line});
    }
}

Config::~Config()
{
}

std::filesystem::path Config::path_of(std::string_view name)
{
    std::filesystem::path p =
        cfg_dir / std::format("{}_{}", profile_name, name);
    std::error_code ec{};
    if (!std::filesystem::exists(p, ec) || ec)
    {
        int fd = open(p.c_str(), O_WRONLY | O_CREAT,
                      S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        close(fd);
    }
    return p;
}

std::unique_ptr<Config>& Config::get(std::string_view profile)
{
    static std::unique_ptr<Config> _this{};
    static std::string current_profile = "default";
    if (profile.size() == 0)
    {
        profile = current_profile;
    }
    if (current_profile != profile)
    {
        _this.reset();
    }
    if (!_this)
    {
        current_profile = std::string{profile};
        _this = std::make_unique<Config>(Private{}, profile);
    }
    return _this;
}

std::optional<std::string_view> Config::readline()
{
    if (next_line != lines.end())
    {
        return *next_line++;
    }
    return std::nullopt;
}

} // namespace smrty
