#ifndef _ICP_H_
#define _ICP_H_

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include <sys/wait.h>

#include "definitions.h"
#include "sharedFunctions.h"

#define ICP_DATA_SEPARATOR		 '&'
#define ICP_FILE_MAX_BYTES		 512
#define ICP_READ_FRAME_ERROR	 0
#define ICP_READ_FRAME_NO_ERROR	 1
/* Messages */
#define ICP_MSG_RECEIVED_MSG     "\nNew message received!\nYour neighbor %s says:\n%s\n"
#define ICP_FILE_RECEIVED_MSG    "\nNew file received!\nYour neighbor %s has sent:\n%s\n"

/**********************************************************************
* @Purpose: Receives a message from another process in the same machine
*           and prints it.
* @Params: in: frame = ICP frame containing the message and the name
*              of the sender
* 		   in/out: mutex = screen mutex to prevent printing to screen
*                  by different users at the same time
* @Return: ----
**********************************************************************/
void ICP_receiveMsg(char *frame, pthread_mutex_t *mutex);

char ICP_receiveFile(char **frame, char *directory, struct mq_attr *attr, int qfd, pthread_mutex_t *mutex);

#endif
