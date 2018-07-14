

#define MAX_DC_ROLES			10

#define EVERYTHING_OKAY			0
#define HYDRAULIC_FAILURE		1
#define SAFETY_BUTTON_FAILURE	2
#define NO_RAW_MATERIAL			3
#define TEMP_OUT_OF_RANGE		4
#define OPERATOR_ERROR			5
#define MACHINE_OFFLINE			6

#define DC_LOG_FILE_PATH		"/tmp/dataCorruptor.log"
#define DR_LOG_FILE_PATH		"/tmp/dataMonitor.log"

#define SHARED_MEM_KEY			16535
#define SHARED_MEM_LOCATION		"."

#define QUEUE_KEY				'A'
#define QUEUE_LOCATION			"."

typedef struct tagQueueMessage 
{
	// the following is a requriement of UNIX/Linux
	long type;

	// now the specifics of our message
	int randomNumber;
	pid_t pID;
} QueueMessage;

typedef struct tagDCInfo
{
	pid_t dcProcessID;
	int lastTimeHeardFrom;
} DCInfo;

typedef struct tagMasterList
{
	int msgQueueId;
	int numberOfDCs;
	DCInfo dc[MAX_DC_ROLES];
} MasterList;