#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "../../Common/inc/Common.h"
#include "../inc/DC.h"


int send_message (int message_id, int random_number)
{
  pid_t pId = getpid();
  struct theMESSAGE msg;
  msg.p = pId;
  msg.randoNum = random_number;

  msgsnd (message_id, (void *)&msg, sizeof(struct theMESSAGE), 0);

  return 0;
}


/*int get_response (int mid)
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
}*/

/* ----------------------------------------------------------------------
  This application will:
    1. get the unique identifier for a message queue using an agreed upon secret key 
    2. check that the queue exists
    3. if the queue doesn't exist, then the process will fork()
       a. and spawn a child process (msgServer1)
       b. msgClient1 will sleep for 5 seconds waiting for the msgServer1 to launch
    4. check for the queue again and begin to send messages
   ---------------------------------------------------------------------- */
int main ()
{
  key_t message_key;
  int check_for_existing_que = 0;
  int is_client_finished = 0;
  srand((unsigned) time(NULL));
  int number_to_send = 0;
  int checker = 0;

  message_key = ftok (".", 'A');
  if (message_key == -1)
  {
    printf("Error generating message key\n");
  }

  check_for_existing_que = msgget (message_key, 0);

  if (check_for_existing_que == -1) 
  {
    sleep (5);
    check_for_existing_que = msgget (message_key, 0);
    if (check_for_existing_que == -1) 
    {
      printf ("Cant find a queue, creating one...\n");
      check_for_existing_que = msgget (message_key, IPC_CREAT | 0660); 
      if (check_for_existing_que == -1)
      {
        printf("Can not create a queue.. Exiting\n");
        return 0;
      }
    }

    printf ("(CLIENT) Message queue ID: %d\n\n\n", check_for_existing_que);
  }
 

  // main CLIENT processing loop
  while (is_client_finished == 0) 
  {
    for (int x = 1; x <= 10; x++)
    {
      number_to_send = rand() % 7;

      if (number_to_send == OFF_LINE)
      {
        checker = send_message(check_for_existing_que, number_to_send);
        if (checker == 1)
        {
        	printf("There was an error sending the message\n");
        	return 0;
        }

        msgctl (check_for_existing_que, IPC_RMID, (struct msqid_ds *)NULL);
        return 0;
      }

      checker = send_message(check_for_existing_que, number_to_send);
      if (checker == 1)
      {
       	    printf("There was an error sending the message\n");
       	    break;
      }
    }
    /*choice = atoi (buffer);

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
         is_client_finished = 1;
         break;
      }
    
      default:
      {
         printf ("**WRONG CHOICE ... follow the rules!\n");
         break;
      }
    }*/
  }

  return 0;
}



