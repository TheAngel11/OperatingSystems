#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h> // function strcasecmp() is case insensitive

/* CUSTOM COMMANDS */
#define UPDATE_USERS_CMD	"UPDATE USERS"
#define LIST_USERS_CMD		"LIST USERS"
#define SEND_MSG_CMD		"SEND MSG"
#define SEND_FILE_CMD		"SEND FILE"
#define EXIT_CMD			"EXIT"
#define CMD_ID_BYTE			'$'
#define CMD_END_BYTE		'\n'

char * readCommand();

#endif
