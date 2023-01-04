#ifndef _ICP_H_
#define _ICP_H_

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "sharedFunctions.h"

#define ICP_DATA_SEPARATOR				'&'
#define MSG_NEIGHBOURS_RECEIVED_MSG     "\nNew message received!\nYour neighbor %s says:\n%s\n"

void ICP_sendMsg(char *message_receive, pthread_mutex_t *mutex);

#endif
