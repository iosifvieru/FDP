#include <stdio.h>
#include <mpi/mpi.h>
#include <stdlib.h>
#include <time.h>

#define ATTACK 0
#define RETREAT 1

typedef struct ByzantineMessage {
    int maj;
    int mult;
} ByzantineMessage;

void print_decisions(int* decisions, int size, int pid){
    printf("pid: %d: ", pid);
    for(int i = 0; i < size; i++){
        printf("decision[%d] = %d ", i, decisions[i]);
    }
    printf("\n");
}

ByzantineMessage compute_byzantine_message(int* decisions, int size){
    ByzantineMessage message;
    int maj[2] = {0, 0};
    
    for(int i = 0; i < size; i++){
        int decision = decisions[i];
        maj[decision]++;
    }

    message.maj = maj[0] > maj[1] ? ATTACK : RETREAT;
    message.mult = maj[0] > maj[1] ? maj[0] : maj[1];

    return message;
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

    for(int k = 0; k <= no_faulty_generals; k++){
        if(is_faulty == 1){
            decision = (rand() % 100) > 50 ? ATTACK : RETREAT;
        }

        // send everybody to everybody. 
        MPI_Allgather(&decision, 1, MPI_INT,  
            decision_array, 1, MPI_INT, MPI_COMM_WORLD);

        ByzantineMessage computed_message = compute_byzantine_message(decision_array, world_size);
        // printf("pid: %d: decision: %d maj: %d mult: %d\n", my_pid, decision, computed_message.maj, computed_message.mult);

        int king_maj = 0;
        // round 2k+1
        if(k == my_pid){
            king_maj = computed_message.maj;
            printf("I am the king and my maj is: %d\n", king_maj);
        }

        MPI_Bcast(&king_maj, 1, MPI_INT, k, MPI_COMM_WORLD);
        // printf("I am pid: %d and king's mult is %d\n", my_pid, king_maj);

        if(computed_message.mult > (world_size / 2) + no_faulty_generals){
            decision = computed_message.maj;
        } else {
            decision = king_maj;
        }
    }

    if(decision == ATTACK){
        printf("pid: %d: my FINAL decision is ATTACK\n", my_pid);
    } else {
        printf("pid: %d: my FINAL decision is RETREAT\n", my_pid);
    }

    MPI_Finalize();
    return 0;
}
