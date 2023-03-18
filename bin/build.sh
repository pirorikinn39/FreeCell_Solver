#!/bin/bash -e
CFLAGS+=" -Wall -O2"
[ ! "$1" = debug ] && CFLAGS+=" -DNDEBUG"
set -x
g++ -std=c++11 src/common/card.cpp src/solver1/Bits.cpp src/solver1/Position.cpp src/solver1/Solve.cpp src/solver1/Main.cpp $CFLAGS -o bin/solver1
g++ -std=c++11 src/common/card.cpp src/solver2/Bits.cpp src/solver2/Position.cpp src/solver2/Solve.cpp src/solver2/Main.cpp $CFLAGS -o bin/solver2
