#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>
#include <limits.h>

typedef struct Payload
{
	int sequence_number;
	int operation_number;
	char graph_file_name[1024];
	int result[1024];
} Payload;

// Message Structure Definition
typedef struct Message
{
	long mtype;
	Payload payload;
} Message;

// pthread_mutex_t writeLock;
// pthread_mutex_t readLock;
// int readerCount = 0;

int main(int argc, char *argv[])
{

	// Define Connection
	key_t key;
	int msg_id;

	// Create Shared Message Queuep
	key = ftok("msgq", 65);
	msg_id = msgget(key, 0666 | IPC_CREAT);

	// Error Handling
	if (msg_id == -1)
	{
		perror("Server could not create message queue");
		exit(1);
	}

	printf("Load Balancer created Message Queue.\n\n");

	// // pthread_mutex_init(&writeLock, NULL);
	// // pthread_mutex_init(&readLock, NULL);

	while (true)
	{
		Message m;

		// Receive Message;
		int fetchRes = msgrcv(msg_id, &m, sizeof(m.payload), 1, 0);

		// Error Handling
		if (fetchRes == -1)
		{
			msgctl(msg_id, IPC_RMID, NULL);
			perror("Load Balancer could not receive message");
			exit(1);
		}

		printf(
			"\nRecieved message with: \nMessage Type: %d\nSequence Number:%d \nOperation Number:%d \nFile Name:%s\n", m.mtype, m.payload.sequence_number, m.payload.operation_number, m.payload.graph_file_name);

		if (m.payload.sequence_number == INT_MAX)
		{
			printf("hi");
			m.mtype = 2;
			int sendRes = msgsnd(msg_id, &m, sizeof(m.payload), 0);

			// Error Handling
			if (sendRes == -1)
			{
				perror("Load Balancer could not send message");
				exit(1);
			}

			m.mtype = 3;
			sendRes = msgsnd(msg_id, &m, sizeof(m.payload), 0);

			// Error Handling
			if (sendRes == -1)
			{
				perror("Load Balancer could not send message");
				exit(1);
			}

			m.mtype = 4;
			sendRes = msgsnd(msg_id, &m, sizeof(m.payload), 0);

			// Error Handling
			if (sendRes == -1)
			{
				perror("Load Balancer could not send message");
				exit(1);
			}

			msgctl(msg_id, IPC_RMID, NULL);
			exit(0);
		}

		if (m.payload.operation_number == 1 || m.payload.operation_number == 2)
		{
			m.mtype = 2;
		}
		else if (m.payload.sequence_number % 2)
		{
			m.mtype = 3;
		}
		else
		{
			m.mtype = 4;
		}

		// Send To Server (Either Primary or Secondary)
		int sendRes = msgsnd(msg_id, &m, sizeof(m.payload), 0);

		// Error Handling
		if (sendRes == -1)
		{
			perror("Load Balancer could not send message");
			exit(1);
		}

		printf(
			"\n Sent message with: \nMessage Type: %d\nSequence Number:%d \nOperation Number:%d \nFile Name:%s\n", m.mtype, m.payload.sequence_number, m.payload.operation_number, m.payload.graph_file_name);

		// 	// Payload p = m.payload;

		// 	// pthread_t thread;
		// 	// pthread_attr_t thread_attr;
		// 	// pthread_attr_init(&thread_attr);

		// 	// struct ThreadParams params = {
		// 	// 	.m = m,
		// 	// 	.msg_id = msg_id
		// 	// };

		// 	// int pthread_create(thread, thread_attr, HandleRequest, (void *)&params);
		// 	// pthread_join(thread, NULL);
	}

	return 0;
}

// int HandleRequest(void *params)
// {
// 	struct ThreadParams *threadParams = (struct ThreadParams *)params;

// 	Message m = threadParams->m;
// 	int msg_id = threadParams->msg_id;
// 	// int reader;

// 	// if (operationNumber == 1 || operationNumber == 2)
// 	// {
// 	// 	reader = 0;
// 	// 	pthread_mutex_lock(&writeLock);
// 	// 	m.MessageType = ToPrimaryServer;
// 	// }
// 	// else
// 	// {
// 	// 	reader = 1;
// 	// 	pthread_mutex_lock(&readLock);
// 	// 	readerCount++;
// 	// 	if (readerCount == 1)
// 	// 		pthread_mutex_lock(&writeLock);
// 	// 	pthread_mutex_unlock(&readLock);

// 	// 	if (sequenceNumber % 2 == 0)
// 	// 		m.MessageType = ToSecondaryServer1;
// 	// 	else
// 	// 		m.MessageType = ToSecondaryServer2;
// 	// }

// 	m.MessageType = ToSecondaryServer1;

// 	// Send To Server (Either Primary or Secondary)
// 	int sendRes = msgsnd(msg_id, &m, sizeof(m), 0);
// 	// Error Handling
// 	if (sendRes == -1)
// 	{
// 		perror("Load Balancer could not send message");
// 		exit(1);
// 	}

// 	// // Receive Message
// 	// int fetchRes = msgrcv(msg_id, &m, sizeof(m), ToLoadReceiver, 0);

// 	// // Error Handling
// 	// if (fetchRes == -1)
// 	// {
// 	// 	perror("Server could not receive message");
// 	// 	exit(1);
// 	// }

// 	// if (!reader)
// 	// 	pthread_mutex_unlock(&writeLock);
// 	// else
// 	// {
// 	// 	pthread_mutex_lock(&readLock);
// 	// 	readerCount--;
// 	// 	if (readerCount == 0)
// 	// 		pthread_mutex_unlock(&writeLock);
// 	// 	pthread_mutex_unlock(&readLock);
// 	// }

// 	return 0;
// }