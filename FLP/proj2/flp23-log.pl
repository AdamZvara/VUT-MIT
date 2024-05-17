/**
 * Project variant: Rubik's cube
 * Author: Adam Zvara (xzvara01)
 * Date: 04/2024
 */

/* Main predicate */
start :-
	% Read the input cube
    prompt(_, ''),
    read_lines(LL),
    split_lines(LL,S),

	/*
	Parse the cube - the cube is represented as array of values for each side
	in the following order: TOP = T, FRONT = F, RIGHT = R, BACK = B, LEFT = L, DOWN = D
	[6T, 3F, 3R, 3B, 3L, 3F, 3R, 3B, 3L, 3F, 3R, 3B, 3L, 6D]
	*/
	flatten(S, X), !,

	% Solve the cube with IDS
	(is_solved(X) -> Solution_moves = [], Solved = true ; solve_ids(X, 0, 40, Solution_moves)),

	% Print the solution if it was found
	length(Solution_moves, Solution_length),
	((Solution_length \== 0; Solved) ->
		print_cube(X),
		print_solution(X, Solution_moves)
	;
		write('Could not find solution')
	).

/** =============== IO functions from input2.pl =============== */

read_line(L,C) :-
	get_char(C),
	(isEOFEOL(C), L = [], !;
		read_line(LL,_),
		[C|LL] = L).

isEOFEOL(C) :-
	C == end_of_file;
	(char_code(C,Code), Code==10).

read_lines(Ls) :-
	read_line(L,C),
	( C == end_of_file, Ls = [] ;
	  read_lines(LLs), Ls = [L|LLs]
	).

split_line([],[[]]) :- !.
split_line([' '|T], [[]|S1]) :- !, split_line(T,S1).
split_line([32|T], [[]|S1]) :- !, split_line(T,S1).
split_line([H|T], [[H|G]|S1]) :- split_line(T,[G|S1]).

split_lines([],[]).
split_lines([L|Ls],[H|T]) :- split_lines(Ls,T), split_line(L,H).

/** ===================== Printing Rubik's cube =====================
 * Since the cube is represented as an array of values for each side,
 * create a function to print out row consisting of 3 values ending with
 * delimiter (`print_3seq_delim`). Create another function, that
 * will print N of these sequences with specified delimiter
 * (`print_n_seqs_delim`), which can be used directly to print first
 * and last side. For the remaining sides, create another function, which
 * will print out 4 sides on the same row (`print_4sides_inline` where
 * delimiter between each side is ' ').
 */

/* Print 3 consecutive elements from array ending with Delimiter,
   return the remaining items (T) */
print_3seq_delim([E1, E2, E3 | T], Delim, T) :-
	format('~w~w~w~w', [E1, E2, E3, Delim]).

/* Print N consecutive sequences (of size 3) from Cube ending with
   Delimiter, return the remaining items (T) */
print_n_seqs_delim(T, 0, _, T).
print_n_seqs_delim(Cube, N, Delim, T) :-
	print_3seq_delim(Cube, Delim, T_new),
	N_new is N - 1,
	print_n_seqs_delim(T_new, N_new, Delim, T).

/* Print 4 sides of Cube N times in single line, return the remaining items (T) */
print_4sides_inline(T, 0, T).
print_4sides_inline(Cube, N, T) :-
	print_n_seqs_delim(Cube, 4, ' ', T_new),
	N_new is N - 1,
	nl,
	print_4sides_inline(T_new, N_new, T).

/* Print the Rubik's cube */
print_cube(Cube) :-
	% Print first side - 3 rows
	print_n_seqs_delim(Cube,  3, '\n', T1),
	% Print 4 sides of cube on 3 separate rows
	print_4sides_inline(T1, 3, T2),
	% Print the last side of the cube - 3 rows
	print_n_seqs_delim(T2, 3, '\n', _).

/** ===================== Moves on the Rubik's cube ===================== */

% Turn the top face clockwise
u(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T7, T4, T1, T8, T5, T2, T9, T6, T3,
		R1, R2, R3, B1, B2, B3, L1, L2, L3, F1, F2, F3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	]).

