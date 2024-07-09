#pragma once

#include <exception>
#include <iostream>
#include <span>
#include <string_view>
#include <vector>

int cpp_main(std::span<std::string_view> args);
// int cpp_main(const std::vector<std::string_view>& args);

int main(int argc, char** argv)
{
    // create a span of string views
    std::vector<std::string_view> args{argv, argv + argc};
    return cpp_main(args);
}

#define main(...) cpp_main(__VA_ARGS__)
