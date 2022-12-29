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

#include "definitions.h"
#include "sharedFunctions.h"
#include "bidirectionallist.h"
#include "gpc.h"

#define FD_NOT_FOUND -1 

typedef struct {
    int server_fd;
	BidirectionalList clients;  //CLAUDIA: Perquè volem la llista de clients aqui quan la tenim com a global al Iluvatar? 
} Client;

Client CLIENT_init(char *ip, int port);

#endif
