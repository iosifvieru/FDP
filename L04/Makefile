all:
	mpicc -Wall main.c -o main.out

run:
	mpirun -np 10 --oversubscribe main.out 2 5 2

clean:
	rm *.out