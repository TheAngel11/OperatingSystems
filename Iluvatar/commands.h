#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define _GNU_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h> // function strcasecmp() is case insensitive
#include <string.h>
#include <sys/wait.h>
#include <mqueue.h>
#include <pthread.h>

#include "../definitions.h"
#include "../sharedFunctions.h"
#include "../bidirectionallist.h"
#include "../gpc.h"
#include "../icp.h"
#include "../server.h"
#include "../client.h"

/* CUSTOM COMMANDS */
#define UPDATE_USERS_CMD		"UPDATE USERS\0"
#define LIST_USERS_CMD			"LIST USERS\0"
#define SEND_MSG_CMD			"SEND MSG\0"
#define SEND_FILE_CMD			"SEND FILE\0"
#define EXIT_CMD				"EXIT\0"
#define CMD_END_BYTE			'\n'
#define CMD_MSG_SEPARATOR		'"'

/* Messages */
#define UNKNOWN_CMD_MSG					"Unknown command\n"
#define ERROR_SEND_MSG_FORMAT			"ERROR: Missing message separators (\")\n"
#define ERROR_SEND_MSG_LESS_ARGS		"ERROR: \"SEND MSG\" requires a user and a message\n"
#define ERROR_SEND_MSG_MORE_ARGS		"ERROR: \"SEND MSG\" has too many arguments\n"
#define ERROR_SEND_FILE_LESS_ARGS		"ERROR: \"SEND FILE\" requires a user and a file\n"
#define ERROR_SEND_FILE_MORE_ARGS		"ERROR: \"SEND FILE\" has too many arguments\n"
#define UPDATE_USERS_SUCCESS_MSG		"Users list updated\n"
#define LIST_USERS_N_USERS_MSG 			"There are %d children of Iluvatar connected:\n"
#define SEND_MSG_ERROR_SAME_USER		"ERROR: You cannot send a message to yourself\n"
#define SEND_MSG_INVALID_MSG_ERROR		"ERROR: Message cannot be empty\n"
#define USER_NOT_FOUND_ERROR_MSG		"ERROR: %s was not found. Try updating the list of users\n"
#define SEND_FILE_INVALID_FILE_ERROR 	"ERROR: File could not be sent due to an error in the data\n"

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

/* Other constants */
#define USER_FOUND				1
#define USER_NOT_FOUND			0
#define IS_REMOTE_USER			1
#define IS_LOCAL_USER			0
#define SEND_MSG_OK				1
#define SEND_MSG_KO				0

/*********************************************************************
* @Purpose: Executes the command entered by the user.
* @Params: in: user_input = entire command (with args) entered by user
*          in/out: iluvatar = IluvatarSon issuing command
*		   in: fd_arda = Arda's file descriptor (connected to server)
* @Return: 0 if EXIT command entered, otherwise 1.
*********************************************************************/
int COMMANDS_executeCommand(char *user_input, IluvatarSon *iluvatar, int fd_arda, BidirectionalList *users_list, pthread_mutex_t *mutex);

#endif
