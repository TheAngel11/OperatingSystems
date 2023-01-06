#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <string.h>
#include <stdlib.h>

/* Messages */
#define ERROR_N_LESS_ARGS_MSG	 		"ERROR: Missing configuration file (not enough arguments)\n"
#define ERROR_N_MORE_ARGS_MSG	 		"ERROR: Too many arguments\n"
#define ERROR_OPENING_FILE		 		"ERROR: File %s could not be opened\n"
#define ERROR_CREATING_CHILD 	 		"ERROR: Child could not be created\n"
#define ERROR_CREATING_SOCKET_MSG		"ERROR: Socket could not be created\n"
#define ERROR_IP_CONFIGURATION_MSG		"ERROR: Failed to configure IP address\n"
#define ERROR_SERVER_CONNECTION_MSG		"ERROR: Failed to connect to Arda server\n"
#define ERROR_DISCONNECT_ILUVATAR_MSG 	"ERROR: Failed to disconnect from Arda\n"
#define ARDA_CONNECTION_CLOSED_MSG      "\nArda server has closed his connection\n"
#define MSG_RECIEVED_MSG                "\nNew message received!\n%s, from %s says:\n\"%s\"\n"
#define FILE_RECIEVED_MSG               "\nNew file received!\n%s, from %s has sent:\n%s\n"
#define ERROR_SELECT_MSG                "ERROR: Select failed\n"
#define ERROR_CREATING_MQ_MSG           "ERROR: Message queue could not be created\n"
#define ERROR_RECEIVING_MSG_MSG         "ERROR: Message could not be received\n"
/* Other constants */
#define CMD_ID_BYTE				    	'$'
#define CMD_LINE_PROMPT					"%s%c "
#define COLOR_DEFAULT_TXT				"\033[0m"
#define COLOR_CLI_TXT					"\033[1;36m"
#define COLOR_RED_TXT					"\033[1;31m"
#define END_OF_LINE		    			'\n'
#define FILE_MD5SUM_OK					1
#define FILE_MD5SUM_KO					0
#define MAX_FD_SET_SIZE                 1024

typedef struct {
    char *username;
	char *directory;
    char *arda_ip_address;
    int arda_port;
	char *ip_address;
	int port;
} IluvatarSon;

typedef struct {
    char *ip_address;
	int port;
    char *directory;
} Arda;

#endif