% Turn the top face counter-clockwise
u2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T3, T6, T9, T2, T5, T8, T1, T4, T7,
		L1, L2, L3, F1, F2, F3, R1, R2, R3, B1, B2, B3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	]).

% Turn the the horizontal slice clockwise
e(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		L4, L5, L6, F4, F5, F6, R4, R5, R6, B4, B5, B6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	]).

% Turn the the horizontal slice counter-clockwise
e2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		R4, R5, R6, B4, B5, B6, L4, L5, L6, F4, F5, F6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	]).

% Turn the bottom face clockwise
d(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		L7, L8, L9, F7, F8, F9, R7, R8, R9, B7, B8, B9,
		D7, D4, D1, D8, D5, D2, D9, D6, D3
	]).

% Turn the bottom face counter-clockwise
d2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		R7, R8, R9, B7, B8, B9, L7, L8, L9, F7, F8, F9,
		D3, D6, D9, D2, D5, D8, D1, D4, D7
	]).

% Turn the right face clockwise
r(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, F3, T4, T5, F6, T7, T8, F9,
		F1, F2, D3, R7, R4, R1, T9, B2, B3, L1, L2, L3,
		F4, F5, D6, R8, R5, R2, T6, B5, B6, L4, L5, L6,
		F7, F8, D9, R9, R6, R3, T3, B8, B9, L7, L8, L9,
		D1, D2, B7, D4, D5, B4, D7, D8, B1
	]).

% Turn the right face counter-clockwise
r2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, B7, T4, T5, B4, T7, T8, B1,
		F1, F2, T3, R3, R6, R9, D9, B2, B3, L1, L2, L3,
		F4, F5, T6, R2, R5, R8, D6, B5, B6, L4, L5, L6,
		F7, F8, T9, R1, R4, R7, D3, B8, B9, L7, L8, L9,
		D1, D2, F3, D4, D5, F6, D7, D8, F9
	]).

% Turn the left face clockwise
l(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		B9, T2, T3, B6, T5, T6, B3, T8, T9,
		T1, F2, F3, R1, R2, R3, B1, B2, D7, L7, L4, L1,
		T4, F5, F6, R4, R5, R6, B4, B5, D4, L8, L5, L2,
		T7, F8, F9, R7, R8, R9, B7, B8, D1, L9, L6, L3,
		F1, D2, D3, F4, D5, D6, F7, D8, D9
	]).

% Turn the left face counter-clockwise
l2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		F1, T2, T3, F4, T5, T6, F7, T8, T9,
		D1, F2, F3, R1, R2, R3, B1, B2, T7, L3, L6, L9,
		D4, F5, F6, R4, R5, R6, B4, B5, T4, L2, L5, L8,
		D7, F8, F9, R7, R8, R9, B7, B8, T1, L1, L4, L7,
		B9, D2, D3, B6, D5, D6, B3, D8, D9
	]).

% Turn the vertical slice clockwise
m(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, B8, T3, T4, B5, T6, T7, B2, T9,
		F1, T2, F3, R1, R2, R3, B1, D8, B3, L1, L2, L3,
		F4, T5, F6, R4, R5, R6, B4, D5, B6, L4, L5, L6,
		F7, T8, F9, R7, R8, R9, B7, D2, B9, L7, L8, L9,
		D1, F2, D3, D4, F5, D6, D7, F8, D9
	]).

% Turn the vertical slice counter-clockwise
m2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, F2, T3, T4, F5, T6, T7, F8, T9,
		F1, D2, F3, R1, R2, R3, B1, T8, B3, L1, L2, L3,
		F4, D5, F6, R4, R5, R6, B4, T5, B6, L4, L5, L6,
		F7, D8, F9, R7, R8, R9, B7, T2, B9, L7, L8, L9,
		D1, B8, D3, D4, B5, D6, D7, B2, D9
	]).

