**Author**: Adam Zvara<br>
**Login**: xzvara01   <br>
**Date**: 04/2024     <br>
**Project variant**: Rubik's cube<br>

## Rubik's cube solver

This project implements simple Rubik's cube solver with Iterative Deepening Search (IDS) as its main algorithm. First,
the Rubik's cube is read from STDIN and is internally represented as an array of colors for each side of the cube
in the following order: <br>
`TOP = T, FRONT = F, RIGHT = R, BACK = B, LEFT = L, DOWN = D` <br>
`[6xT, 3xF, 3xR, 3xB, 3xL, 3xF, 3xR, 3xB, 3xL, 3xF, 3xR, 3xB, 3xL, 6xD]` <br>

Here is an example of representation of a solved cube
```
555
555
555
111 222 333 444
111 222 333 444
111 222 333 444
666
666
666
```

```
[5,5,5,5,5,5,5,5,5,1,1,1,2,2,2,3,3,3,4,4,4,1,1,1,2,2,2,3,3,3,4,4,4,1,1,1,2,2,2,3,3,3,4,4,4,6,6,6,6,6,6,6,6,6]
```

The most important part of the solution are the moves. The program contains predicates for 18 possible moves on the
Rubik's cube. Each move predicate is named after the move, except for counter-clockwise moves, which are appended with
the number 2 (for example move `u'` in standard notation is named `u2`). <br>

The IDS algorithm starts at depth = 0, which means, that it tries to find a single step solution to solve the cube.
If it does not succeed, the depth is increased until it reaches the maximum depth (default 40 moves).
If the solution is found at any given point, the steps leading to the solution are returned in a list. After that,
the solution steps are used to print out the cube throughout the solution. <br>

**It is important to note, that cube is solved, if each side contains a single color**, so the cube is considered solved
even if it is not rotated in the same way as described in the assignmed (top side containing color 5, down side 6 and so on).

## How to run

First, build the binary:
```
make
```

After that you can run the program with your cube directly:
```
./flp23-log < inputs/unsolved_example
```

or with Makefile:
```
make run ARGS=inputs/unsolved_example
```

The project contains some example inputs in the `inputs/` folder.
- `solved_basic` - solved cube
- `unsolved_example` - unsolved example cube from the project assignment

It also contains examples of progressively harder cubes (which require more steps to solve), where
each file name is in the following format: `unsolved_MOVESCNT_MOVES`, where `MOVESCNT` is the expected
number of moves to solve the cube and `MOVES` is a sequence of moves to solve the cube (the list was created
from reversing the moves, from which the cube was generated and applying opposite move to each move):
- `unsolved_1_u` - cube solvable in 1 move - `[u]`
- `unsolved_3_f2ru2` - cube solvable in 3 moves - `[f2, r, u2]`
- `unsolved_5_r2df2sm2` - cube solvable in 5 moves - `[r2, d, f2, s, m2]`
- `unsolved_6_f2b2r2u2l2u2` - cube solvable in 6 moves - `[f2, b2, r2, u2, l2, u2]`
- `unsolved_7_u2f2b2r2u2l2u2` - cube solvable in 7 moves - `[u2, f2, b2, r2, u2, l2, u2]`

All submitted cubes have been tested on merlin to aquire the time needed to solve each cube:

|        **Cube type**        | **Moves** | **Time (in seconds)** |
|:---------------------------:|:---------:|:---------------------:|
|        `solved_basic`       |     0     |         00.18         |
|      `unsolved_example`     |     2     |         00.20         |
|        `unsolved_1_u`       |     1     |         00.20         |
|      `unsolved_3_f2ru2`     |     3     |         00.23         |
|    `unsolved_5_r2df2sm2`    |     5     |         00.67         |
|  `unsolved_6_f2b2r2u2l2u2`  |     6     |         17.90         |
| `unsolved_7_u2f2b2r2u2l2u2` |     7     |         73.81         |

## Limitations

**Note, that the time required to solved the cube based on the solution moves is exponential**.
Therefore, it is only possible to solve cubes up to 6 moves in short time (about 20 seconds).