//FILE :DR.c
//PROJECT : PROG1970 - Assignment #3
//PROGRAMMER : Alex Kozak and Attila Katona
//FIRST VERSION : 2018-06-16
//DESCRIPTION :	The data reader (DR) program’s purpose is to monitor its incoming message queue for the varying
//				statuses of the different Hoochamacallit machines on the shop floor. It will keep track of the number
//				of different machines present in the system, and each new message it gets from a machine, it reports
//				it findings to a data monitoring log file. The DR application is solely responsible for creating
//				its incoming message queue and when it detects that all of its feeding machines have gone off-line, the
//				DR application will free up the message queue, release its shared memory and the DR application will
//				terminate.
#include "../../Common/inc/Common.h"

void killIdleProcesses(MasterList *ml, FILE* logFile);
void removeDCByPID(MasterList *ml, int pID, FILE* logFile);
void interpretMessageCode(MasterList *ml, int messageValue, int pID, int i, FILE* logFile);
void printProcesses(MasterList *ml);
void isCurrentlyActivePID(int *i, bool *isRecognized, MasterList* ml, int pID);
void printCurrentTime(FILE* logFile);

int main(int argc, char *argv[])
{
	int messageID, sharedMemoryID;							// Message ID, Shared Memory ID
	key_t messageKey, shmKey;								// Message Key, Shared Memory Key
	int sizeOfData = sizeof(QueueMessage) - sizeof(long);	// The size of the message chunk to be received
	MasterList *masterList;									// The pointer to the MasterList stored in shared memory
	bool isQueueValid = true;								// Boolean for the Queue check loop

	FILE *logFile = fopen(DR_LOG_FILE_PATH, "w");			// Open the Log File

	QueueMessage receivedMessage;

	// Get the Message Queue token, return if not successful
	if ((messageKey = ftok(QUEUE_LOCATION, QUEUE_KEY)) == -1) { return 1; }

	// Create the Message Queue, return if not successful
	if ((messageID = msgget(messageKey, IPC_CREAT | 0660)) == -1) { return 2; }

	// Get the Shared Memory token, return if not successful
	if ((shmKey = ftok(SHARED_MEM_LOCATION, SHARED_MEM_KEY)) == -1) { return 3; }

	// Check to see if Shared Memory already exists
	if (sharedMemoryID = shmget(shmKey, sizeof(MasterList), 0) == -1)
	{
		// If the Shared Memory doesn't exist, create it. If that's also not successful, return
		if ((sharedMemoryID = shmget(shmKey, sizeof(MasterList), (IPC_CREAT | 0660))) == -1) { return 4; }
	}

	// Generate a pointer to the master list from the Shared Memory, return if not successful
	if ((masterList = (MasterList*)shmat(sharedMemoryID, NULL, 0)) == -1) { return 5; }

	// Set the known values of the Master List
	masterList->messageQueueID = messageID;
	masterList->numberOfDCs = 0;

	// give some time before starting so some DCs can be started
	sleep(10);

	while (isQueueValid) // run through while there the queue exists and there are one or more active DCs
	{
		// Grab the front message in the queue, if there arent any messages, pass through
		if (msgrcv(messageID, &receivedMessage, sizeOfData, 0, IPC_NOWAIT) != -1)
		{
			bool isRecognized = false;	// boolean for if the pID has been seen before
			int masterListIndex = 0;	// The index the 

			isCurrentlyActivePID(&masterListIndex, &isRecognized, masterList, receivedMessage.pID);
			if (!isRecognized && masterList->numberOfDCs != MAX_DC_ROLES && receivedMessage.messageValue == EVERYTHING_OKAY)
			{
				printCurrentTime(logFile);
				fprintf(logFile, "DC-%.2d [%d] added to the master list - NEW DC - Status 0 (Everything is OK)\n", masterListIndex + 1, receivedMessage.pID);

				masterList->dc[masterListIndex].dcProcessID = receivedMessage.pID;
				masterList->dc[masterListIndex].lastTimeHeardFrom = (int)time(NULL);
				masterList->numberOfDCs++;

				printProcesses(masterList);
			}
			else
			{
				interpretMessageCode(masterList, receivedMessage.messageValue, receivedMessage.pID, masterListIndex, logFile);
			}

		}
		else { isQueueValid = false; }
		if (masterList->numberOfDCs != 0) 
		{ 
			isQueueValid = ((messageID = msgget(messageKey, 0)) != -1); 
		}

		killIdleProcesses(masterList, logFile);
		printProcesses(masterList);
		usleep(1500000);
	} 

	// Print final message in the log file and close the file
	fprintf(logFile, "All DCs have gone offline or terminated - DR TERMINATING");
	fclose(logFile);

	// Close the Message Queue and the Shared Memory
	msgctl(messageID, IPC_RMID, NULL);
	shmctl(sharedMemoryID, IPC_RMID, NULL);
	return 0;
}

