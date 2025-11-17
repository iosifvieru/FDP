all:
	mpicc -Wall main.c -o main.out

run:
	mpirun -np 8 --oversubscribe main.out 5 6 0 1 2 3  

clean:
	rm *.out
