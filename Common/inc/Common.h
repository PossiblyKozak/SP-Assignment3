/*
 * struct.h
 *
 * this defines the shared structure between the clients and
 * servers
 */

/* some constants to be used in the message passing and handling */
#define TYPE_SERVERMESSAGE		1

#define OPERATION_ADD			1
#define OPERATION_DELETE		2
#define OPERATION_LIST			3
#define OPERATION_EXIT			4

#define OPERATION_SUCCESS		0
#define OPERATION_DBFAILURE		1
#define OPERATION_INVALIDDATA	2
#define SERVER_EXIT				10
#define DUMB_CLIENT_ERROR		999

#define MAX_DC_ROLES			10

#define EVERYTHING_OKAY			0
#define HYDRAULIC_FAILURE		1
#define SAFETY_BUTTON_FAILURE	2
#define NO_RAW_MATERIAL			3
#define TEMP_OUT_OF_RANGE		4
#define OPERATOR_ERROR			5
#define MACHINE_OFFLINE			6

#define kTextFilePath			"/tmp/dataCorruptor.log"
struct theMESSAGE 
{
	// the following is a requriement of UNIX/Linux
	long type;

	// now the specifics of our message
	int randoNum;
	pid_t p;
};

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