% Turn the front face clockwise
f(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, L9, L6, L3,
		F7, F4, F1, T7, R2, R3, B1, B2, B3, L1, L2, D1,
		F8, F5, F2, T8, R5, R6, B4, B5, B6, L4, L5, D2,
		F9, F6, F3, T9, R8, R9, B7, B8, B9, L7, L8, D3,
		R7, R4, R1, D4, D5, D6, D7, D8, D9
	]).

% Turn the front face counter-clockwise
f2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, T4, T5, T6, R1, R4, R7,
		F3, F6, F9, D3, R2, R3, B1, B2, B3, L1, L2, T9,
		F2, F5, F8, D2, R5, R6, B4, B5, B6, L4, L5, T8,
		F1, F4, F7, D1, R8, R9, B7, B8, B9, L7, L8, T7,
		L3, L6, L9, D4, D5, D6, D7, D8, D9
	]).

% Turn the top slice clockwise
s(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, L8, L5, L2, T7, T8, T9,
		F1, F2, F3, R1, T4, R3, B1, B2, B3, L1, D4, L3,
		F4, F5, F6, R4, T5, R6, B4, B5, B6, L4, D5, L6,
		F7, F8, F9, R7, T6, R9, B7, B8, B9, L7, D6, L9,
		D1, D2, D3, R8, R5, R2, D7, D8, D9
	]).

% Turn the top slice counter-clockwise
s2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		T1, T2, T3, R2, R5, R8, T7, T8, T9,
		F1, F2, F3, R1, D6, R3, B1, B2, B3, L1, T6, L3,
		F4, F5, F6, R4, D5, R6, B4, B5, B6, L4, T5, L6,
		F7, F8, F9, R7, D4, R9, B7, B8, B9, L7, T4, L9,
		D1, D2, D3, L2, L5, L8, D7, D8, D9
	]).

% Turn the back face clockwise
b(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		R3, R6, R9, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, D9, B7, B4, B1, T3, L2, L3,
		F4, F5, F6, R4, R5, D8, B8, B5, B2, T2, L5, L6,
		F7, F8, F9, R7, R8, D7, B9, B6, B3, T1, L8, L9,
		D1, D2, D3, D4, D5, D6, L1, L4, L7
	]).

% Turn the back face counter-clockwise
b2(
	[
		T1, T2, T3, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, R3, B1, B2, B3, L1, L2, L3,
		F4, F5, F6, R4, R5, R6, B4, B5, B6, L4, L5, L6,
		F7, F8, F9, R7, R8, R9, B7, B8, B9, L7, L8, L9,
		D1, D2, D3, D4, D5, D6, D7, D8, D9
	],
	[
		L7, L4, L1, T4, T5, T6, T7, T8, T9,
		F1, F2, F3, R1, R2, T1, B3, B6, B9, D7, L2, L3,
		F4, F5, F6, R4, R5, T2, B2, B5, B8, D8, L5, L6,
		F7, F8, F9, R7, R8, T3, B1, B4, B7, D9, L8, L9,
		D1, D2, D3, D4, D5, D6, R9, R6, R3
	]).

/* Return all possible moves on Rubik's cube */
all_moves(['u', 'u2', 'd', 'd2', 'r',  'r2', 'l', 'l2', 'f',
		  'f2', 'b', 'b2', 'm', 'm2', 'e', 'e2', 's', 's2']).

/* For each move define its reversed move */
opposite_move('u', 'u2').
opposite_move('d', 'd2').
opposite_move('r', 'r2').
opposite_move('l', 'l2').
opposite_move('f', 'f2').
opposite_move('b', 'b2').
opposite_move('m', 'm2').
opposite_move('e', 'e2').
opposite_move('s', 's2').
opposite_move('u2', 'u').
opposite_move('d2', 'd').
opposite_move('r2', 'r').
opposite_move('l2', 'l').
opposite_move('f2', 'f').
opposite_move('b2', 'b').
opposite_move('m2', 'm').
opposite_move('e2', 'e').
opposite_move('s2', 's').

