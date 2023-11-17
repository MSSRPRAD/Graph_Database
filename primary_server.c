#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <errno.h>
#include <pthread.h>
#include "graphdb_structs.h"
#include "graphdb_utils.h"
#include "graphdb_graph_functions.h"

#define MAX_SIZE 1024
#define SHARED_MEMORY_SIZE 1024

int main(int argc, char *argv[])
{
	// Define Connection
	key_t key;
	int msg_id;

	// Create Shared Message Queue
	key = ftok("progfile", 65);
	msg_id = msgget(key, 0666);

	// Error Handling
	if (msg_id == -1)
	{
		perror("Server could not create message queue");
		exit(1);
	}

	while (true)
	{
		message m;
		// Receive Message
		int fetchRes = msgrcv(msg_id, &m, sizeof(m), ToPrimaryServer, 0);

		// Error Handling
		if (fetchRes == -1)
		{
			perror("Server could not receive message");
			exit(1);
		}

		payload p = m.p;
		pthread_t thread;
		pthread_attr_t thread_attr;
		pthread_attr_init(&thread_attr);

		int pthread_create(thread, thread_attr, HandleRequest, p, m, msg_id);
		pthread_join(thread, NULL);
	}
}

int HandleRequest(void *p, message m, int msg_id)
{
	payload load = *(payload *)p;

	int sequenceNumber = load.sequenceNumber;
	int operationNumber = load.operationNumber;
	char GraphFileName[256] = load.payload;

	int shared_memory_id;
	shared_memory_id = shmget(sequenceNumber, SHARED_MEMORY_SIZE, 0666);
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

	int nodes = atoi(shared_memory[0]);
	char *returnStr;

	if (operationNumber == 1)
	{
		FILE *fp = fopen(GraphFileName, "w");
		if (fp == NULL)
		{
			printf("Error opening file\n");
			exit(0);
		}
		fprintf(fp, "%d\n", nodes);
		for (int i = 0; i < nodes; i++)
		{
			for (int j = 0; j < nodes; j++)
				fprintf(fp, "%c ", shared_memory[(i * nodes + j + 1)]);
			fprintf(fp, "\n");
		}
		fclose(fp);
		strcpy(returnStr, "File Successfully Added");
	}
	else if (operationNumber == 2)
	{
		FILE *fp = fopen(GraphFileName, "w+");
		if (fp == NULL)
		{
			printf("Error opening file\n");
			exit(0);
		}
		fprintf(fp, "%d\n", nodes);
		for (int i = 0; i < nodes; i++)
		{
			for (int j = 0; j < nodes; j++)
				fprintf(fp, "%c ", shared_memory[(i * nodes + j + 1)]);
			fprintf(fp, "\n");
		}
		fclose(fp);
		strcpy(returnStr, "File Successfully Modified");
	}
	else
	{
		printf("Invalid Operation Number\n");
		exit(0);
	}

	// Send Message
	strcpy(load.payload, returnStr);
	m.p = load;

	// Send To Load balancer
	m.MessageType = ToLoadReceiver;
	int sendRes = msgsnd(msg_id, &m, sizeof(m), 0);

	// Send To Client
	m.MessageType = ToClient;
	int sendRes = msgsnd(msg_id, &m, sizeof(m), 0);

	return 0;
}