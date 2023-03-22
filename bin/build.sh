#!/bin/bash -eu
# if [ "${1-default}" = debug ]
# then CFLAGS+=" -Wall -O2 -g"
# else CFLAGS+=" -Wall -O2 -DNDEBUG"
# fi
DEBUG=" -DNDEBUG"
TEST_ZKEY=""
for arg in $@
do
    case $arg in
	debug ) DEBUG=" -g";;
	test-zkey ) TEST_ZKEY=" -DTEST_ZKEY";;
	* ) echo "bad argument: $arg" >&2; exit 1;;
    esac
done
CFLAGS+=" -Wall -O2 $DEBUG$TEST_ZKEY"

set -x
g++ -std=c++11 src/common/card.cpp src/common/bits.cpp src/common/position-base.cpp src/solver1/position.cpp src/solver1/solve.cpp src/solver1/main.cpp $CFLAGS -o bin/solver1
g++ -std=c++11 src/common/card.cpp src/common/bits.cpp src/common/position-base.cpp src/solver2/position.cpp src/solver2/solve.cpp src/solver2/main.cpp $CFLAGS -o bin/solver2
