all:
	mpicc -Wall main.c -o main.out

run:
	mpirun -np 8 --oversubscribe main.out

clean:
	rm *.out
