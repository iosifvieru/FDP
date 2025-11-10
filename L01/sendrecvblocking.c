#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int arr_size;
    int my_rank, world_size;

    double *arr;

    if(argc != 2){
        printf("Usage: %s <ARR SIZE>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    arr_size = atoi(argv[1]);
    if(arr_size <= 0){
        printf("Array size cannot be < 0.\n");
        exit(EXIT_FAILURE);
    }

    arr = (double*) malloc(arr_size * sizeof(double));
    if(arr == NULL){
        printf("Unable to allocate memory.\n");
        exit(EXIT_FAILURE);
    }

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if(my_rank == 0){
        for(int i = 0; i < arr_size; i++){
            arr[i] = i;
        }
        
        MPI_Send(arr, arr_size, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
        printf("I am %d and I sent an array with %d elements\n", my_rank, arr_size);
    } else {
        // recv data
        MPI_Recv(arr, arr_size, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        printf("I am %d and I received: \n", my_rank);
        for(int i = 0; i < arr_size; i++){
            printf("%.2lf ", arr[i]);
        }

        printf("\n");
    }

    free(arr);
    MPI_Finalize();
    return 0;
}
