/*	FILE			: DX.c
 *	PROJECT			: PROG1970 - Assignment #3
 *	PROGRAMMER		: Alex Kozak and Attila Katona
 *	FIRST VERSION	: 2018-07-14
 *	DESCRIPTION		: This file is the data corruptor and its purpose is to gain knowledge of the 
 *					  resources and processes involved in the program. It will randomly decide
 *				 	  between a set of allowable corruptions, including : 
 *							--> Kill a DC process
 *							-->	Delete the message queue.
 */
#include "../../Common/inc/Common.h"

void wheelOfDestruction(int randomWODNumber, MasterList *masterList, FILE *logFile);
void printCurrentTimeToFile(FILE* logFile);

int main()
{
	srand((unsigned)time(NULL)); //Initiate the random number generator 

	FILE *logFile = fopen(DC_LOG_FILE_PATH, "w");
	time_t currentTime = time(0);
	key_t sharedMemoryKey, messageQueueKey;

	int sharedMemoryID, queueID, randomWODNumber;
	MasterList *masterList;

	if (logFile) //If the log file was opened, continue on
	{
		//Get and check the Shared Memory, returning an error if unable to obtain it
		if ((sharedMemoryKey = ftok(SHARED_MEM_LOCATION, SHARED_MEM_KEY)) == -1) { return 1; }

		//Get and check the Queue, returning an error if unable to obtain it
		if ((messageQueueKey = ftok(QUEUE_LOCATION, QUEUE_KEY)) == -1) { return 1; }

		//Below is getting a Shared Memory ID to use and error checking at the same time
		if ((sharedMemoryID = shmget(sharedMemoryKey, sizeof(MasterList), 0)) == -1)
		{
			//Trying to open the Shared Memory key 100 times, breaking if unsuccessful
			for (int x = 0; x < 99; x++)
			{
				sleep(10);
				if ((sharedMemoryID = shmget(sharedMemoryKey, sizeof(MasterList), 0)) != -1) { break; }
			}
		}
		masterList = (MasterList*)shmat(sharedMemoryID, NULL, 0);	//Attaching to the Shared Memory data

		if (masterList == NULL) { return 1; }

		while (1) //Forever loop, will break when msgQ is gone
		{
			sleep((rand() % 20) + 10);

			if ((queueID = msgget(messageQueueKey, 0)) == -1)
			{
				printCurrentTimeToFile(logFile);
				fprintf(logFile, "DX detected that msgQ is gone - assuming DR/DCs done");
				shmdt(masterList);
				fclose(logFile);
				break;
			}

			randomWODNumber = rand() % 20;//Making a random number to pass to WOD function
			//Below are the cases when we close the message que and exit
			if (randomWODNumber == DELETE_MSGQ_10 || randomWODNumber == DELETE_MSGQ_17)
			{
				if ((msgctl(queueID, IPC_RMID, NULL) == -1))
				{
					printCurrentTimeToFile(logFile);
					fprintf(logFile, "DX Failed to delete the msgQ, must already be deleted - exiting");
					break;
				}
				else //Below is where we log the what were doing
				{
					printCurrentTimeToFile(logFile);
					fprintf(logFile, "DX Deleted the msgQ - the DR/DCs cant talk anymore - exiting");
					shmdt(masterList);
					break;
				}

			}
			wheelOfDestruction(randomWODNumber, masterList, logFile);//Call the function to kill processes
		}

	}
	else { return 1; }

	fclose(logFile);
	return 0;
}

void wheelOfDestruction(int randomWODNumber, MasterList *masterList, FILE *logFile)
{
	// FUNCTION		: wheelOfDestruction
	// DESCRIPTION	: This is the main processing function. It will take a random number and peform the
	//				  action that the number represents. All numbers represent killing of a process.
	//				  The random number passed is what determiens which process of DC to kill.
	// PARAMETERS	:	
	//	  int			randomWODnumber : Holds a random integer number. 1 to 20, except 0,8,10,17,19
	//	  MasterList*	masterlist		: The pointer to a struct that holds the process IDs of the open programs
	//									  that need to be killed. DC processes only.
	//	  FILE*		    logfile			: A pointer to the log file for logging what was killed and when.
	// RETURNS		: 
	//    VOID
	
	int killIndex = 0;
	pid_t killPID = 0;

	switch (randomWODNumber)
	{
	case 1:
	case 4:
	case 11:
	{
		killIndex = 1;
		break;
	}
	case 2:
	case 5:
	case 15:
	{
		killIndex = 3;
		break;
	}
	case 3:
	case 6:
	case 13:
	{
		killIndex = 2;
		break;
	}
	case 7:
	{
		killIndex = 4;
		break;
	}
	case 9:
	{
		killIndex = 5;
		break;
	}
	case 12:
	{
		killIndex = 6;
		break;
	}
	case 14:
	{
		killIndex = 7;
		break;
	}
	case 16:
	{
		killIndex = 8;
		break;
	}
	case 18:
	{
		killIndex = 9;
		break;
	}
	case 20:
	{
		killIndex = 10;
		break;
	}
	}
	if (killIndex <= masterList->numberOfDCs) //Making sure the number to kill is not higher than the number of open processes
	{
		killPID = masterList->dc[killIndex - 1].dcProcessID; //Get the PID from the shared memory 

		if (!kill(killPID, SIGHUP))
		{
			printCurrentTimeToFile(logFile);
			fprintf(logFile, "WOD Action %.2d - D-%.2d [%d] TERMINATED\n", randomWODNumber, killIndex, (int)killPID);
		}
	}
}

void printCurrentTimeToFile(FILE* logFile)
{
	// FUNCTION		: printCurrentTime
	// DESCRIPTION	: This function will generate the time and date formatted to the project standards.
	//				  It will then print this time and date to the logfile.
	// PARAMETERS	:	
	//	    FILE* logfile	: A pointer to the log file for logging what was killed and when.
	// RETURNS		: 
	//		Void
	time_t currTime;
	struct tm* currTimeInfo;
	char formattedTimeString[28];

	time(&currTime);
	currTimeInfo = localtime(&currTime);

	strftime(formattedTimeString, 28, "[%Y-%m-%d %H:%M:%S]", currTimeInfo);
	fprintf(logFile, "%s : ", formattedTimeString);
}