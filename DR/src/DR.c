#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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
  message_key = ftok ("/.", 'A');
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

  printf ("(SERVER) Message queue ID: %d\n\n", mid);
  fflush (stdout);

  // compute size of data portion of message
  sizeofdata = sizeof (struct theMESSAGE) - sizeof (long);

  // loop until we are told to stop ...
  while (continueToRun == 1) 
  {
    printf ("(SERVER) Waiting for a message ...\n");
    fflush (stdout);

    // receive the incoming message and process it
    msgrcv (mid, &msg, sizeofdata, 1, 0);

    printf ("(SERVER) Got a message!\n");
    fflush (stdout);

    printf("%d\n", msg.randoNum);



    /*switch (msg.dbop) 
    {
      case OPERATION_ADD:
      {
  // add to player DB
  printf ("(SERVER) Adding player %s, jersey %d, to team %s\n",
    msg.player, msg.jersey, msg.team);
  fflush (stdout);

  // generate return message for confirmation
  // assume success
  response.type = msg.callerType; // set up for client filter
  response.resultcode = 0;  // all is well!
  break;
      }
      case OPERATION_DELETE:
      {
  // remove from player DB
  printf ("(SERVER) Deleting player %s, jersey %d, from team %s\n",
    msg.player, msg.jersey, msg.team);
  fflush (stdout);

  // generate random return message for confirmation
  response.type = msg.callerType; // set up for client filter
  response.resultcode = rand() % 3; // 3 codes ... 0, 1 or 2
  break;
      }
      case OPERATION_LIST:
      {
  // list player(s) in DB -- notice there is no actual database being used
  //  - nothing is being saved!!
  printf ("(SERVER) Received command to list player(s) in DBase ... this is ackward ...\n");
  fflush (stdout);

  // generate random return message for confirmation
  response.type = msg.callerType;   // set up for client filter
  response.resultcode = rand() % 3; // 3 codes ... 0, 1 or 2
  break;
      }
      case OPERATION_EXIT:
      {
  // client is shutting down
  printf ("(SERVER) received word that the CLIENT is exiting ...");
  fflush (stdout);

  // generate random return message for confirmation
  response.type = msg.callerType; // set up for client filter
  response.resultcode = SERVER_EXIT;  // indicate that the server is exiting as well
  continueToRun = 0;
  break;
      }

      default:
      {
  // invalid DB command!
  printf ("(SERVER) Invalid command! what are you thinking?\n");
  fflush (stdout);

  // generate random return message for confirmation
  response.type = msg.callerType; // set up for client filter
  response.resultcode = DUMB_CLIENT_ERROR;
  break;
      }
    } */
  }

  // done with the message queue - so clean-up
  msgctl (mid, IPC_RMID, NULL);
  printf ("(SERVER) Message QUEUE has been removed\n");
  fflush (stdout);

  return 0;
}



