#include <stdio.h>
#include <mpi/mpi.h>
#include <stdlib.h>
#include <time.h>

#define ATTACK 0
#define RETREAT 1

void print_decisions(int* decisions, int size, int pid){
    printf("pid: %d: ", pid);
    for(int i = 0; i < size; i++){
        printf("decision[%d] = %d ", i, decisions[i]);
    }
    printf("\n");
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    if(argc < 2){
        fprintf(stderr, "Usage: ./%s <no_faulty> <faulty pids>\n", argv[0]);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    int my_pid, world_size;
    int no_faulty_generals = atoi(argv[1]);
    int decision, is_faulty = 0;
    int* decision_array;

    MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_size < 4 * no_faulty_generals){
        fprintf(stderr, "Error: world_size(%d) must be greater than 4 * faulty(%d).\n", world_size, no_faulty_generals);
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    decision_array = (int*) malloc(world_size * sizeof(int));
    if(decision_array == NULL){
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    srand(time(NULL) + my_pid);
    decision = (rand() % 100) > 50 ? ATTACK : RETREAT;

    for(int i = 2; i < argc; i++){
        int faulty_pid = atoi(argv[i]);
        if(faulty_pid > world_size){
            fprintf(stderr, "faulty pid must be lower than world size.\n");
            MPI_Finalize();
            exit(EXIT_FAILURE);
        }
        if(my_pid == faulty_pid){
            is_faulty = 1;
        }
    }

    decision_array[my_pid] = decision;

    printf("pid: %d is_faulty: %d decision: %d\n", my_pid, is_faulty, decision);
    
    print_decisions(decision_array, world_size, my_pid);

    for(int k = 0; k <= no_faulty_generals; k++){

    }

    MPI_Finalize();
    return 0;
}