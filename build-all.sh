#!/bin/bash -x

rm -rf .test-builds

CC_ARR=(clang gcc)
CXX_ARR=(clang++ g++)

BUILD_ERRORS=0

for ((i=0; i<${#CC_ARR[@]}; i++)); do
	CC=${CC_ARR[$i]}
	CXX=${CCX_ARR[$i]}
	for BACKEND in mpfr gmp boost native; do
		echo -n "Building $CC / $BACKEND: "
		DIR=.test-builds/build-$CC-$BACKEND
		mkdir -p $DIR
		CC=$CC CXX=$CXX meson -Dnumeric=$BACKEND $DIR >>$DIR/log 2>&1
		ninja -v -C $DIR >>$DIR/log 2>&1
		if [ $? -ne 0 ]; then
			echo -e "FAIL! Log follows\n"
			cat $DIR/log
			echo
			BUILD_ERRORS=$((BUILD_ERRORS+1))
		else
			echo "OK"
		fi
	done
done

exit $BUILD_ERRORS