void isCurrentlyActivePID(int *masterListIndex, bool *isRecognized, MasterList* ml, int pID)
{
	for (; *masterListIndex < MAX_DC_ROLES && !(*isRecognized); (*masterListIndex)++)
	{
		if (ml->dc[*masterListIndex].dcProcessID != NULL)
		{
			if (ml->dc[*masterListIndex].dcProcessID == pID)
			{
				ml->dc[*masterListIndex].lastTimeHeardFrom = (int)time(NULL);
				*isRecognized = true;
			}
		}
		else
		{
			break;
		}
	}
}
// FUNCTION : printCurrentTime
// DESCRIPTION : This function will generate the time and date formatted to the project standards.
//				 It will then print this time and date to the logfile.
//
// PARAMETERS :	FILE* logfile : A pointer to the log file for logging the different events of the program
//				
// RETURNS : Void
//
void printCurrentTime(FILE* logFile)
{
	time_t currTime;
	struct tm* currTimeInfo;
	char formattedTimeString[28];

	time(&currTime);
	currTimeInfo = localtime(&currTime);

	strftime(formattedTimeString, 28, "[%Y-%m-%d %H:%M:%S]", currTimeInfo);
	fprintf(logFile, "%s : ", formattedTimeString);
}

void interpretMessageCode(MasterList *masterList, int messageValue, int pID, int masterListIndex, FILE* logFile)
{
	printCurrentTime(logFile);
	fprintf(logFile, "DC-%.2d [%d] ", masterListIndex + 1, pID);
	if (messageValue != MACHINE_OFFLINE) { fprintf(logFile, "updated in the master list - MSG RECEIVED - "); }

	switch (messageValue)
	{
	case (EVERYTHING_OKAY):
	{
		fprintf(logFile, "Status 0 (Everything is OK)\n");
		break;
	}
	case (HYDRAULIC_FAILURE):
	{
		fprintf(logFile, "Status 1 (Hydraulic Pressure Failure)\n");
		break;
	}
	case (SAFETY_BUTTON_FAILURE):
	{
		fprintf(logFile, "Status 2 (Safety Button Failure)\n");
		break;
	}
	case (NO_RAW_MATERIAL):
	{
		fprintf(logFile, "Status 3 (No Raw Material in the Process)\n");
		break;
	}
	case (TEMP_OUT_OF_RANGE):
	{
		fprintf(logFile, "Status 4 (Operating Temperatures Out of Range)\n");
		break;
	}
	case (OPERATOR_ERROR):
	{
		fprintf(logFile, "Status 5 (Operator Error)\n");
		break;
	}
	case (MACHINE_OFFLINE):
	{
		fprintf(logFile, "has gone OFFLINE - removing from master-list\n");
		removeDCByPID(masterList, pID, logFile);
		break;
	}
	default:
		fprintf(logFile, "UNKNOWN COMMAND %.2d [%d]\n", masterListIndex + 1, pID);
	}
	printf("\n");
}

void removeDCByPID(MasterList *masterList, int pID, FILE* logFile)
{
	for (int i = 0; i < MAX_DC_ROLES; i++)
	{
		if (masterList->dc[i].dcProcessID == pID)
		{
			memmove(&masterList->dc[i], &masterList->dc[i + 1], sizeof(DCInfo) * (MAX_DC_ROLES - (i + 1)));
		}
	}
	masterList->numberOfDCs--;
}

void killIdleProcesses(MasterList *masterList, FILE* logFile)
{
	for (int i = 0; i < MAX_DC_ROLES && i < masterList->numberOfDCs; i++)
	{
		if (((int)time(NULL) - masterList->dc[i].lastTimeHeardFrom) > 35)
		{
			printCurrentTime(logFile);
			fprintf(logFile, "DC-%.2d [%d] removed from master list - NON-RESPONSIVE\n", i + 1, masterList->dc[i].dcProcessID);
			memmove(&masterList->dc[i], &masterList->dc[i + 1], sizeof(DCInfo) * (MAX_DC_ROLES - (i + 1)));
			masterList->numberOfDCs--;
		}
	}
}

void printProcesses(MasterList *masterList)
{
	for (int i = 0; i < MAX_DC_ROLES && i < masterList->numberOfDCs; i++)
	{
		if (masterList->dc[i].dcProcessID != NULL)
		{
			printf("Process %d: (%d) Last heard from %d seconds ago\n", i, masterList->dc[i].dcProcessID, (int)time(NULL) - masterList->dc[i].lastTimeHeardFrom);
		}
		else
		{
			break;
		}
	}
}