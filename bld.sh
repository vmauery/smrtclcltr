#!/bin/bash

export BOOST_ROOT="$HOME/local/git/boost"
export CC=clang
export CXX=clang++
export NUMERIC=mpfr

DIR_EXTRA=""
BUILDTYPE=""
# process args
args=()
while [ $# -gt 0 ]; do
	case "$1" in
		gcc) export CC=gcc; export CXX=g++; echo "Using GCC";;
		clang) export CC=clang; export CXX=clang++; echo "Using Clang";;
		mpfr) export NUMERIC="mpfr";;
		native) export NUMERIC="native";;
		gmp) export NUMERIC="gmp";;
		boost) export NUMERIC="boost";;
		release) export BUILDTYPE="-Dbuildtype=release"; DIR_EXTRA="-release";;
		*) args+=("$1");;
	esac
	shift
done

(
	export BUILD_DIR="build-${CC}-${NUMERIC}${DIR_EXTRA}"
	[ -d "$BUILD_DIR" ] || (
			mkdir "$BUILD_DIR"
			cd "$BUILD_DIR"
			meson setup "-Dnumeric=$NUMERIC" $BUILDTYPE ..
		)

	ninja -C "$BUILD_DIR" "${args[@]}"
) |& less -R
