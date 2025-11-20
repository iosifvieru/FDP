#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int my_pid, world_size;
    int d = 500, u = 100;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);

    long current_clock = clock();
    printf("pid: %d current clock: %ld\n", my_pid, current_clock);
    
    long differences[world_size];
    
    MPI_Allgather(&current_clock, 1, MPI_LONG, &differences, 1, MPI_LONG, MPI_COMM_WORLD);

    long sum = 0;

    for(int i = 0; i < world_size; i++){
        differences[i] = differences[i] + d - (u / 2) - clock();
        sum += differences[i];
    }

    long adjustment = sum / world_size;
    printf("pid: %d, my adjusment: %ld\n", my_pid, adjustment);

    // current_clock += adjustment;
    printf("pid: %d, my clock after adjustment is: %ld\n", my_pid, clock() + adjustment);
    
    MPI_Finalize();
    return 0;
}
