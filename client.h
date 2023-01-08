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

Client CLIENT_init(char *ip, int port);

char CLIENT_manageArdaServerAnswer(Client *c, BidirectionalList *users_list, pthread_mutex_t *mutex);

char CLIENT_sendMsg(Client *c, char **data, pthread_mutex_t *mutex);

char CLIENT_sendFile(Client *c, char **data, int *fd_file, int file_size, pthread_mutex_t *mutex);

#endif
