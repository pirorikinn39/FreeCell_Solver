#!/bin/bash -eu
if [ "${1-default}" = debug ]
then CFLAGS+=" -Wall -O2 -g"
else CFLAGS+=" -Wall -O2 -DNDEBUG"
fi
set -x
g++ -std=c++11 src/common/card.cpp src/solver1/bits.cpp src/solver1/position.cpp src/solver1/solve.cpp src/solver1/main.cpp $CFLAGS -o bin/solver1
g++ -std=c++11 src/common/card.cpp src/solver2/bits.cpp src/solver2/position.cpp src/solver2/solve.cpp src/solver2/main.cpp $CFLAGS -o bin/solver2
