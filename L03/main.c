// Hirschberg Sinclair (HS) algorithm for leader election on networks based on ring communication topology
#include <mpi/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Message tags
#define TAG_PROBE 0
#define TAG_REPLY 1
#define TAG_LEADER 2

#define MESSAGE_SIZE 4

static inline int pow2(int k){
    return 1 << k; 
}

void forward_probe(int destination, int uuid, int k, int d){
    int forward_msg[MESSAGE_SIZE] = {TAG_PROBE, uuid, k, d+1};
    MPI_Send(forward_msg, MESSAGE_SIZE, MPI_INT, destination, 0, MPI_COMM_WORLD);
}

void reply_probe(int destination, int uuid, int k){
    int reply_msg[MESSAGE_SIZE] = {TAG_REPLY, uuid, k, 0};
    MPI_Send(reply_msg, MESSAGE_SIZE, MPI_INT, destination, 0, MPI_COMM_WORLD);
}

void send_probe(int destination, int uuid, int k, int d){
    int msg[MESSAGE_SIZE] = {TAG_PROBE, uuid, k, d};
    MPI_Send(msg, MESSAGE_SIZE, MPI_INT, destination, 0, MPI_COMM_WORLD);
}

int main(int argc, char** argv){
    MPI_Init(&argc, &argv);

    int my_pid, world_size, UUID;
    int left, right; // for neighbours
    int running = 1;

    int k = 0; // current phase
    int recv_reply_left = 0;
    int recv_reply_right = 0;

    MPI_Status status;

    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_pid);

    srand(time(NULL) + my_pid);
    
    UUID = rand() % 100;

    left = (my_pid - 1 + world_size) % world_size;
    right = (my_pid + 1) % world_size;

    printf("pid: %d, UUID: %d, left: %d, right: %d\n", my_pid, UUID, left, right);

    int recvbuf[MESSAGE_SIZE];

    send_probe(left, UUID, k, 1);
    send_probe(right, UUID, k, 1);

    while(running){
        // waiting for messages
        MPI_Recv(recvbuf, MESSAGE_SIZE, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);
        int source = status.MPI_SOURCE;

        // message format: {TYPE, UUID, k, d}
        printf("pid: %d, MESSAGE: [%d %d %d %d]\n", my_pid, recvbuf[0], recvbuf[1], recvbuf[2], recvbuf[3]);

        int recv_type = recvbuf[0];
        int recv_uuid = recvbuf[1];
        int recv_k = recvbuf[2];
        int recv_d = recvbuf[3];

        switch(recv_type){
            case TAG_PROBE:
                // if i received my own UUID send a leader message ( i am the leader )
                if(recv_uuid == UUID){
                    int leader_msg[MESSAGE_SIZE] = {TAG_LEADER, UUID, 0, 0};
                    MPI_Send(leader_msg, MESSAGE_SIZE, MPI_INT, left, 0, MPI_COMM_WORLD);
                    
                    printf("pid: %d I am the leader UUID: %d !!\n", my_pid, UUID);
                    // running = 0;
                }

                if((recv_uuid > UUID) && recv_d < pow2(recv_k)){
                    if(source == left){
                        forward_probe(right, recv_uuid, recv_k, recv_d);
                    } else {
                        forward_probe(left, recv_uuid, recv_k, recv_d);
                    }
                }

                if((recv_uuid > UUID) && recv_d >= pow2(recv_k)){
                    if(source == left){
                        reply_probe(left, recv_uuid, recv_k);
                    } else {
                        reply_probe(right, recv_uuid, recv_k);
                    }
                }

                // if j < id message is swallowed
                break;
            case TAG_REPLY:
                if(recv_uuid != UUID){
                    if(source == left){
                        forward_probe(right, recv_uuid, recv_k, 0);
                    } else {
                        forward_probe(left, recv_uuid, recv_k, 0);
                    }
                } else {
                    if(source == left){
                        recv_reply_left = 1;
                    } else {
                        recv_reply_right = 1;
                    }

                    if(recv_reply_left == 1 && recv_reply_right == 1){
                        k += 1; // move to the next phase.
                        recv_reply_left = 0;
                        recv_reply_right = 0;

                        send_probe(left, UUID, k, 1);
                        send_probe(right, UUID, k, 1);
                    }
                }
            
                break;
            case TAG_LEADER:
                if(recv_uuid != UUID){
                    MPI_Send(recvbuf, MESSAGE_SIZE, MPI_INT, left, 0, MPI_COMM_WORLD);
                }
                printf("pid: %d UUID: %d, Leader is %d.\n", my_pid, UUID, recv_uuid);
                running = 0;
                break;
        }
    }

    MPI_Finalize();
    return 0;
}
