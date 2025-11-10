#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int number;
    int my_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    MPI_Request request;
    MPI_Status status;

    if (my_rank == 0) {
        // consumer
        while (1) {
            MPI_Irecv(&number, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &request);
            MPI_Wait(&request, &status);

            printf("I am root and I received %d\n", number);
        }
    }
    else {
        // producer
        srand(time(NULL) + my_rank);

        while (1)
        {
            int rnd = rand() % 100;

            MPI_Isend(&rnd, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);

            printf("I am %d and I am sending %d\n", my_rank, rnd);
            sleep(1);
        }
    }

    MPI_Finalize();
    return 0;
}
