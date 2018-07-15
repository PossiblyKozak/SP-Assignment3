//FILE :Common.h
//PROJECT : PROG1970 - Assignment #3
//PROGRAMMER : alex Kozak and Attila Katona
//FIRST VERSION : 2018-07-14
//DESCRIPTION :	This file holds all the #includes, #define constants and structs used in the
//				Hoochamacallit System. This file is included in DC.c, DX.c and DR.c
//
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
#include <signal.h>

#define MAX_DC_ROLES			10

#define EVERYTHING_OKAY			0
#define HYDRAULIC_FAILURE		1
#define SAFETY_BUTTON_FAILURE	2
#define NO_RAW_MATERIAL			3
#define TEMP_OUT_OF_RANGE		4
#define OPERATOR_ERROR			5
#define MACHINE_OFFLINE			6
#define DELETE_MSGQ_10			10
#define DELETE_MSGQ_17          17
#define DEFAULT_SLEEP_LENGTH    10
#define NUMBER_OF_MESSAGES      7
#define DC_LOG_FILE_PATH		"/tmp/dataCorruptor.log"
#define DR_LOG_FILE_PATH		"/tmp/dataMonitor.log"

#define SHARED_MEM_KEY			16535
#define SHARED_MEM_LOCATION		"."

#define QUEUE_KEY				'A'
#define QUEUE_LOCATION			"."

typedef struct tagQueueMessage
{
	// the following is a requirement of UNIX/Linux
	long type;

	// now the specifics of our message
	int messageValue;
	pid_t pID;
} QueueMessage;

typedef struct tagDCInfo
{
	pid_t dcProcessID;
	int lastTimeHeardFrom;
} DCInfo;

typedef struct tagMasterList
{
	int messageQueueID;
	int numberOfDCs;
	DCInfo dc[MAX_DC_ROLES];
} MasterList;