/** ===================== Solving Rubik's cube =====================
 * The Rubik's cube is solved with Iterative Deepening Search (IDS)
 * algorithm. In each iteration, we increase the depth of the search
 * and try to find the solution in the current depth. If the solution
 * is not found, we increase the depth and try again.
 */

/*
Check if the cube is solved (does not check if the cube is rotated
the required way, as in project forum it was discussed, that any result
where the cube has a single color on each side is correct solution)
*/
is_solved([
	T, T, T, T, T, T, T, T, T,
	F, F, F, R, R, R, B, B, B, L, L, L,
	F, F, F, R, R, R, B, B, B, L, L, L,
	F, F, F, R, R, R, B, B, B, L, L, L,
	D, D, D, D, D, D, D, D, D]).
is_solved([_]) :- false.

/*
Print solution of the Rubik's cube from list of moves (predicates)
@Cube - the original cube
@Moves - list of predicates to be applied to the cube in order
*/
print_solution(_, []).
print_solution(Cube, [Move|Moves]) :-
	call(Move, Cube, Moved_cube),
	nl,
	print_cube(Moved_cube),
	print_solution(Moved_cube, Moves).

/*
Check, if list of moves leads to a solution (in single move)
@Cube - cube state to find the solution from
@Moves - list of moves to check
@Move_to_solve - move, which leads to solution (if found)
*/
list_contains_solution(_, [], _) :- false.
list_contains_solution(Cube, [Move|Moves], Move_to_solve) :-
	call(Move, Cube, New_cube),
	is_solved(New_cube), Move_to_solve = Move; % If solved, return the move
	list_contains_solution(Cube, Moves, Move_to_solve).

/*
Predicate to store, which move was the last .. this is useful so
we can eliminate the reverse move, as we would end up in the same
position as before
*/
:- dynamic lastMove/1.

/*
Delete reverse move from the list of moves, the last move is stored
in the lastMove predicate
@Moves - list of moves to delete from
@Clean_moves - list of moves without reverse move
*/
delete_reverse_move(Moves, Clean_moves) :-
	(lastMove(Last) ->
		opposite_move(Last, OppLast),
		delete(Moves, OppLast, Clean_moves)
	;
		Clean_moves = Moves
	).

/*
End condition of IDS iteration - if we are at depth 0, try to find solution
directly from the current position, if it exists, return it as an array
@Cube - current cube state
@Depth(=0) - current depth
@Move_to_solve - list containing single move to solve the cube from the current position (if found)
*/
ids_iteration(Cube, 0, [Move_to_solve]) :-
	!,
	all_moves(All_moves),
	delete_reverse_move(All_moves, Moves),
	list_contains_solution(Cube, Moves, Move_to_solve).

/*
IDS iteration - for each depth, try all possible moves and recursively call
the iteration with new depth
@Cube - current cube state
@Depth - current depth
@Moves_to_solve - list of moves to solve the cube from the current position
*/
ids_iteration(Cube, Depth, Moves_to_solve) :-
	New_depth is Depth - 1,
	all_moves(All_moves),
	% Delete reverse move from the list of moves
	delete_reverse_move(All_moves, Moves),
	% Get next possible move
	member(Move, Moves),
	% Store the current move as the last move
	retractall(lastMove(_)),
	assertz(lastMove(Move)),
	% Generate new cube state, recursively call the iteration with new depth
	call(Move, Cube, New_cube),
	ids_iteration(New_cube, New_depth, Result),
	% If we have found the solution, append the current move to the list
	append([Move], Result, Moves_to_solve).

/*
Solve the Rubik's cube with IDS
@Cube - initial cube state
@Current_depth - current depth of the iteration (set to 0 for starting)
@End_depth - maximum depth of the iteration
@Solution - list of moves to solve the cube
*/
solve_ids(_, End_depth, End_depth, []).
solve_ids(Cube, Current_depth, End_depth, Solution) :-
	ids_iteration(Cube, Current_depth, Solution);
	% The solution was not found, increase the depth and try again
	New_depth is Current_depth + 1,
	solve_ids(Cube, New_depth, End_depth, Solution).