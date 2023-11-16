#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define MAX_SIZE 1024
#define SHARED_MEMORY_SIZE 1024

// Define a structure for the message
typedef struct message_buffer
{
    long type;
    char text[100];
} message;

int main()
{
    long client_id;

    // Define Connection
    key_t key;
    int msgid;
    msgid = msgget(key, 0666);

    // Get Shared Message Queue
    key = ftok("progfile", 65);
    key_t shared_memory_key = ftok("shmfile", 65);
    if (msgid == -1)
    {
        perror("Client could not get message queue");
        exit(1);
    }

    while (1)
    {
        message m;
        strcpy(m.text, "");

        int shared_memory_id;
        shared_memory_id = shmget(shared_memory_key, SHARED_MEMORY_SIZE, IPC_CREAT | 0666);
        if (shared_memory_id == -1)
        {
            perror("shmget");
            exit(0);
        }
        char *shared_memory = shmat(shared_memory_id, NULL, 0);
        // CHECK
        if (shared_memory == (char *)-1)
        {
            perror("Error attaching shared memory");
            exit(EXIT_FAILURE);
        }

        // Display menu options
        printf("1. Add a new graph to the database\n");
        printf("2. Modify an existing graph of the database\n");
        printf("3. Perform DFS on an existing graph of the database\n");
        printf("4. Perform BFS on an existing graph of the database\n");

        // int choice;
        // scanf("%d", &choice);

        // Get user input
        char sequence_number[3];
        char operation_number;
        char graph_file_name[MAX_SIZE];
        printf("Enter Sequence Number: ");
        scanf("%s", &m.text[0]);
        // scanf("%d", &sequence_number);
        if (m.text[1] == '\0')
        {
            m.text[1] = ' ';
            m.text[2] = ' ';
        }
        if (m.text[2] == '\0')
            m.text[2] = ' ';

        printf("Enter Operation Number: ");
        scanf("%s", &m.text[3]);
        m.text[4] = ' ';
        printf("Enter Graph File Name: ");
        scanf("%s", &m.text[5]);
        // printf("%s",)

        // Create a shared memory segment for the request

        operation_number = m.text[3];
        // switch (m.text[2])
        // {
        // case 1:
        //     strcpy(m.text, "1");
        //     int num_nodes;
        //     printf("Enter number of nodes of the graph: ");
        //     scanf("%d", &num_nodes);
        //     int adjacency_matrix[num_nodes][num_nodes];

        //     printf("Enter adjacency matrix: ");
        //     for(int i=0;i<num_nodes;i++)
        //     {
        //         for(int j=0;j<num_nodes;j++)
        //             scanf("%d", &adjacency_matrix[i][j]);
        //     }

        //     sprintf(shared_memory, "%d %d", num_nodes, adjacency_matrix);
        //     break;
        // case 2:
        //     strcpy(m.text, "2");
        //     break;
        // case 3:
        //     strcpy(m.text, "3");
        //     break;
        // case 4:
        //     // Exit Client Process
        //     exit(0);
        // default:
        //     printf("Please Choose a valid option");
        //     flag = false;
        //     NL;
        //     break;
        // }
        // write operation
        if (operation_number == '1' || operation_number == '2')
        {
            // For write operations, prompt for additional information
            int num_nodes;
            printf("Enter number of nodes of the graph: ");
            scanf("%d", &num_nodes);
            // Dynamically allocate memory for the adjacency matrix
            // int adjacency_matrix[num_nodes][num_nodes];
            int **adjacency_matrix = malloc(num_nodes * sizeof(int *));
            for (int i = 0; i < num_nodes; i++)
            {
                adjacency_matrix[i] = malloc(num_nodes * sizeof(int));
            }

            printf("Enter adjacency matrix: ");
            for (int i = 0; i < num_nodes; i++)
            {
                for (int j = 0; j < num_nodes; j++)
                    scanf("%d", &adjacency_matrix[i][j]);
            }
            memcpy(shared_memory, adjacency_matrix, sizeof(adjacency_matrix));

            sprintf(shared_memory, "%d\n %d", num_nodes, adjacency_matrix);
            for (int i = 0; i < num_nodes; i++)
            {
                free(adjacency_matrix[i]);
            }
            free(adjacency_matrix);
        }

        else if (operation_number == '3' || operation_number == '4')
        {
            // For read operations, prompt for the starting vertex
            int starting_vertex;
            printf("Enter starting vertex: ");
            scanf("%d", &starting_vertex);
            // Write to shared memory
            sprintf(shared_memory, "%d", starting_vertex);
        }

        // Send the request to the load balancer via the message queue
        // message msg;
        // msg.type = 1;  // Message type
        // sprintf(msg.text, "%s %s %s", sequence_number, operation_number, graph_file_name);

        // Wait for the server's response
        // msgrcv(key, &msg, sizeof(msg.text), 2, 0);

        // Process the response (this part needs to be implemented based on your server's response format)
        // printf("Received response: %s\n", msg.text);

        // Send Message
        int sendRes = msgsnd(msgid, &m, sizeof(m), 0);

        // Error Handling
        if (sendRes == -1)
        {
            perror("Message could not be sent by client");
            exit(1);
        }

        printf("Sent \"%s\"", &m.text);

        // Detach the shared memory segment
        shmdt(shared_memory);
        shmctl(shared_memory_id, IPC_RMID, NULL);
    }

    return 0;
}
