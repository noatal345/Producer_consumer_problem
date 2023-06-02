all: main

main: main.c
	gcc -o ex3.out -std=c11 -pthread main.c
