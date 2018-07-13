
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "../inc/DX.h"
#include "../../Common/inc/Common.h"

//#include "../inc/encodeSREC.h"

#define MAX_FILE_NAME_LENGTH 100
#define MAX_FLAGS 5

void main()
{
  srand((unsigned) time(NULL));

  FILE *logFile = fopen(kTextFilePath,"w");
  time_t currentTime = time(0);

  key_t sharedMemoryKey = 0;
  key_t messageQueKey = 0;

  int checker = 0;
  int sharedMemory_ID = 0;
  int process_ID = 0;
  int check_for_existing_que = 0;
  int randomSleepTime = 0;
  int randomWODnumber = 0;
  struct MasterList *sharedData; 

  if (logFile) //If the log file was opened, continue on
  {
    sharedMemoryKey = ftok("/.", 16535);//Get a key for the shared memory
    messageQueKey = ftok("/.", 'A');//Get a key for the message que, used to check its existence

    if (sharedMemoryKey == -1)//Error checking against the shared memeory
    {
      printf("Could not get a key/n");
      return 0;
    }
    if (messageQueKey == -1)//Error checking against the que
    {
      printf("Could not get a key/n");
      return 0;
    }

    //Below is getting a shared memory ID to use and error checking at the same time
    if ((sharedMemory_ID = shmget (sharedMemoryKey, sizeof (MasterList), 0)) == -1) 
	  {
      //Below is a loop to keep trying to open the shared memory key
		  for (int x = 0; x < 98; x++)
      {
        sleep(10);
        sharedMemory_ID = shmget (sharedMemoryKey, sizeof (MasterList), 0);
        if (sharedMemory_ID != -1)
        { 
          break;
        }
      }
	  }
    sharedData = (MasterList *)shmat (sharedMemory_ID, NULL, 0);

	  if (sharedData == NULL) 
	  {
	    printf ("(CORRUPTOR) Cannot attach to Shared-Memory!\n");
	    return 0;
	  }


    while(1)
    { 
      randomSleepTime = (rand() % 20) + 10;
      sleep(randomSleepTime); 

      check_for_existing_que = msgget(messageQueKey, 0);

      if (check_for_existing_que == -1) 
      {
        fprintf(logFile,"%s : DX detected that msgQ is gone - assuming DR/DCs done", ctime(&currentTime));
        shmdt(sharedData);
        fclose(logFile);
      }

      randomWODnumber = rand() % 20;





    }
    




  }
  else
  {
    printf("Error opening file\n");
    return 0;
  }
  return 0;
}

void wheelOfDestruction (int number, MasterList *p)
{
  if ()
  switch(number)
  {
    case 1 :
    {

    }
          
    case 0 :

    case 0 :

    case 0 :

    case 0 :

    case 0 :

    case 0 :

    case 0 :

    case 0 :

    case 0 :

  }

}