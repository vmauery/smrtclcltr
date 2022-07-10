/*
Copyright © 2020 Vernon Mauery; All rights reserved.

SPDX-License-Identifier: BSD-3-Clause
*/

#pragma once

#include <string>

struct Version
{
    static const char* version();
    static const char* timestamp();
    static std::string full();
};
