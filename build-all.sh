#!/bin/sh

setup() {
	rm -rf .test-builds
	mkdir -p .test-builds

	# inform git we should ignore this dir
	echo "*" > .test-builds/.gitignore
}

progress_watcher() {
	local log="$1"
	local msg="$2"
	local v=""
	while :; do
		v=$(cat $log | grep "ninja: build stopped\|\[[0-9]\+/[0-9]\+\]" | sed "s,\[\([0-9]\+\)/\([0-9]\+\)\].*,\1/\2," | tail -n 1)
		case "$v" in
			"ninja: build stopped"*) return ;;
			"") v=0;; 
			*) v=$((100*$v)) ;;
		esac
		printf "$msg %02d%% " $v
		if [ $v -eq 100 ]; then
			return;
		fi
		sleep .1
	done
}

run() {
	while read NAME CC CXX; do
		for BACKEND in native boost gmp mpfr; do
			MSG="\rBuilding $NAME / $BACKEND:  "
			DIR=.test-builds/build-$NAME-$BACKEND
			mkdir -p $DIR
			CC=$CC CXX=$CXX meson -Dnumeric=$BACKEND $DIR >>$DIR/log 2>&1
			progress_watcher $DIR/log "$MSG" &
			ninja -v -C $DIR >>$DIR/log 2>&1
			br=$?
			wait %1
			if [ $br -ne 0 ]; then
				echo "FAIL!"
			else
				echo "OK"
			fi
		done
	done <<-EOF
		clang clang clang++
		gcc gcc g++
	EOF
}

setup
run
