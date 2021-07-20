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

CHECK_INCLUDE_FILES(gmp.h HAS_GMP_HEADERS)
CHECK_LIBRARY_EXISTS (gmp __gmpf_abs "" HAS_GMP_LIB)

CMAKE_DEPENDENT_OPTION(USE_GMP_MATH "Build using the GNU Multiprecision (GMP) library" OFF
                       "HAS_GMP_HEADERS;HAS_GMP_LIB" OFF)

CHECK_INCLUDE_FILES(mpfr.h HAS_MPFR_HEADERS)
CHECK_LIBRARY_EXISTS (mpfr __mpfr_abs "" HAS_MPFR_LIB)

CMAKE_DEPENDENT_OPTION(USE_MPFR_MATH "Build using the GNU MPFR library" OFF
    "HAS_GMP_HEADERS;HAS_GMP_LIB;HAS_MPFR_HEADERS;HAS_MPFR_LIB" OFF)

option(USE_BOOST_MATH "Build using the Boost CPP Multiprecision library" ON)

if (USE_GMP_MATH)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_GMP_BACKEND")
    message("Using GMP for multiprecision support")
elseif (USE_MPFR_MATH)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DUSE_MPFR_BACKEND")
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
    target_link_libraries (smrtclcltr -lgmp)
endif()
if (USE_MPFR_MATH)
    target_link_libraries (smrtclcltr -lgmp -lmpfr)
endif()
