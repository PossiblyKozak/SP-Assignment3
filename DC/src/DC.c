/*	FILE			: DC.c
 *	PROJECT			: PROG1970 - Assignment #3
 *	PROGRAMMER		: Alex Kozak and Attila Katona
 *	FIRST VERSION	: 2018-07-14
 *	DESCRIPTION		: The data creator (DC) program’s job is to artificially generate a status condition representing the
 *					  state of the Hoochamacallit machine on your shop’s floor.
 */

#include "../../Common/inc/Common.h"

 // Prototypes
int sendMessage(int queueID, int messageValue);

int main()
{
	key_t queueKey;
	int queueID = 0, messageValue = 0;

	srand((unsigned)time(NULL));						// Initiate the random number generator

	queueKey = ftok(QUEUE_LOCATION, QUEUE_KEY);			// Get a queue key to use for the message que
	if (queueKey == -1) { return 1; }

	queueID = msgget(queueKey, 0);						// Get the queue ID
	while (queueID == -1)								// repeat until a Message Queue is found
	{
		sleep(DEFAULT_SLEEP_LENGTH);
		queueID = msgget(queueKey, IPC_CREAT | 0660);	// If a Message Queue wasn't found, create one
	}

	// Send an initial message to the queue saying it started successfully.
	if ((sendMessage(queueID, EVERYTHING_OKAY)) == -1) { return 1; }

	while ((queueID = msgget(queueKey, 0)) != -1)
	{
		sleep((rand() % 20) + 10);						// Random number generated from 10 to 30 for sleep in seconds
		messageValue = rand() % NUMBER_OF_MESSAGES;
		if (messageValue == MACHINE_OFFLINE)
		{
			if (sendMessage(queueID, messageValue) == -1) { return 1; }
			return 0;
		}
		if (sendMessage(queueID, messageValue) == -1) { return 1; }
	}
	return 0;
}

int sendMessage(int queueID, int messageValue)
{
	// FUNCTION : send_message
	//
	// DESCRIPTION : This function will send a message(int value) to the messageQ queue.
	//
	// PARAMETERS :	int queueID : Holds the ID number of the queue.
	//				int messageValue : The number for which the reader will know the message.
	//				
	// RETURNS : -1 if there is an error
	//			 Not -1 if there is no error

	QueueMessage messageToSend;
	messageToSend.pID = (int)getpid();
	messageToSend.messageValue = messageValue;

	return msgsnd(queueID, (void *)&messageToSend, sizeof(QueueMessage) - sizeof(long), 0);
}