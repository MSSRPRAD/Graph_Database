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
#include "graphdb_structs.h"

pthread_mutex_t writeLock;
pthread_mutex_t readLock;
int readerCount = 0;

int main(int argc, char *argv[])
{
	// Define Connection
	key_t key;
	int msg_id;

	// Create Shared Message Queue
	key = ftok("progfile", 65);
	msg_id = msgget(key, 0666 | IPC_CREAT);

	// Error Handling
	if (msg_id == -1)
	{
		perror("Server could not create message queue");
		exit(1);
	}

	pthread_mutex_init(&writeLock, NULL);
	pthread_mutex_init(&readLock, NULL);

	while (true)
	{
		message m;
		// Receive Message
		int fetchRes = msgrcv(msg_id, &m, sizeof(m), ToLoadReceiver, 0);

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

	int reader;

	if (operationNumber == 1 || operationNumber == 2)
	{
		reader = 0;
		pthread_mutex_lock(&writeLock);
		m.MessageType = ToPrimaryServer;
	}
	else
	{
		reader = 1;
		pthread_mutex_lock(&readLock);
		readerCount++;
		if (readerCount == 1)
			pthread_mutex_lock(&writeLock);
		pthread_mutex_unlock(&readLock);

		if (sequenceNumber % 2 == 0)
			m.MessageType = ToSecondaryServer1;
		else
			m.MessageType = ToSecondaryServer2;
	}

	// Send To Server (Either Primary or Secondary)
	int sendRes = msgsnd(msg_id, &m, sizeof(m), 0);

	// Receive Message
	int fetchRes = msgrcv(msg_id, &m, sizeof(m), ToLoadReceiver, 0);

	// Error Handling
	if (fetchRes == -1)
	{
		perror("Server could not receive message");
		exit(1);
	}

	if (!reader)
		pthread_mutex_unlock(&writeLock);
	else
	{
		pthread_mutex_lock(&readLock);
		readerCount--;
		if (readerCount == 0)
			pthread_mutex_unlock(&writeLock);
		pthread_mutex_unlock(&readLock);
	}

	return 0;
}