#include "../../Common/inc/Common.h"

int send_message(int queueID, int messageValue)
{
	QueueMessage messageToSend;
	messageToSend.pID = (int)getpid();
	messageToSend.messageValue = messageValue;

	return msgsnd(queueID, (void *)&messageToSend, sizeof(QueueMessage) - sizeof(long), 0);
}

int main()
{
	key_t queueKey;
	int queueID = 0, messageValue = 0;

	srand((unsigned)time(NULL));

	queueKey = ftok(QUEUE_LOCATION, QUEUE_KEY);
	if (queueKey == -1) { return 1; }

	queueID = msgget(queueKey, 0);
	while (queueID == -1)
	{
		sleep(10);
		queueID = msgget(queueKey, IPC_CREAT | 0660);
	}

	if (send_message(queueID, EVERYTHING_OKAY) == -1) { return 1; }

	while ((queueID = msgget(queueKey, 0)) != -1)
	{
		sleep((rand() % 20) + 10);
		messageValue = rand() % 7;
		if (messageValue == MACHINE_OFFLINE)
		{
			if (send_message(queueID, messageValue) == -1) { return 1; }
			return 0;
		}
		if (send_message(queueID, messageValue) == -1) { return 1; }
	}
	return 0;
}