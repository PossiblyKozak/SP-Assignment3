/*	FILE			: DR.c
 *	PROJECT			: PROG1970 - Assignment #3
 *	PROGRAMMER		: Alex Kozak and Attila Katona
 *	FIRST VERSION	: 2018-07-14
 *	DESCRIPTION		: The data reader (DR) program’s purpose is to monitor its incoming message queue for the varying
 *					  statuses of the different Hoochamacallit machines on the shop floor. It will keep track of the number
 *					  of different machines present in the system, and each new message it gets from a machine, it reports
 *					  it findings to a data monitoring log file. The DR application is solely responsible for creating
 *					  its incoming message queue and when it detects that all of its feeding machines have gone off-line, the
 *					  DR application will free up the message queue, release its shared memory and the DR application will
 *					  terminate.
 */

#include "../../Common/inc/Common.h"

 // Prototypes
void interpretMessage(MasterList*, QueueMessage, FILE*);
void interpretMessageCode(MasterList *, QueueMessage, int, FILE*);
int  getSharedMemory(size_t);
int  getMessageQueue();
void isCurrentlyActivePID(int*, bool*, MasterList*, int);
void printCurrentTimeToFile(FILE*);
void removeDCByPID(MasterList*, int);
void killIdleProcesses(MasterList*, FILE*);

int main(int argc, char *argv[])
{
	int messageID, sharedMemoryID;							// Message ID, Shared Memory ID
	int sizeOfData = sizeof(QueueMessage) - sizeof(long);	// The size of the message chunk to be received
	MasterList *masterList;									// The pointer to the MasterList stored in shared memory
	bool isQueueValid = true;								// Boolean for the Queue check loop

	FILE *logFile = fopen(DR_LOG_FILE_PATH, "w");			// Open the Log File

	QueueMessage receivedMessage;

	sharedMemoryID = getSharedMemory(sizeof MasterList);
	messageID = getMessageQueue();

	// if either the Shared Memory or Message Queue creation failed, exit the program with an error code
	if (sharedMemoryID == -1 || messageID == -1) { return 1; }

	// Generate a pointer to the master list from the Shared Memory, return if not successful
	if ((masterList = (MasterList*)shmat(sharedMemoryID, NULL, 0)) == -1) { return 1; }

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
			interpretMessage(masterList, receivedMessage, logFile);
		}
		else
		{
			isQueueValid = false;
		}

		if (masterList->numberOfDCs != 0)
		{
			isQueueValid = ((messageID = msgget(messageKey, 0)) != -1);
		}

		killIdleProcesses(masterList, logFile);	// Check to see if any processes have been idle for more than 35 seconds		
		usleep(1500000);						// Delay for 1.5 seconds
	}

	// Print final message in the log file and close the file
	fprintf(logFile, "All DCs have gone offline or terminated - DR TERMINATING");
	fclose(logFile);

	// Close the Message Queue and the Shared Memory
	msgctl(messageID, IPC_RMID, NULL);
	shmctl(sharedMemoryID, IPC_RMID, NULL);
	return 0;
}

void interpretMessage(MasterList* masterList, QueueMessage receivedMessage, FILE* logFile)
{
	// FUNCTION		: interpretMessage
	// DESCRIPTION	: This function interprets a message from the Message Queue and acts appropriately for 
	//				  the contents of the message
	// PARAMETERS	:	
	//	  MasterList*	masterlist		: The pointer to a struct that holds the shared memory information
	//									  including the number and PIDs of currently running DCs among other things
	//	  QueueMessage	receivedMessage	: The message taken from the Message Queue to be interpreted
	//	  FILE*		    logfile			: A pointer to the log file for logging purposes
	// RETURNS		: 
	//    VOID

	bool isRecognized = false;	// boolean for if the pID has been seen before or if it is new
	int masterListIndex = 0;	// The index the currently selected pID in the masterList's dc array

	// Use this function to determine if the pID is already in the list of active pIDs
	isCurrentlyActivePID(&masterListIndex, &isRecognized, masterList, receivedMessage.pID);

	if (!isRecognized && masterList->numberOfDCs != MAX_DC_ROLES && receivedMessage.messageValue == EVERYTHING_OKAY)
	{
		// This occurs if the pID was not on the list and there are still slots in the DC array, and the
		// message is EVERYTHING_OKAY (0) which is the set first message by the DC program

		// Log the adding of the current item to the active array in the log file
		printCurrentTimeToFile(logFile);
		fprintf(logFile, "DC-%.2d [%d] added to the master list - NEW DC - Status 0 (Everything is OK)\n", masterListIndex + 1, receivedMessage.pID);

		// Update the masterList's active DC array and add one to the active DC count 
		masterList->dc[masterListIndex].dcProcessID = receivedMessage.pID;
		masterList->dc[masterListIndex].lastTimeHeardFrom = (int)time(NULL);
		masterList->numberOfDCs++;
	}
	else
	{
		// Regular message from a known pID, interpret normally
		interpretMessageCode(masterList, receivedMessage, masterListIndex, logFile);
	}
}

