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
  int pID = (int)getpid();
  QueueMessage SendingMessage;
  SendingMessage.pID = pID;
  SendingMessage.randomNumber = random_number;

  printf("Send Message: pID: %d, randomNumber: %d\n", SendingMessage.pID, SendingMessage.randomNumber);
  return msgsnd (message_id, (void *)&SendingMessage, sizeof(QueueMessage) - sizeof(long), 0);    
}

int main ()
{
  key_t message_key;
  int check_for_existing_que = 0;
  int is_client_finished = 0;
  srand((unsigned) time(NULL));
  int number_to_send = 0;
  int checker = 0;

  message_key = ftok (QUEUE_LOCATION, QUEUE_KEY);
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
  }
 	printf ("(CLIENT) Message queue ID: %d\n\n\n", check_for_existing_que);

  // main CLIENT processing loop
  while (is_client_finished == 0 && (check_for_existing_que = msgget (message_key, 0)) != -1) 
  {
      number_to_send = rand() % 7;
      if (number_to_send == MACHINE_OFFLINE)
      {
        checker = send_message(check_for_existing_que, number_to_send);
        if (checker == 1)
        {
        	printf("There was an error sending the message\n");
        	return 0;
        }
        return 0;
      }
      checker = send_message(check_for_existing_que, number_to_send);
      if (checker == 1)
      {
       	    printf("There was an error sending the message\n");
       	    break;
      }
      sleep((rand()%20) + 10);
  }
  return 0;
}



