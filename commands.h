#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h> // function strcasecmp() is case insensitive
#include <string.h>
#include <sys/wait.h>
#include "definitions.h"

/* CUSTOM COMMANDS */
#define UPDATE_USERS_CMD		"UPDATE USERS\0"
#define LIST_USERS_CMD			"LIST USERS\0"
#define SEND_MSG_CMD			"SEND MSG\0"
#define SEND_FILE_CMD			"SEND FILE\0"
#define EXIT_CMD				"EXIT\0"
#define CMD_ID_BYTE				'$'
#define CMD_END_BYTE			'\n'
#define CMD_CUSTOM_MAX_WORDS	2
#define UNKNOWN_CMD_MSG			"Unknown command\n"
#define IS_EXIT_CMD				5

char * readCommand();

int executeCommand(char *user_input);

#endif
