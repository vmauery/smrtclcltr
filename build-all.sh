#!/bin/sh

rm -rf .test-builds
mkdir -p .test-builds

# inform git we should ignore this dir
echo "*" > .test-builds/.gitignore

while read NAME CC CXX; do
	for BACKEND in native boost gmp mpfr; do
		echo -n "Building $NAME / $BACKEND: "
		DIR=.test-builds/build-$NAME-$BACKEND
		mkdir -p $DIR
		CC=$CC CXX=$CXX meson -Dnumeric=$BACKEND $DIR >>$DIR/log 2>&1
		ninja -v -C $DIR >>$DIR/log 2>&1
		if [ $? -ne 0 ]; then
			echo "FAIL!"
		else
			echo "OK"
		fi
	done
done <<-EOF
	clang clang clang++
	gcc gcc g++
EOF
