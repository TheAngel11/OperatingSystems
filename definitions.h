#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <string.h>
#include <stdlib.h>

/* Messages */
#define ERROR_N_LESS_ARGS_MSG	 	"ERROR: Missing configuration file (not enough arguments)\n"
#define ERROR_N_MORE_ARGS_MSG	 	"ERROR: Too many arguments\n"
#define ERROR_OPENING_FILE		 	"ERROR: File %s could not be opened\n"
#define ERROR_CREATING_CHILD 	 	"ERROR: Child could not be created\n"
#define ERROR_CREATING_SOCKET_MSG	"ERROR: Socket could not be created\n"
#define ERROR_IP_CONFIGURATION_MSG	"ERROR: Failed to configure IP address\n"
#define ERROR_SERVER_CONNECTION_MSG	"ERROR: Failed to connect to Arda server\n"

#define COLOR_DEFAULT_TXT			"\033[0m"
#define COLOR_CLI_TXT				"\033[1;36m"
#define COLOR_RED_TXT				"\033[1;31m"
#define END_OF_LINE		    		'\n'

/* General Protocol Communication (GPC) constants */
#define GPC_DATA_SEPARATOR			'&'
#define GPC_DATA_SEPARATOR_STR		"&"
#define GPC_USERS_SEPARATOR			'#'
#define GPC_UPDATE_USERS_HEADER_IN	"LIST_REQUEST"
#define GPC_UPDATE_USERS_HEADER_OUT	"LIST_RESPONSE"

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
