/*
 * struct.h
 *
 * this defines the shared structure between the clients and
 * servers
 */

struct theMESSAGE 
{
	// the following is a requriement of UNIX/Linux
	long type;

	// now the specifics of our message
	int randoNum;
	pid_t p;
};

//Define constants

#define OFF_LINE 6

int send_message (int mid, int random_number);
