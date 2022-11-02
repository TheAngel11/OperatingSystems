#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h> // function strcasecmp() is case insensitive
#include <string.h>
#include <sys/wait.h>

#include "../definitions.h"
#include "../sharedFunctions.h"

/* CUSTOM COMMANDS */
#define UPDATE_USERS_CMD		"UPDATE USERS\0"
#define LIST_USERS_CMD			"LIST USERS\0"
#define SEND_MSG_CMD			"SEND MSG\0"
#define SEND_FILE_CMD			"SEND FILE\0"
#define EXIT_CMD				"EXIT\0"
#define CMD_ID_BYTE				'$'
#define CMD_END_BYTE			'\n'
#define CMD_MSG_SEPARATOR		'"'

/* Messages */
#define UNKNOWN_CMD_MSG			"Unknown command\n"
#define ERROR_SEND_MSG_FORMAT	"ERROR: Missing message separators\n"
#define ERROR_SEND_MSG_ARGS		"ERROR: \"SEND MSG\" requires a user and a message\n"
#define ERROR_SEND_FILE_ARGS	"ERROR: \"SEND FILE\" requires a user and a file\n"

/* Number of required args for custom command */
#define UPDATE_USERS_N_ARGS		2
#define LIST_USERS_N_ARGS		2
#define SEND_MSG_N_ARGS			4
#define SEND_FILE_N_ARGS		4
#define EXIT_N_ARGS				1

/* ID to identify custom command */
#define IS_UPDATE_USERS_CMD		1
#define IS_LIST_USERS_CMD		2
#define IS_SEND_MSG_CMD			3
#define IS_SEND_FILE_CMD		4
#define IS_EXIT_CMD				5
#define IS_NOT_CUSTOM_CMD		0
#define ERROR_CMD_ARGS			-1

int COMMANDS_executeCommand(char *user_input, IluvatarSon *iluvatar);

#endif
