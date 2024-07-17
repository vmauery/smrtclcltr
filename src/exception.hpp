/*
Copyright © 2024 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <exception>
#include <string>

namespace smrty
{

struct insufficient_args : public std::runtime_error
{
    insufficient_args() : std::runtime_error("Insufficient arguments")
    {
    }
};

struct smrty_exception : public std::exception
{
    explicit smrty_exception(const std::string& msg = "gnrc smrty xcptn") :
        std::exception(), msg(msg)
    {
    }
    virtual ~smrty_exception()
    {
    }
    virtual const char* what() const noexcept
    {
        return msg.c_str();
    }
    std::string msg;
};

struct not_yet_implemented : public smrty_exception
{
    not_yet_implemented(const std::string& msg) : smrty_exception(msg)
    {
    }
};

class bad_conversion : public std::runtime_error
{
  public:
    bad_conversion() : std::runtime_error("bad conversion")
    {
    }
    ~bad_conversion() = default;
};

class unit_parse_error : public std::runtime_error
{
  public:
    unit_parse_error() : std::runtime_error("unit parse error")
    {
    }
    ~unit_parse_error() = default;
};

struct units_prohibited : public std::invalid_argument
{
    units_prohibited() : std::invalid_argument("Units not allowed")
    {
    }
};

struct units_mismatch : public std::invalid_argument
{
    units_mismatch() : std::invalid_argument("Units do not match")
    {
    }
};

} // namespace smrty
