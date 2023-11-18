#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <stdbool.h>

typedef struct Payload
{
    int sequence_number;
    int operation_number;
    char graph_file_name[1024];
    int result[50];
} Payload;

// Message Structure Definition
typedef struct Message
{
    long mtype;
    Payload payload;
} Message;

// typedef struct message
// {
//     long mtype;
//     int mtext[50];
// } message;

int main() {
    // Define Connection
    key_t key;
    int msgid;

    // Get Shared Message Queue
    key = ftok("msgq", 65);
    msgid = msgget(key, 0666);


        key_t shared_memory_key = ftok("shmfile", 65);
    if (msgid == -1)
    {
        perror("Client could not get message queue");
        exit(1);
    }

    while (true) {
        Message m;

        // Set the MessageType
        m.mtype = 1;

        // int shared_memory_id;


        // Display menu options
        printf("\n\n");
        printf("1. Add a new graph to the database\n");
        printf("2. Modify an existing graph of the database\n");
        printf("3. Perform DFS on an existing graph of the database\n");
        printf("4. Perform BFS on an existing graph of the database\n");
        printf("\n\n");

        // Get and store user input
        int sequence_number;
        int operation_number;
        char graph_file_name[1024];

        printf("Enter Sequence Number: ");
        scanf("%d", &sequence_number);
        printf("Enter Operation Number: ");
        scanf("%d", &operation_number);
        printf("Enter Graph File Name: ");
        scanf("%s", graph_file_name);

        Payload p;
        p.operation_number = operation_number;
        strcpy(p.graph_file_name, graph_file_name);
        p.sequence_number = sequence_number;
        m.payload = p;

//         // Create a shared memory segment for the request
//         shared_memory_id = shmget(m.payload.sequenceNumber, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
//         if(shared_memory_id==-1)
//         {
//             perror("shmget");
//             exit(0);
//         }

//         char *shared_memory = shmat(shared_memory_id, NULL, 0);
//         //CHECK
//         if (shared_memory == (char*)-1) {
//             perror("Error attaching shared memory");
//             exit(EXIT_FAILURE);
//         }


        int sendRes = msgsnd(msgid, &m, sizeof(m.payload), 0);

        // Error Handling
        if (sendRes == -1)
        {
            perror("Message could not be sent by client");
            exit(1);
        }

        printf(
            "\nSent message with: \nMessage Type: %d\nSequence Number:%d \nOperation Number:%d \nFile Name:%s\n",m.mtype ,m.payload.sequence_number, m.payload.operation_number, m.payload.graph_file_name);

        //write operation
        if (operation_number == 1 || operation_number == 2) {

            // For write operations, prompt for additional information
            int num_nodes;
            printf("Enter number of nodes of the graph: ");
            scanf("%d", &num_nodes);

            // Dynamically allocate memory for the adjacency matrix
            // int adjacency_matrix[num_nodes][num_nodes];
            int **adjacency_matrix = malloc(num_nodes * sizeof(int *));
            for (int i = 0; i < num_nodes; i++) {
                adjacency_matrix[i] = malloc(num_nodes * sizeof(int));
            }

            printf("Enter adjacency matrix: ");
            for (int i = 0; i < num_nodes; i++) {
                for (int j = 0; j < num_nodes; j++)
                    scanf("%d", &adjacency_matrix[i][j]);
            }
//            memcpy(shared_memory,adjacency_matrix,sizeof(adjacency_matrix));
//assuming shared memory has adequate space

            //sprintf(shared_memory, "%d", num_nodes);
            //shared_memory+=4;
            for(int i=0;i<num_nodes;i++)
            {
                for(int j=0;j<num_nodes;j++){
                    //sprintf(shared_memory,"%d", adjacency_matrix[i][j]);
                    //shared_memory+=4;
                }

            }
            for (int i = 0; i < num_nodes; i++) {
                free(adjacency_matrix[i]);
            }
            free(adjacency_matrix);
        }

        else if (operation_number == 3 || operation_number == 4) {
            // For read operations, prompt for the starting vertex
            int starting_vertex;
            printf("Enter starting vertex: ");
            scanf("%d", &starting_vertex);
            // Write to shared memory
            //sprintf(shared_memory, "%d", starting_vertex);
            
        }
        // {
        //     // Get Reply
        Message reply;
		// Receive Message;
		int fetchRes = msgrcv(msgid, &reply, sizeof(m.payload), 5, 0);

		// Error Handling
		if (fetchRes == -1)
		{
			perror("Client could not receive message");
			exit(1);
		}

		 printf(
            "\nRecieved: \nSequence Number:%d for\nOperation Number:%d",reply.payload.sequence_number, reply.payload.operation_number);
            for(int i=0;i<reply.payload.sequence_number;i++)
                printf("%d", reply.payload.result);

        //     //If msgtyp is equal to zero, the first message on the queue is received.
        //     //If msgtyp is greater than 0, the first message of type, msgtyp, is received.
        //     //If msgtyp is less than 0, the first message of the lowest type that is less than or equal to the absolute value of msgtyp is received.

        //     // Error Handling
        //     if (fetchRes == -1)
        //     {
        //         perror("Message could not be recieved by client");
        //         exit(1);
        //     }
        //     printf("Reply from Server: \"%s\"", reply.payload.payload);
        // }

        // // Detach the shared memory segment
        // shmdt(shared_memory);
        // shmctl(shared_memory_id, IPC_RMID, NULL);
    }

    return 0;
}
