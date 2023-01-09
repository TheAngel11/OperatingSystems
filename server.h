#ifndef _SERVER_H_
#define _SERVER_H_

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "definitions.h"
#include "sharedFunctions.h"
#include "bidirectionallist.h"
#include "gpc.h"

/* Messages */
#define ERROR_BINDING_SOCKET_MSG		"ERROR: Server could not bind the server socket\n"
#define ERROR_LISTENING_MSG         	"ERROR: Server could not make the listen\n"
#define ERROR_ACCEPTING_MSG	        	"ERROR: Server could not accept the connection request\n"
#define ERROR_CREATING_THREAD_MSG   	"ERROR: Server could not create the thread\n"
#define ERROR_TYPE_NOT_IMPLEMENTED_MSG	"ERROR: That type of frame has not been implemented yet\n"
#define NEW_LOGIN_MSG                   "New login: %s, IP: %s, port: %d, PID %d\n"
#define UPDATING_LIST_MSG               "Updating user's list\n"
#define SENDING_LIST_MSG                "Sending user's list\n"
#define RESPONSE_SENT_LIST_MSG          "Response sent\n\n"
#define PETITION_UPDATE_MSG             "New petition: %s demands the user's list\nSending user's list to %s\n\n"
#define PETITION_EXIT_MSG               "New exit petition: %s has left Arda\n"
/* Constants */
#define BACKLOG     					10

typedef struct {
	pthread_t id;
	int terminated;
} ThreadInfo;

typedef struct {
    int listen_fd;
	int client_fd;
	char *client_ip;
	ThreadInfo *thread;
	int n_threads;
	pthread_mutex_t mutex;
	pthread_mutex_t client_fd_mutex;
	pthread_mutex_t *mutex_print;
	BidirectionalList clients;
	int n_clients;
} Server;

typedef struct {
    IluvatarSon *iluvatar;
	Server *server;
} ServerIluvatar;

/*********************************************************************
* @Purpose: Initializes a server opening its passive socket.
* @Params: in: ip = IP address of the host.
*          in: port = port of the host.
* @Return: Returns an instance of Server.
*********************************************************************/
Server SERVER_init(char *ip, int port);

/*********************************************************************
* @Purpose: Runs an initialized Arda server.
* @Params: in/out: arda = instance of Arda containing the name the
*                  IP address, the port and the directory of the
*				   server.
*		   in/out: server = initialized instance of Server. 
* @Return: ----
*********************************************************************/
void SERVER_runArda(Arda *arda, Server *server);

/*********************************************************************
* @Purpose: Runs an initialized IluvatarSon passive socket.
* @Params: in/out: iluvatarSon = instance of IluvatarSon
*		   in/out: server = initialized instance of Server
*          in/out: mutex_print = screen mutex to prevent writing to
*                  screen simultaneously
* @Return: ----
*********************************************************************/
void SERVER_runIluvatar(IluvatarSon *iluvatarSon, Server *server, pthread_mutex_t *mutex_print); 

/*********************************************************************
* @Purpose: Runs an initialized Arda server.
* @Params: in/out: server = instance of Server.
* @Return: ----
*********************************************************************/
void SERVER_close(Server *server);

#endif
