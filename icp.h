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

#define ICP_DATA_SEPARATOR		 	'&'
#define ICP_FILE_MAX_BYTES		 	512
#define ICP_READ_FRAME_ERROR	 	0
#define ICP_READ_FRAME_NO_ERROR	 	1

/* Messages */
#define ICP_MSG_RECEIVED_MSG     	"\nNew message received!\nYour neighbor %s says:\n%s\n"
#define ICP_FILE_RECEIVED_MSG    	"\nNew file received!\nYour neighbor %s has sent:\n%s\n"
#define SEND_FILE_OPEN_FILE_ERROR	"ERROR: Could not open %s\n"
#define SEND_FILE_EMPTY_FILE_ERROR	"ERROR: Cannot send an empty file\n"
#define SEND_FILE_MQ_ERROR				"ERROR: Message Queue failed to send the file\n"

/*********************************************************************
* @Purpose: Sends a message to a user using message queues.
* @Params: in: pid = PID of the user that will receive the message
* 		   in: message = string containing the message to send
* 		   in: origin_username = string with the name of the sender
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if message was sent successfully, otherwise 1.
*********************************************************************/
char ICP_sendMsg(int pid, char *message, char *origin_username, pthread_mutex_t *mutex);

/*********************************************************************
* @Purpose: Sends a file to a user using message queues.
* @Params: in: qfd = message queue file descriptor
* 		   in: filename = name of the file to send
* 		   in: directory = string with the directory of the file
* 		   in: username = string containing the name of the sender
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if the file was sent successfully, otherwise 1.
*********************************************************************/
char ICP_sendFile(mqd_t qfd, char *filename, char *directory, char *username, pthread_mutex_t *mutex);

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
