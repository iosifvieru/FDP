#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>

/*
    A ------------ B
    | \            /
    |  \         /
    |    \     /
    |       C
    |   /     \
    D/         \      
                E

    PROCESS             NEIGHBOURS
    A  (0)                 B,D,C
    B  (1)                 C,A
    C  (2)                 A,E,D,B
    D  (3)                 C,A
    E  (4)                 C   

    nnodes = 5
    index = 3, 5, 9, 11, 12
    edges = 1, 2, 3, 2, 0, 0, 4, 3, 1, 2, 0, 2
*/

#define ROOT_PID 2

/* 
                                0       1
msg structure: int[2] -----> [type, payload]
msg types:
    0 -> message
    1 -> parent
    2 -> already
*/
#define TYPE_MESSAGE 0
#define TYPE_PARENT 1
#define TYPE_ALREADY 2

int main(int argc, char** argv){
    int my_pid, world_size;

    int nnodes = 5;
    int index[] = {3, 5, 9, 11, 12};
    int edges[] = {
        1, 2, 3,
        2, 0,
        0, 4, 3, 1,
        2, 0,
        2
    };
    
    int msg[2] = {TYPE_MESSAGE, 0};
    int recvbuf[2];

    int parent = -1;
    int children = 0, others = 0;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    if(world_size != 5){
        fprintf(stderr, "Use -np 5 when running the program. \n");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);

    MPI_Status status;

    MPI_Comm tree_comm;
    MPI_Graph_create(MPI_COMM_WORLD, nnodes, index, edges, 0, &tree_comm);

    int no_neighbours;
    MPI_Graph_neighbors_count(tree_comm, my_pid, &no_neighbours);

    int *my_neighbours = (int*) malloc(no_neighbours * sizeof(int));
    if(my_neighbours == NULL){
        printf("Cannot initialize memory for my_neighbours.\n");
        MPI_Finalize();
        exit(EXIT_FAILURE);
    }

    MPI_Graph_neighbors(tree_comm, my_pid, no_neighbours, my_neighbours);

    // root sends to everybody
    if(my_pid == ROOT_PID){
        msg[0] = TYPE_MESSAGE;
        msg[1] = 8888; // some payload

        for(int i = 0; i < no_neighbours; i++){
            // printf("pid: ROOT - sending [%d %d] to %d\n", msg[0], msg[1], my_neighbours[i]);
            MPI_Send(msg, 2, MPI_INT, my_neighbours[i], 0, tree_comm);
        }
        parent = ROOT_PID;

        printf("i am %d and my parent is %d\n", my_pid, parent);
    }

    int running = 1;
    while(running){
        // wait for message
        MPI_Recv(recvbuf, 2, MPI_INT, MPI_ANY_SOURCE, 0, tree_comm, &status);
        
        int sender = status.MPI_SOURCE;
        // printf("pid: %d - got [%d %d] from %d\n", my_pid, recvbuf[0], recvbuf[1], sender);
        
        int expected = (my_pid == ROOT_PID) ? no_neighbours : no_neighbours - 1;
        
        switch(recvbuf[0]){
            // i recv message
            case 0:
                if(parent == -1){
                    parent = sender;
                    
                    printf("i am %d and my parent is %d\n", my_pid, parent);

                    int parent_msg[2] = {TYPE_PARENT, -1};
                    MPI_Send(parent_msg, 2, MPI_INT, parent, 0, tree_comm);

                    for(int i = 0; i < no_neighbours; i++){
                        int neighbour = my_neighbours[i];
                        if(neighbour == parent) continue;
                        
                        MPI_Send(recvbuf, 2, MPI_INT, neighbour, 0, tree_comm);
                    }

                    if (expected == 0) {
                        printf("pid: %d terminating (leaf)\n", my_pid);
                        running = 0;
                    }

                } else {
                    int already[2] = {TYPE_ALREADY, -1};
                    MPI_Send(already, 2, MPI_INT, sender, 0, tree_comm);
                }
                break;
            case 1:
                children++;

                if(children + others == expected){
                    running = 0;
                }
                break;
            case 2:
                others++;
                
                if(children + others == expected){
                    running = 0;
                }
                break;
        }
        // printf("pid: %d, children = %d, others = %d, no_neighbours = %d \n", my_pid, children, others, no_neighbours);
    }

    MPI_Finalize();
    return 0;
}
