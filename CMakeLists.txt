# Copyright © 2020 Vernon Mauery; All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required (VERSION 2.8.10 FATAL_ERROR)
#set (BUILD_SHARED_LIBRARIES OFF)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

project (smrtclcltr CXX)

enable_language(CXX)

INCLUDE(CheckIncludeFiles)
INCLUDE (CheckLibraryExists)
INCLUDE (CMakeDependentOption)
find_package(PkgConfig REQUIRED)


option(USE_BOOST_MATH "Build using the Boost CPP Multiprecision library" ON)
option(USE_NATIVE_TYPES "Build using the native types" OFF)
option(USE_GMP_MATH "Build using the Boost GMP Multiprecision library" OFF)
option(USE_MPFR_MATH "Build using the Boost MPFR Multiprecision library" OFF)

if (USE_GMP_MATH)
    pkg_check_modules(GMP REQUIRED IMPORTED_TARGET gmp)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GMP_CFLAGS} -DUSE_GMP_BACKEND")
    message("Using GMP for multiprecision support")
elseif (USE_MPFR_MATH)
    pkg_check_modules(MPFR REQUIRED IMPORTED_TARGET mpfr)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MPFR_CFLAGS} -DUSE_MPFR_BACKEND")
    message("Using MPFR for multiprecision support")
elseif (USE_BOOST_MATH)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BOOST_CPP_BACKEND")
    message("Using Boost CPP for multiprecision support")
else()
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_BASIC_TYPES")
    message("Using native types; no multiprecision support")
endif()

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

add_definitions (-DBOOST_ERROR_CODE_HEADER_ONLY)
add_definitions (-DBOOST_SYSTEM_NO_DEPRECATED)
add_definitions (-DBOOST_ALL_NO_LIB)
add_definitions (-DBOOST_NO_RTTI)
add_definitions (-DBOOST_NO_TYPEID)
add_definitions (-DBOOST_ASIO_DISABLE_THREADS)

include_directories (${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB FUNCTIONS functions/*.cpp)

add_executable (smrtclcltr main.cpp numeric.cpp constants.cpp calculator.cpp ${FUNCTIONS})
if (USE_GMP_MATH)
    target_link_libraries (smrtclcltr PUBLIC PkgConfig::GMP)
endif()
if (USE_MPFR_MATH)
    target_link_libraries (smrtclcltr PUBLIC PkgConfig::MPFR)
endif()
