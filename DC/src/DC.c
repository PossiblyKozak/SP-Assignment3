#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../../Common/inc/Common.h"


int send_message (int mid, int op, char *team, char *player, int jersey)
{
  PLAYERDBMESSAGE msg;
  int sizeofdata = sizeof (PLAYERDBMESSAGE) - sizeof (long);

  // indicate message is destined for server
  msg.type = TYPE_SERVERMESSAGE;

  // indicate our returning type, used for filtering the queue
  msg.callerType = (int)getpid();

  // what are we doing?
  msg.dbop = op;

  // copy in associated data
  strcpy (msg.team, team);
  strcpy (msg.player, player);
  msg.jersey = jersey;

  // send the message to server
  msgsnd (mid, (void *)&msg, sizeofdata, 0);

  return 0;
}


int get_response (int mid)
{
  PLAYERDBMESSAGE response;
  int sizeofdata = sizeof (PLAYERDBMESSAGE) - sizeof (long);

  // get response
  msgrcv (mid, (void *)&response, sizeofdata, getpid(), 0);
  if (response.resultcode != 0) 
  {
    if(response.resultcode == SERVER_EXIT)
    {
      printf ("\n\n (CLIENT) SERVER is exiting ... %d\n", response.resultcode);
    }
    else
    {
      printf ("\n\n(CLIENT) ERROR: DB error code: %d\n", response.resultcode);
    }
  } 
  else 
  {
    printf ("\n\n(CLIENT) Successful DB operation\n");
  }

  return 0;
}

/* ----------------------------------------------------------------------
  This application will:
    1. get the unique identifier for a message queue using an agreed upon secret key 
    2. check that the queue exists
    3. if the queue doesn't exist, then the process will fork()
       a. and spawn a child process (msgServer1)
       b. msgClient1 will sleep for 5 seconds waiting for the msgServer1 to launch
    4. check for the queue again and begin to send messages
   ---------------------------------------------------------------------- */
int main (int argc, char *argv[])
{
  pid_t p;
  
  int mid; // message ID
  int done = 0;
  int choice;
  int j;
  
  key_t message_key;
  
  char buffer[100];
  char team[100];
  char player[100];

  // get the unique token for the message queue (based on some agreed 
  // upon "secret" information
  message_key = ftok ("/.", 'M');
  printf("%d\n", message_key);
  if (message_key == -1) 
  { 
    printf ("(CLIENT) Cannot create key!\n");
    fflush (stdout);
    return 0;
  }

  // check if the msg queue already exists
  mid = msgget (message_key, 0);
  if (mid == -1) 
  {
    p = fork();
    if (p == -1) 
    {
      printf ("(CLIENT) Can't fork!\n");
      return 1;
    }

    if (p == 0)   // this is the CHILD process
    {
      // in context of child, launch the server app!!
      execl ("./msgServer1", "msgServer1", NULL);
      exit(0);  // after launching the process, the child is done
    }
    else
    {
        // This is the parent - it will sleep for 5 seconds, and then retry
      // getting the message queue id. if this still
      // fails, then something seriously wrong!
      sleep (5);
      mid = msgget (message_key, 0);
      printf("%d\n", message_key);
      if (mid == -1) 
      {
        printf ("(CLIENT) Can't find the message queue!\n");
        return 2;
      }

        printf ("(CLIENT) Message queue ID: %d\n\n\n", mid);
    }
  }

  // main CLIENT processing loop
  while (done == 0) 
  {
    printf ("\n\nMenu\n\n");
    printf ("1 ... Add Player to DBase\n");
    printf ("2 ... Delete Player from DBase\n");
    printf ("3 ... List Player(s) in DBase\n");
    printf ("4 ... Quit Client App\n");
    printf ("\n\nEnter Your Choice: ");
    gets (buffer);
    printf ("\n\n");
    choice = atoi (buffer);

    switch (choice) 
    {
      case OPERATION_ADD:
      {
        // gather data for ADD
        printf ("Enter Team Name: ");
        gets (team);
        printf ("Enter Player's Last Name: ");
        gets (player);
        printf ("Enter Jersey #: ");
        gets (buffer);
        j = atoi (buffer);
  
        send_message (mid, OPERATION_ADD, team, player, j);
        get_response (mid);
        break;
      }

      case OPERATION_DELETE:
      {
         // gather data for DELETE
         printf ("Enter Team Name: ");
         gets (team);
         printf ("Enter Player Name: ");
         gets (player);
         printf ("Enter Jersey #: ");
         gets (buffer);
         j = atoi (buffer);

         send_message (mid, OPERATION_DELETE, team, player, j);
         get_response (mid);
         break;
      }

      case OPERATION_LIST:
      {
         send_message (mid, OPERATION_LIST, "", "", 0);
         get_response (mid);
         break;
      }

      case OPERATION_EXIT:
      {
         send_message (mid, OPERATION_EXIT, "", "", 0);
         get_response (mid);
         done = 1;
         break;
      }
    
      default:
      {
         printf ("**WRONG CHOICE ... follow the rules!\n");
         break;
      }
    }
  }

  return 0;
}



