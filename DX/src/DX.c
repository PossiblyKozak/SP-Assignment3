/*  
*  FILE          : encodeInput.c
*  PROJECT       : PROG1970 - Assignment #2
*  PROGRAMMER    : Alex Kozak 
*  FIRST VERSION : 2018-06-18 
*  DESCRIPTION   : 
*    The functions in this file are used determine flags and required input/output files aswell as format, 
*    which will be used by either the encodeSREC or encodeASM files to convert the given input to the 
*    desired output format.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

//#include "../inc/encodeSREC.h"

#define MAX_FILE_NAME_LENGTH 100
#define MAX_FLAGS 5

void main(int argc, char *argv[])
{
  key_t msgKey;
  int qID;

  msgKey = ftok("Log.txt", 0);

  if ((qID == msgget(msgKey,0)) == -1)
  {
    qID = msgget(msgKey, (IPC_CREAT | 0660));
    if (qID == -1)
    {
      printf("This is an error");
    }
  }
  // declare variables

}