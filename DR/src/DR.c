#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "../../Common/inc/Common.h"
#include "../inc/DR.h"


/* ----------------------------------------------------------------------
  This application will:
    1. get the unique identifier for a message queue using an agreed upon secret key 
    2. create the message queue
    3. allocate enough space for the expected incoming messages and continue
       a. to receive incoming messages from the client (msgClient1)
       b. take the appropriate action and store / delete the data from the
    local databast
    4. until the "STOP" command comes in - at which point, the queue is free'd and deleted
   ---------------------------------------------------------------------- */

void killIdleProcesses(MasterList *ml);
void removeDCByPID(MasterList *ml, int pID);
void interpretMessageCode(MasterList *ml, int randomNumber, int pID);
void printProcesses(MasterList *ml);
void isCurrentlyActivePID( int *i, bool *isRecognized, MasterList* ml, int pID);

int main (int argc, char *argv[])
{
  int mid; // message ID
  int sizeofdata;
  int continueToRun = 1;
  
  key_t message_key;
  
  struct theMESSAGE msg;
  struct theMESSAGE response;

  // get the unique token for the message queue (based on some agreed 
  // upon "secret" information  
  message_key = ftok (QUEUE_LOCATION, QUEUE_KEY);
  printf("%d", message_key);
  if (message_key == -1) 
  { 
    printf ("(SERVER) Cannot create key!\n");
    fflush (stdout);
    return 1;
  }

  printf("%d\n", message_key);

  // create the message queue
  mid = msgget (message_key, IPC_CREAT | 0660);
  if (mid == -1) 
  {
    printf ("(SERVER) ERROR: Cannot create queue\n");
    fflush (stdout);
    return 2;
  }

  printf ("(SERVER) Message queue ID: %d\n", mid);
  fflush (stdout);

  int shmID;
  key_t shmKey = ftok(".", SHARED_MEM_KEY);  
  if (shmID = shmget(shmKey, 100, 0) == -1)
  {
    printf("Shared memory doesn't not exist, creating new block...\n");
    shmID = shmget(shmKey, sizeof(MasterList), (IPC_CREAT | 0660));
    shmctl(shmID, IPC_RMID, 0);
    shmID = shmget(shmKey, sizeof(MasterList), (IPC_CREAT | 0660));
    if (shmID == -1)
    {
      printf("Error creating the shared memory");
    }
  }
  else
  {
    printf("Clearing Shared Memory...\n");
  }

  printf ("(SERVER) Shared Memory ID: %d\n", shmID);
  MasterList *ml = (MasterList*)shmat(shmID, NULL, 0);
  ml->numberOfDCs = 0;

  if (ml == NULL)
  {
    printf("Cannot attach to shared memory!");
  }
  else
  {
    ml->msgQueueId = mid;
    printf ("(SERVER) Message queue ID: %d\n", ml->msgQueueId);
  }  


  // compute size of data portion of message
  sizeofdata = sizeof (struct theMESSAGE) - sizeof (long);

  // loop until we are told to stop ...
  sleep(10);
  do
  {
    printf ("(SERVER) Waiting for a message ... %d\n", ml->numberOfDCs);
    fflush (stdout);

    // receive the incoming message and process it
    if (msgrcv (mid, &msg, sizeofdata, 0, IPC_NOWAIT) != -1)
    {
      printf ("(SERVER) Got a message!\n");
      fflush (stdout);

      printf("(SERVER) pID: %d\tRandom Number: %d\n", msg.p ,msg.randoNum);

      bool isRecognized = false;
      int i = 0;
      for (; i < MAX_DC_ROLES && !(isRecognized); i++) 
      {
        if (ml->dc[i].dcProcessID != NULL)
        {
          if (ml->dc[i].dcProcessID == msg.p)
          {
            ml->dc[i].lastTimeHeardFrom = (int)time(NULL);
            isRecognized = true;
          }
        }
        else
        {
          break;
        }
      }
      interpretMessageCode(ml, msg.randoNum, msg.p);

      if (!isRecognized && ml->numberOfDCs != MAX_DC_ROLES)
      {
        printf("(SERVER) New PID Recognized (%d) Adding to shared memory at dc[%d]\n", msg.p, i);
        ml->dc[i].dcProcessID = msg.p;
        ml->dc[i].lastTimeHeardFrom = (int)time(NULL);
        ml->numberOfDCs++;
        printf("(SERVER) Current number of active DCs: %d\n", ml->numberOfDCs);
        printProcesses(ml);
      }   
    }
    killIdleProcesses(ml);
    printProcesses(ml);
    usleep(1500000);
  } while (ml->numberOfDCs > 0 && ((mid = msgget (message_key, 0)) != -1));
  msgctl (mid, IPC_RMID, NULL);
  shmctl (shmID, IPC_RMID, NULL);
  printf ("(SERVER) Message QUEUE has been removed\n");
  fflush (stdout);  
  return 0;
}

