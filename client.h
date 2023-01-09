#ifndef _CLIENT_H_
#define _CLIENT_H_

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "definitions.h"
#include "sharedFunctions.h"
#include "bidirectionallist.h"
#include "gpc.h"

#define FD_NOT_FOUND 	-1
#define EXIT_ARDA_MSG	"\nDisconnecting from Arda. See you soon, son of Iluvatar\n\n"

typedef struct {
    int server_fd;
} Client;

/*********************************************************************
* @Purpose: Initializes a client.
* @Params: in: ip = string with the server ip
*          in: port = server port
* @Return: Returns an initialized instance of Client.
*********************************************************************/
Client CLIENT_init(char *ip, int port);

/*********************************************************************
* @Purpose: Manages replies from Arda server.
* @Params: in/out: c = initialized instance of Client
*          in/out: users_list = list of users of the client
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 1 if received EXIT, otherwise 0.
*********************************************************************/
char CLIENT_manageArdaServerAnswer(Client *c, BidirectionalList *users_list, pthread_mutex_t *mutex);

/*********************************************************************
* @Purpose: Sends a message to an IluvatarSon in different machines.
* @Params: in/out: c = initialized instance of Client
*          in/out: data = string with message to send
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if no errors, otherwise 1.
*********************************************************************/
char CLIENT_sendMsg(Client *c, char **data, pthread_mutex_t *mutex);

/*********************************************************************
* @Purpose: Sends a file to an IluvatarSon in different machines.
* @Params: in/out: c = initialized instance of Client
*          in/out: data = string with info about file to send
*          in/out: fd_file = open file descriptor of the file to send
*          in: file_size = size in bytes of the file to send
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if no errors, otherwise 1.
*********************************************************************/
char CLIENT_sendFile(Client *c, char **data, int *fd_file, int file_size, pthread_mutex_t *mutex);

#endif
