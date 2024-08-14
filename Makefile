all: 1

1:
	gcc -o openmp openmp.c -fopenmp -lm
	gcc -o sequencial sequencial.c -lm