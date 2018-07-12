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
#define OPERATION_INVALIDDATA		2
#define SERVER_EXIT			10
#define DUMB_CLIENT_ERROR		999