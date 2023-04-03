#!/bin/bash -eu
DEBUG=" -DNDEBUG"
TEST_ZKEY=""
TARGETS=(solver1 solver2)
for arg in $@
do
    case $arg in
	solver1 )   TARGETS=(solver1);;
	solver2 )   TARGETS=(solver2);;
	debug )     DEBUG=" -g";;
	test-zkey ) TEST_ZKEY=" -DTEST_ZKEY";;
	* ) echo "bad argument: $arg" >&2; exit 1;;
    esac
done
CFLAGS+=" -Wall -O2 $DEBUG$TEST_ZKEY"

if [[ "${TARGETS[*]}" =~ solver1 ]]; then
    (set -x; g++ -std=c++11 src/common/card.cpp src/common/bits.cpp src/common/position-base.cpp src/common/utility.cpp src/solver1/position.cpp src/solver1/solve.cpp src/solver1/main.cpp $CFLAGS -o bin/solver1)
fi
if [[ "${TARGETS[*]}" =~ solver2 ]]; then
    (set -x; g++ -std=c++11 src/common/card.cpp src/common/bits.cpp src/common/position-base.cpp src/common/utility.cpp src/solver2/position.cpp src/solver2/solve.cpp src/solver2/main.cpp $CFLAGS -o bin/solver2)
fi

