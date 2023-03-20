# Implementation of solvers that find optimal solutions to FreeCell

This contains implementation developed in master's thesis [1].

<dl>
<dt> solver1 </dt>
<dd> written based on the method in Ref. [2] </dd>
<dt> solver2 </dt>
<dd> written based on the proposed method in Ref. [1] </dd>
</dl>

## How to build

A C++11 compiler is required. If you have an appropriate version of g++, execute bash script bin/build.sh to build bin/solver1 and bin/solver2.

## How to solve a FreeCell instance

Specify an initial cards' position using Microsoft game number. To solve game #2867, execute "bin/solver[12] 2867", and obtain outputs: "2867, 88, 6h 23 82 1a 8b 83 73 b3 7b 76 27 87 2c 8d 8h 1h 28 24 b8 58 2b 2h 2h d2 52 5d 5h 5h 7h 5h b5 1b 1h 1h ch 7c 7h 7h 7h ch 6h 6h ah 3a 67 65 37 6h 1h bh dh 3h 6h 4h 3b 35 36 46 4c 4d 4h 3h 41 4h 3h 35 3h ah 4h 8h 8h 3h 7h 7h 5h 8h 1h 1h bh 5h 6h 2h 2h dh 5h 5h 6h ch, 5564140, 1729771".
The outputs are comma-delimited, first output is the game number, second output is the solution length, third output is an optimal solution.
The moves in the solution is space-delimited, and each move represents source (first character) and desitination (second character). Each character represents one of free cells (a, b, c, d), piles (1,...,8), and home cell (h).


Hiroki Sukegawa and Kunihito Hoki

1. Hiroki Sukegawa, Development of freecell solvers for optimal solutions (最短手順を求めるフリーセルのソルバーの開発), Master's thesis, The University of Electro-Communications, Chofu, Tokyo, 2023 (written in Japanese).
2. Gerald Paul and Malte Helmert, Optimal Solitaire Game Solutions Using A<sup>*</sup> Search and Deadlock Analysis, Proc. of the ninth international symposium on combinatorial search (SoCS 2016), pp. 135--136, 2016.
