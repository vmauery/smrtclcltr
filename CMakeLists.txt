# Copyright © 2020 Vernon Mauery; All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)
#set (BUILD_SHARED_LIBRARIES OFF)
set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (
    CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
    -Werror \
    -Wall \
    -Wextra \
    -Wnon-virtual-dtor \
    -Wold-style-cast \
    -Wcast-align \
    -Wunused \
    -Woverloaded-virtual \
    -Wpedantic \
    -Wmisleading-indentation \
    -Wnull-dereference \
    -Wdouble-promotion \
    -Wformat=2 \
    -Wno-reorder \
    -O3 \
    -g \
"
)
# todo: get rid of nos, add the below:
#  -Wshadow \
#  -Wconversion \

# for gcc, add these?
#   -Wduplicated-cond \
#   -Wduplicated-branches \
#   -Wlogical-op \
#   -Wuseless-cast \
#   -Wno-psabi \
#   -fconcepts \

set (CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

project (smrtclcltr CXX)

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)
add_definitions (-DBOOST_ASIO_DISABLE_THREADS)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

add_executable (smrtclcltr main.cpp numeric.cpp calculator.cpp )
target_link_libraries (smrtclcltr -lgmp)
