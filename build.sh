#!/bin/bash

g++ -std=c++11 Common/Card.cpp Solver1/Bits.cpp Solver1/Position.cpp Solver1/Solve.cpp Solver1/Main.cpp -Wall -O2 -DNDEBUG -o Solver1.out

g++ -std=c++11 Common/Card.cpp Solver2/Bits.cpp Solver2/Position.cpp Solver2/Solve.cpp Solver2/Main.cpp -Wall -O2 -DNDEBUG -o Solver2.out