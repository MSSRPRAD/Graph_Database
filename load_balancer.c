#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <pthread.h>
#include "graphdb_structs.h"

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

		int sequenceNumber = p.sequenceNumber;
		int operationNumber = p.operationNumber;
		char GraphFileName[256] = p.payload;

		if (operationNumber == 1 || operationNumber == 2)
		{
			m.MessageType = ToPrimaryServer;
		}
		else if (sequenceNumber % 2 == 0)
			m.MessageType = ToSecondaryServer1;
		else
			m.MessageType = ToSecondaryServer2;

		int sendRes = msgsnd(msg_id, &m, sizeof(m), 0);
	}
}