void isCurrentlyActivePID( int *i, bool *isRecognized, MasterList* ml, int pID)
{
}

void interpretMessageCode(MasterList *ml, int randomNumber, int pID)
{
  switch (randomNumber)
  {
  case (EVERYTHING_OKAY):
  {
    printf("(%d) EVERYTHING OKAY", pID);
    break;
  }
  case (HYDRAULIC_FAILURE):
  {
    printf("(%d) HYDRAULIC PRESSURE FAILURE", pID);
    break;
  }
  case (SAFETY_BUTTON_FAILURE):
  {
    printf("(%d) SAFETY BUTTON FAILURE", pID);
    break;
  }
  case (NO_RAW_MATERIAL):
  {
    printf("(%d) NO RAW MATERIAL IN PROCESS", pID);
    break;
  }
  case (TEMP_OUT_OF_RANGE):
  {
    printf("(%d) OPERATING TEMPERATURE OUT OF RANGE", pID);
    break;
  }
  case (OPERATOR_ERROR):
  {
    printf("(%d) OPERATOR ERROR", pID);
    break;
  }
  case (MACHINE_OFFLINE):
  {
    printf("(%d) MACHINE IS OFF-LINE", pID);
    removeDCByPID(ml, pID);
    break;
  }
  default:
  {
    printf("(%d) UNKNOWN COMMAND", pID);
  }
  }
  printf("\n"); 
}

void removeDCByPID(MasterList *ml, int pID)
{
  for (int i = 0; i < MAX_DC_ROLES; i++)
  {
    if (ml->dc[i].dcProcessID == pID)
    {
      printf("\nRemoving %d from index %d of the active DC list\n", pID, i);
      memmove(&ml->dc[i], &ml->dc[i+1], sizeof(DCInfo) * (MAX_DC_ROLES - (i + 1)));              
    }
  }
  ml->numberOfDCs--;
}

void killIdleProcesses(MasterList *ml)
{
  for (int i = 0; i < MAX_DC_ROLES && i < ml->numberOfDCs; i++)
  {
    if (((int)time(NULL) - ml->dc[i].lastTimeHeardFrom) > 35)
    {
      printf("\nRemoving %d from index %d of the active DC list, idle for too long.\n", ml->dc[i].dcProcessID, i);
      memmove(&ml->dc[i], &ml->dc[i+1], sizeof(DCInfo) * (MAX_DC_ROLES - (i + 1)));              
      ml->numberOfDCs--;
    }
  }  
}

void printProcesses(MasterList *ml)
{
  for (int i = 0; i < MAX_DC_ROLES && i < ml->numberOfDCs; i++)
  {
    if (ml->dc[i].dcProcessID != NULL)
    {
      printf("Process %d: (%d) Last heard from %d seconds ago\n", i, ml->dc[i].dcProcessID, (int)time(NULL) - ml->dc[i].lastTimeHeardFrom);
    }
    else
    {
      break;
    }
  }
}