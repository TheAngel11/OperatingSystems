/*********************************************************************
* @Purpose: Module that contains functions to manage a dedicated
*           server with sockets.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 10/12/2022
* @Last change: 11/12/2022
*********************************************************************/
#include "server.h"

/*********************************************************************
* @Purpose: Initializes a server opening its passive socket.
* @Params: in: ip = IP address of the host.
*          in: port = port of the host.
* @Return: Returns an instance of Server.
*********************************************************************/
Server SERVER_init(char *ip, int port) {
    Server s;
	struct sockaddr_in server;

	// init Server
	s.listen_fd = FD_NOT_FOUND;
	s.client_fd = FD_NOT_FOUND;
	s.thread = NULL;
	s.n_threads = 0;
	pthread_mutex_init(&s.mutex, NULL);
	s.n_clients = 0;

    // Creating the server socket
    if ((s.listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_CREATING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        return (s);
    }

    // Clear and assign the values to the server structure
    bzero(&server, sizeof(server));
    server.sin_port = htons(port);
    server.sin_family = AF_INET;
	
	if (inet_pton(AF_INET, ip, &server.sin_addr) < 0) {
	    printMsg(ERROR_IP_CONFIGURATION_MSG);
		close(s.listen_fd);
		s.listen_fd = FD_NOT_FOUND;
		return (s);
	}

    // Binding server socket (Assigning IP and port to the socket)
    if (bind(s.listen_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_BINDING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        close(s.listen_fd);
		s.listen_fd = FD_NOT_FOUND;
        return (s);
    }

    // Listening (Transforms the server active socket into a passive socket)
    if (listen(s.listen_fd, BACKLOG) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_LISTENING_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        close(s.listen_fd);
		s.listen_fd = FD_NOT_FOUND;
		return (s);
    }

	s.clients = BIDIRECTIONALLIST_create();

	return (s);
}

/*********************************************************************
* @Purpose: Closes all the file descriptors of the clients.
* @Params: ----
* @Return: ----
*********************************************************************/
void closeAllClientFDs(Server *server) {
	Element e;

    BIDIRECTIONALLIST_goToHead(&server->clients);

    while (BIDIRECTIONALLIST_isValid(server->clients)) {
        e = BIDIRECTIONALLIST_get(&server->clients);
        close(e.clientFD);
        BIDIRECTIONALLIST_next(&server->clients);
		free(e.username);
		e.username = NULL;
		free(e.ip_network);
		e.ip_network = NULL;
    }
}

/*********************************************************************
* @Purpose: Creates the thread for the client that has connected.
* @Params: in: args = arguments to pass to thread.
* @Return: ----
*********************************************************************/
void *ardaClient(void *args) {
    Server *s = (Server *) args;
    char type = 0x07;
    char *header = NULL;
    char *data = NULL;
    char *buffer = NULL;
    Element element;

    while (1) {
        GPC_readFrame(s->client_fd, &type, &header, &data);

        switch (type) {
            // Connection request
            case 0x01:
				// get username, IP, port and PID
				GPC_parseUserFromFrame(data, &element);
                // get clientFD
				element.clientFD = s->client_fd;
				free(data);
				data = NULL;

                // Printing the new login
                asprintf(&buffer, NEW_LOGIN_MSG, element.username, element.ip_network, element.port, element.pid);
                printMsg(buffer);
                free(buffer);
				buffer = NULL;

                printMsg(UPDATING_LIST_MSG);

                // We check with mutual exclusion that only 1 process is added to the list at the same time
                // if there are 2 or more users connecting at the same time
                pthread_mutex_lock(&s->mutex);
                // Adding the client to the list (critical region)
                BIDIRECTIONALLIST_addAfter(&s->clients, element);
                pthread_mutex_unlock(&s->mutex);
				free(element.username);
				element.username = NULL;
				free(element.ip_network);
				element.ip_network = NULL;

                printMsg(SENDING_LIST_MSG);
                // Write connexion frame
                if (s->clients.error == LIST_NO_ERROR) {
					data = GPC_getUsersFromList(s->clients);
					GPC_writeFrame(s->client_fd, 0x01, GPC_HEADER_CONOK, data); 
					free(data);
					data = NULL;
                } else {
                    GPC_writeFrame(s->client_fd, 0x01, GPC_HEADER_CONKO, NULL);
                }
                
                printMsg(RESPONSE_SENT_LIST_MSG);
                break;
            
            // Update list petition
            case 0x02:
                // data is the username
                asprintf(&buffer, PETITION_UPDATE_MSG, data, data);
                printMsg(buffer);
                free(buffer);
				buffer = NULL;
				free(data);
				data = NULL;

                // We don't need to apply mutex because the list is not modified
				data = GPC_getUsersFromList(s->clients);
                GPC_writeFrame(s->client_fd, 0x02, GPC_UPDATE_USERS_HEADER_OUT, data);     
                break;
            
            // Types not implemented yet
            case 0x03: case 0x04: case 0x05: case 0x08:
                printMsg(COLOR_RED_TXT);
		        printMsg(ERROR_TYPE_NOT_IMPLEMENTED_MSG);
                printMsg(COLOR_DEFAULT_TXT);
                break;

            // Exit petition
            case 0x06:
                // data is the username
                asprintf(&buffer, PETITION_EXIT_MSG, data);
                printMsg(buffer);
                free(buffer);
				buffer = NULL;
                
                printMsg(UPDATING_LIST_MSG);
                // Removing client from the list

                // We check with mutual exclusion that only 1 process is removed to the list at the same time
                // if there are 2 or more users disconnecting at the same time
                pthread_mutex_lock(&s->mutex);
                // Critical region
                BIDIRECTIONALLIST_goToHead(&s->clients);
                // 1 - Searching the client
				element = BIDIRECTIONALLIST_get(&s->clients);
                
				while (strcmp(element.username, data) != 0) {
                    BIDIRECTIONALLIST_next(&s->clients);
					free(element.username);
					element.username = NULL;
					free(element.ip_network);
					element.ip_network = NULL;
					element = BIDIRECTIONALLIST_get(&s->clients);
                }
				
				free(element.username);
				element.username = NULL;
				free(element.ip_network);
				element.ip_network = NULL;
                // 2 - Removing the client (critical region)
                BIDIRECTIONALLIST_remove(&s->clients);
                pthread_mutex_unlock(&s->mutex);
                printMsg(SENDING_LIST_MSG);

                // 3 - writing the exit frame
                if (s->clients.error == LIST_NO_ERROR) {
                    GPC_writeFrame(s->client_fd, 0x06, GPC_HEADER_CONOK, NULL);             
                } else {
                    GPC_writeFrame(s->client_fd, 0x06, GPC_HEADER_CONKO, NULL);
                }             
                printMsg(RESPONSE_SENT_LIST_MSG);

                // 4 - Closing the client connection
				free(header);
				header = NULL;
				if (NULL != data) {
				    free(data);
				    data = NULL;
				}
                pthread_mutex_lock(&s->mutex);
				(s->n_clients)--;
                pthread_mutex_unlock(&s->mutex);
				close(s->client_fd);
                return NULL;
            // Unknown command
            default:
                // When UNKNOWN_COMMAND do nothing (message already shown in IluvatarSon)
                break;
        }

		if (NULL != data) {
		    free(data);
			data = NULL;
		}
		if (NULL != header) {
		    free(header);
			header = NULL;
		}
        
		type = 0x07;
    }
    
    return NULL;
}

/*********************************************************************
* @Purpose: Runs an initialized Arda server.
* @Params: in/out: arda = instance of Arda containing the name the
*                  IP address, the port and the directory of the
*				   server.
*		   in/out: server = initialized instance of Server.
* @Return: ----
*********************************************************************/
void SERVER_runArda(Arda *arda, Server *server) {
	while (1) {
	    // Accept (Blocks the system until a connection request arrives)
		if ((server->client_fd = accept(server->listen_fd, (struct sockaddr *) NULL, NULL)) < 0) {
            printMsg(COLOR_RED_TXT);
			printMsg(ERROR_ACCEPTING_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			return;
		} else {
		    //if (server->n_clients > 0) {
			if (server->n_threads > 0) {
			    // add thread to array
				(server->n_clients)++;
				(server->n_threads)++;
//				server->thread = (pthread_t *) realloc (server->thread, sizeof(pthread_t) * server->n_clients);
				server->thread = (pthread_t *) realloc (server->thread, sizeof(pthread_t) * server->n_threads);
			} else {
			    // init threads array
				if (NULL != server->thread) {
				    free(server->thread);
					server->thread = NULL;
				}

				(server->n_clients)++;
				(server->n_threads)++;
				server->thread = (pthread_t *) malloc (sizeof(pthread_t));
			}

			// Creating the thread
			if (pthread_create(&server->thread[server->n_clients - 1], NULL, ardaClient, server) != 0) {
			    printMsg(COLOR_RED_TXT);
				printMsg(ERROR_CREATING_THREAD_MSG);
				printMsg(COLOR_DEFAULT_TXT);
				// free memory
				SHAREDFUNCTIONS_freeArda(arda);
				free(server->thread);
				server->thread = NULL;
				// close FDs
				close(server->listen_fd);
				closeAllClientFDs(server);
				BIDIRECTIONALLIST_destroy(&server->clients);
				return;
			}
		}
	}

	pthread_mutex_destroy(&server->mutex);
}

/*********************************************************************
* @Purpose: Runs an initialized Arda server.
* @Params: in/out: server = instance of Server.
* @Return: ----
*********************************************************************/
void SERVER_close(Server *server) {
    int i = 0;

	closeAllClientFDs(server);

    if (server->listen_fd != FD_NOT_FOUND) {
        close(server->listen_fd);
    }

    BIDIRECTIONALLIST_destroy(&server->clients);
	
	for (i = 0; i < server->n_threads; i++) {
	    pthread_detach(server->thread[i]);
		pthread_cancel(server->thread[i]);
	}

	pthread_mutex_destroy(&server->mutex);
}