void interpretMessageCode(MasterList *masterList, QueueMessage receivedMessage, int masterListIndex, FILE* logFile)
{
	// FUNCTION		: interpretMessageCode
	// DESCRIPTION	: This function interprets a message from the Message Queue and acts appropriately for 
	//				  the contents of the message
	// PARAMETERS	:	
	//	  MasterList*	masterlist		: The pointer to a struct that holds the shared memory information
	//									  including the number and PIDs of currently running DCs among other things
	//	  QueueMessage	receivedMessage	: The message taken from the Message Queue to be interpreted
	//	  int			masterListIndex	: The index of the currently active pID in the masterList list of active DCs
	//	  FILE*		    logfile			: A pointer to the log file for logging purposes
	// RETURNS		: 
	//    VOID

	// Log the header for all message codes
	printCurrentTimeToFile(logFile);
	fprintf(logFile, "DC-%.2d [%d] ", masterListIndex + 1, receivedMessage.pID);

	// print the secondary header for all codes other than MACHINE_OFFLINE
	if (receivedMessage.messageValue != MACHINE_OFFLINE) { fprintf(logFile, "updated in the master list - MSG RECEIVED - "); }

	switch (receivedMessage.messageValue)
	{
		// For each message ID, print the associated string to the log file
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
		removeDCByPID(masterList, receivedMessage.pID);	// Since the machine has gone offline, remove the DC from masterList's list of active DCs
		break;
	}
	default:
		// Should there be some kind of other index received, output UKNOWN COMMAND to the log file
		fprintf(logFile, "UNKNOWN COMMAND %.2d [%d]\n", masterListIndex + 1, pID);
	}
}

int getSharedMemory(size_t size)
{
	// FUNCTION		: getSharedMemory
	// DESCRIPTION	: This function gets/opens a shared memory location given by shmget() using the constants
	//				  called SHARED_MEM_LOCATION and SHARED_MEM_KEY found in Common.h
	// PARAMETERS	:	
	//	  size_t	size	: The desired size for the Shared Memory segment
	// RETURNS		: 
	//    int				: The ID for accessing the Shared Memory location

	key_t sharedMemoryKey;
	int sharedMemoryID = -1;

	// Get the Shared Memory token, continue only if successful
	if ((sharedMemoryKey = ftok(SHARED_MEM_LOCATION, SHARED_MEM_KEY)) != -1)
	{
		// Check to see if Shared Memory already exists
		if (sharedMemoryID = shmget(sharedMemoryKey, size, 0) == -1)
		{
			// Shared Memory doesn't exist yet thus create it
			sharedMemoryID = shmget(sharedMemoryKey, size, (IPC_CREAT | 0660));
		}
	}
	return sharedMemoryID;
}

int getMessageQueue(void)
{
	// FUNCTION		: getMessageQueue
	// DESCRIPTION	: This function gets/opens a Message Queue location given by msgget() using the constants
	//				  called QUEUE_LOCATION and QUEUE_KEY found in Common.h
	// PARAMETERS	:	
	//	  VOID
	// RETURNS		: 
	//    int				: The ID for accessing the Message Queue location

	key_t messageKey;
	int messageID = -1;

	// Get the Message Queue token, return if not successful
	if ((messageKey = ftok(QUEUE_LOCATION, QUEUE_KEY)) == -1)
	{
		// Create a new Message Queue using the generated key
		messageID = msgget(messageKey, IPC_CREAT | 0660);
	}
	return messageID;
}

