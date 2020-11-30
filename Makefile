aoc : main.o
	cc -o aoc main.o

main.o : main.c
	cc -c main.c