void isCurrentlyActivePID(int *masterListIndex, bool *isRecognized, MasterList* masterList, int pID)
{
	// FUNCTION		: isCurrentlyActivePID
	// DESCRIPTION	: This function searches through the masterList's list of active DCs and changing the value
	//				  of the masterListIndex and isRecognized pointers to reflect the results of the search
	// PARAMETERS	:	
	//	  int*			masterListIndex	: The pointer to an int used as an iterator to find the index of the pID
	//									  in the masterList's list of active DCs
	//	  bool*			isRecognised	: A pointer to a bool representing if the pID was already in the masterlist's
	//									  list of active DCs or it is a new pID
	//	  MasterList*	masterlist		: The pointer to a struct that holds the shared memory information
	//									  including the number and PIDs of currently running DCs among other things
	//	  int			pID				: The pID to be searched in the msaterList's list of active DCs
	// RETURNS		: 
	//    VOID

	for (; *masterListIndex < MAX_DC_ROLES && !(*isRecognized); (*masterListIndex)++)
	{
		if (masterList->dc[*masterListIndex].dcProcessID != NULL)
		{
			if (masterList->dc[*masterListIndex].dcProcessID == pID)
			{
				// If it has been found in the active list, change the lastTimeHeardFrom value to now
				// and change the isRecognised flag to true to represent that is was previously in the list
				masterList->dc[*masterListIndex].lastTimeHeardFrom = (int)time(NULL);
				*isRecognized = true;
			}
		}
		else { break; }
	}
}

void printCurrentTimeToFile(FILE* logFile)
{
	// FUNCTION		: printCurrentTime
	// DESCRIPTION	: This function will generate the time and date formatted to the project standards.
	//				  It will then print this time and date to the logfile.
	// PARAMETERS	:	
	//	  FILE* logfile		: A pointer to the log file for logging the different events of the program
	// RETURNS		: 
	//    VOID

	time_t currTime;
	struct tm* currTimeInfo;
	char formattedTimeString[28];

	// Get the local time in the tm struct
	time(&currTime);
	currTimeInfo = localtime(&currTime);

	// Format the time to the expected style
	strftime(formattedTimeString, 28, "[%Y-%m-%d %H:%M:%S]", currTimeInfo);

	// Print the resulting string into the given log file
	fprintf(logFile, "%s : ", formattedTimeString);
}

void removeDCByPID(MasterList *masterList, int pID)
{
	// FUNCTION		: removeDCByPID
	// DESCRIPTION	: This function will remove a DC (identified by pID) from the active list
	// PARAMETERS	:	
	//	  MasterList*	masterlist		: The pointer to a struct that holds the shared memory information
	//									  including the number and PIDs of currently running DCs among other things
	//	  int pID						: The active pID to be searched
	// RETURNS		: 
	//    VOID

	for (int masterListIndex = 0; masterListIndex < MAX_DC_ROLES; masterListIndex++)
	{
		if (masterList->dc[masterListIndex].dcProcessID == pID)
		{
			// Shift the array down one index.
			memmove(&masterList->dc[masterListIndex], &masterList->dc[masterListIndex + 1], sizeof(DCInfo) * (MAX_DC_ROLES - (masterListIndex + 1)));
			break;
		}
	}
	// since a DC has been closed, remove one from the active DC count
	masterList->numberOfDCs--;
}

void killIdleProcesses(MasterList *masterList, FILE* logFile)
{
	// FUNCTION		: killIdleProcesses
	// DESCRIPTION	: This function will kill any processes which have been idle for more than 35 seconds
	// PARAMETERS	:	
	//	  MasterList*	masterlist		: The pointer to a struct that holds the shared memory information
	//									  including the number and PIDs of currently running DCs among other things
	//	  FILE*		    logfile			: A pointer to the log file for logging purposes
	// RETURNS		: 
	//    VOID
	for (int i = 0; i < MAX_DC_ROLES && i < masterList->numberOfDCs; i++)
	{
		if (((int)time(NULL) - masterList->dc[i].lastTimeHeardFrom) > 35)
		{
			// Print the header and the log message to the log file
			printCurrentTimeToFile(logFile);
			fprintf(logFile, "DC-%.2d [%d] removed from master list - NON-RESPONSIVE\n", i + 1, masterList->dc[i].dcProcessID);

			// shift the array up one, covering the inactive DC with the remainder of the list
			memmove(&masterList->dc[i], &masterList->dc[i + 1], sizeof(DCInfo) * (MAX_DC_ROLES - (i + 1)));
			masterList->numberOfDCs--;
		}
	}
}