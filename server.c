/*********************************************************************
* @Purpose: Module that contains functions to manage a dedicated
*           server with sockets.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 10/12/2022
* @Last change: 09/01/2023
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
	pthread_mutex_init(&s.client_fd_mutex, NULL);
	s.mutex_print = NULL;
	s.n_clients = 0;
	s.clients = BIDIRECTIONALLIST_create();

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
	    printMsg(COLOR_RED_TXT);
	    printMsg(ERROR_IP_CONFIGURATION_MSG);
		printMsg(COLOR_DEFAULT_TXT);
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
    }

	return (s);
}

/*********************************************************************
* @Purpose: Closes all the file descriptors of the clients.
* @Params: in/out: server = instance of Server
* @Return: ----
*********************************************************************/
void closeAllClientFDs(Server *server) {
	Element e;
	if (!BIDIRECTIONALLIST_isEmpty(server->clients)) {
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
}

/*********************************************************************
* @Purpose: Sends Connection Request reply.
* @Params: in/out: server = instance of Server
*          in/out: data = string with data from connection frame
*          in: client_fd = file descriptor of the client
* @Return: ----
*********************************************************************/
void answerConnectionRequest(Server *s, char **data, int client_fd) {
    Element element;
	char *buffer = NULL;

	// get username, IP, port and PID
	GPC_parseUserFromFrame(*data, &element);
	// get clientFD
	element.clientFD = client_fd;
	free(*data);
	*data = NULL;

	// Printing the new login
	asprintf(&buffer, NEW_LOGIN_MSG, element.username, element.ip_network, element.port, element.pid);
	pthread_mutex_lock(s->mutex_print);
	printMsg(buffer);
	pthread_mutex_unlock(s->mutex_print);
	free(buffer);
	buffer = NULL;
	pthread_mutex_lock(s->mutex_print);
	printMsg(UPDATING_LIST_MSG);
	pthread_mutex_unlock(s->mutex_print);
	// mutual exclusion so that only 1 process is added to the list at the same time
	pthread_mutex_lock(&s->mutex);
	
	// add the client to the list (critical region)
	if(BIDIRECTIONALLIST_isEmpty(s->clients)) {
		BIDIRECTIONALLIST_goToHeadPhantom(&s->clients);
	}
	
	BIDIRECTIONALLIST_addAfter(&s->clients, element);
	pthread_mutex_unlock(&s->mutex);
	free(element.username);
	element.username = NULL;
	free(element.ip_network);
	element.ip_network = NULL;
	pthread_mutex_lock(s->mutex_print);
	printMsg(SENDING_LIST_MSG);
	pthread_mutex_unlock(s->mutex_print);
	
	// Write connexion frame
	if (s->clients.error == LIST_NO_ERROR) {
		buffer = GPC_getUsersFromList(s->clients);
		GPC_writeFrame(client_fd, 0x01, GPC_HEADER_CONOK, buffer, strlen(buffer)); 
		free(buffer);
		buffer = NULL;
	} else {
	    GPC_writeFrame(client_fd, 0x01, GPC_HEADER_CONKO, NULL, 0); 
	}

	pthread_mutex_lock(s->mutex_print);
	printMsg(RESPONSE_SENT_LIST_MSG);
	printMsg(COLOR_DEFAULT_TXT);
	pthread_mutex_unlock(s->mutex_print);
}

/*********************************************************************
* @Purpose: Sends List Petition reply.
* @Params: in/out: server = instance of Server
*          in/out: data = string with data from update users frame
*          in: client_fd = file descriptor of the client
* @Return: ----
*********************************************************************/
void answerListPetition(Server *s, char **data, int client_fd) {
    char *buffer = NULL;

	// data is the username
	asprintf(&buffer, PETITION_UPDATE_MSG, *data, *data);
	pthread_mutex_lock(s->mutex_print);
	printMsg(buffer);
	pthread_mutex_unlock(s->mutex_print);
	free(buffer);
	buffer = NULL;
	free(*data);
	*data = NULL;
	// We don't need to apply mutex because the list is not modified
	buffer = GPC_getUsersFromList(s->clients);
	GPC_writeFrame(client_fd, 0x02, GPC_UPDATE_USERS_HEADER_OUT, buffer, strlen(buffer));
	free(buffer);
	buffer = NULL;
}

/*********************************************************************
* @Purpose: Sends Exit Petition reply.
* @Params: in/out: server = instance of Server
*          in/out: data = string with data from exit frame
*          in: client_fd = file descriptor of the client
* @Return: ----
*********************************************************************/
void answerExitPetition(Server *s, char **data, int client_fd) {
    Element element;
	char *buffer = NULL;
	int is_empty = 0;
	int found = 0;

	// data is the username
	asprintf(&buffer, PETITION_EXIT_MSG, *data);
	pthread_mutex_lock(s->mutex_print);
	printMsg(buffer);
	pthread_mutex_unlock(s->mutex_print);
	free(buffer);
	buffer = NULL;
	pthread_mutex_lock(s->mutex_print);
	printMsg(UPDATING_LIST_MSG);
	pthread_mutex_unlock(s->mutex_print);
	// Removing client from the list
	pthread_mutex_lock(&s->mutex);
	
	// Critical region
	if (!BIDIRECTIONALLIST_isEmpty(s->clients)) {
		BIDIRECTIONALLIST_goToHead(&s->clients);
		// search client			
		while (BIDIRECTIONALLIST_isValid(s->clients) && !found) {
			element = BIDIRECTIONALLIST_get(&s->clients);
			
			if (strcmp(element.username, *data) == 0) {
				found = 1;
			} else {
				BIDIRECTIONALLIST_next(&s->clients);
			}

			free(element.username);
			element.username = NULL;
			free(element.ip_network);
			element.ip_network = NULL;
		}
		
		// remove client (critical region)
		BIDIRECTIONALLIST_remove(&s->clients);
	} else {
		is_empty = 1;
	}

	pthread_mutex_unlock(&s->mutex);
	pthread_mutex_lock(s->mutex_print);
	printMsg(COLOR_DEFAULT_TXT);
	printMsg(SENDING_LIST_MSG);
	pthread_mutex_unlock(s->mutex_print);

	// write exit frame
	if ((s->clients.error == LIST_NO_ERROR) && !is_empty && found) {
	    GPC_writeFrame(client_fd, 0x06, GPC_HEADER_CONOK, NULL, 0);             
	} else {
	    GPC_writeFrame(client_fd, 0x06, GPC_HEADER_CONKO, NULL, 0);
	}

	pthread_mutex_lock(s->mutex_print);
	printMsg(RESPONSE_SENT_LIST_MSG);
	printMsg(COLOR_DEFAULT_TXT);
	pthread_mutex_unlock(s->mutex_print);
	
	// close the client connection
	if (NULL != *data) {
	    free(*data);
	    *data = NULL;
	}

	pthread_mutex_lock(&s->mutex);
	(s->n_clients)--;
	pthread_mutex_unlock(&s->mutex);
	close(client_fd);
}

/*********************************************************************
* @Purpose: Creates the thread for a new client that has connected to
*           an Arda server.
* @Params: in: args = arguments to pass to thread
* @Return: ----
*********************************************************************/
void *ardaClient(void *args) {
    Server *s = (Server *) args;
    char type = 0x07;
    char *header = NULL;
    char *data = NULL;
	int client_fd = s->client_fd;
	int index_thread = s->n_threads - 1;
	
	pthread_mutex_unlock(&s->client_fd_mutex);

    while (1) {
        GPC_readFrame(client_fd, &type, &header, &data);

        switch (type) {
            // Connection request
            case 0x01:
				answerConnectionRequest(s, &data, client_fd);
                break;
            
            // Update list petition
            case 0x02:
                answerListPetition(s, &data, client_fd);
                break;
            
            // New message has been sent
            case 0x08:
				pthread_mutex_lock(s->mutex_print);
                printMsg(COLOR_RED_TXT);
		        printMsg(ERROR_TYPE_NOT_IMPLEMENTED_MSG);
                printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(s->mutex_print);
                break;

            // Exit petition
            case 0x06:
                answerExitPetition(s, &data, client_fd);
				if(header != NULL) {
					free(header);
					header = NULL;
				}
				pthread_mutex_lock(&s->mutex);
				s->thread[index_thread].terminated = 1;
				pthread_mutex_unlock(&s->mutex);
				// Close thread
				pthread_cancel(s->thread[index_thread].id);
				pthread_detach(s->thread[index_thread].id);
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
* @Purpose: Receives the message and sends a reply.
* @Params: in/out: server = instance of ServerIluvatar
*          in/out: data = string with data from send msg frame
* @Return: ----
*********************************************************************/
char answerSendMsg(ServerIluvatar *s, char **data) {
	char *buffer = NULL;
	char *origin_user = NULL;
	char *message = NULL;

	// parsing the message
	GPC_parseSendMessage(*data, &origin_user, &message);
	free(*data);
	*data = NULL;
			
	// Reply message petition
	if (message != NULL && /*strcmp(GPC_SEND_MSG_HEADER_IN, header) == 0 &&*/ s->server->clients.error == LIST_NO_ERROR) {
		// Send the OK frame
		GPC_writeFrame(s->server->client_fd, 0x03, GPC_HEADER_MSGOK, NULL, 0);
		// Print the message
		asprintf(&buffer, MSG_RECIEVED_MSG, origin_user, s->server->client_ip, message);
		pthread_mutex_lock(s->server->mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(s->server->mutex_print);
		free(buffer);
		buffer = NULL;
	} else {
		// Send the KO frame
		GPC_writeFrame(s->server->client_fd, 0x03, GPC_HEADER_MSGKO, NULL, 0);
		
		// free memory
		if (origin_user != NULL) {
		    free(origin_user);
			origin_user = NULL;
		}

		if (message != NULL) {
		    free(message);
			message = NULL;
		}

		return (0);
	}
			
	// free memory
	if (origin_user != NULL) {
		free(origin_user);
		origin_user = NULL;
	}

	if (message != NULL) {
		free(message);
		message = NULL;
	}

	return (1);
}

/*********************************************************************
* @Purpose: Receives the file and sends a reply.
* @Params: in/out: server = instance of ServerIluvatar
*          in/out: data = string with data from send file initial frame
* @Return: ----
*********************************************************************/
char answerSendFile(ServerIluvatar *s, char **data) {
	char *buffer = NULL;
	char *filename = NULL;
	char *md5sum = NULL;
	char *path = NULL;
	char *origin_user = NULL;
	char *header = NULL;
	char type = 0x07;
	int file_size = 0;
	int file_fd = -1;

	// parsing the file information
	GPC_parseSendFileInfo(*data, &origin_user, &filename, &file_size, &md5sum);
	free(*data);
	*data = NULL;
	// create file to copy received file
	asprintf(&path, ".%s/%s", s->iluvatar->directory, filename);
	file_fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			
	while (file_size > GPC_FILE_MAX_BYTES) {
		// Read the frame
		GPC_readFrame(s->server->client_fd, &type, &header, &buffer);
		write(file_fd, buffer, GPC_FILE_MAX_BYTES);

		// free memory
		if (buffer != NULL) {
		    free(buffer);
			buffer = NULL;
		}

		free(header);
		header = NULL;
		// next fragment
		file_size -= GPC_FILE_MAX_BYTES;
	}

	// Read the last frame
	GPC_readFrame(s->server->client_fd, &type, &header, &buffer);
	write(file_fd, buffer, file_size);
		
	// free memory
	if (buffer != NULL) {
	    free(buffer);
		buffer = NULL;
	}

	free(header);
	header = NULL;
	// close file
	close(file_fd);
	// check the md5sum
	buffer = SHAREDFUNCTIONS_getMD5Sum(path);
	free(path);
	path = NULL;
	//TODO:debug
		pthread_mutex_lock(s->server->mutex_print);
	printMsg("OG MD5SUM: ");
	printMsg(md5sum);
	printMsg("\nnew MD5SUM: ");
	printMsg(buffer);
	printMsg("\n");
		pthread_mutex_unlock(s->server->mutex_print);
	//TODO:end
			
	if (strcmp(buffer, md5sum) == 0) {
		// Send OK frame
		GPC_writeFrame(s->server->client_fd, GCP_SEND_FILE_TYPE, GPC_SEND_FILE_HEADER_OK_OUT, NULL, 0);
		free(buffer);
		buffer = NULL;
		// Print the message
		asprintf(&buffer, FILE_RECIEVED_MSG, origin_user, s->server->client_ip, filename);
		pthread_mutex_lock(s->server->mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(s->server->mutex_print);
		free(buffer);
		buffer = NULL;
	} else {
		// Send KO frame
		GPC_writeFrame(s->server->client_fd, GCP_SEND_FILE_TYPE, GPC_SEND_FILE_HEADER_KO_OUT, NULL, 0);
		// free memory
		free(buffer);
		buffer = NULL;
		free(md5sum);
		md5sum = NULL;
		free(origin_user);
		origin_user = NULL;
		free(filename);
		filename = NULL;
		return (0);
	}
			
	// free memory
	free(buffer);
	buffer = NULL;
	free(md5sum);
	md5sum = NULL;
	free(origin_user);
	origin_user = NULL;
	free(filename);
	filename = NULL;

	return (1);
}

/*********************************************************************
* @Purpose: Creates the thread for the client that has connected to
*           an Iluvatar server.
* @Params: in: args = arguments to pass to thread
* @Return: ----
*********************************************************************/
void *iluvatarClient(void *args) {
    ServerIluvatar *s = (ServerIluvatar *) args;
    char type = 0x07;
    char *header = NULL;
    char *data = NULL;
	char *buffer = NULL;
	int received_OK = 1;
	
	pthread_mutex_unlock(&s->server->client_fd_mutex);
	// get frame
	GPC_readFrame(s->server->client_fd, &type, &header, &data);
	pthread_mutex_lock(s->server->mutex_print);
	// reset command line
	printMsg(COLOR_DEFAULT_TXT);
	pthread_mutex_unlock(s->server->mutex_print);

	switch (type) {            
		// Send message petition
		case GCP_SEND_MSG_TYPE:
			received_OK = answerSendMsg(s, &data);
			break;

		// Send file petition
		case GCP_SEND_FILE_TYPE:
			received_OK = answerSendFile(s, &data);
			break;

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

	if (received_OK) {
		// open again the command line
		asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		pthread_mutex_lock(s->server->mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(s->server->mutex_print);
		free(buffer);
		buffer = NULL;
	}

    return (NULL);
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
	pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;
	server->mutex_print = &mutex_print;

	while (1) {
		// We need a mutex to save the client_fd in the thread before it changes to the next client
		pthread_mutex_lock(&server->client_fd_mutex);
	    // Accept (Blocks the system until a connection request arrives)
		if ((server->client_fd = accept(server->listen_fd, (struct sockaddr *) NULL, NULL)) < 0) {
			pthread_mutex_lock(server->mutex_print);
            printMsg(COLOR_RED_TXT);
			printMsg(ERROR_ACCEPTING_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(server->mutex_print);
			return;
		} else {
			pthread_mutex_lock(&server->mutex);
			if (server->n_threads > 0) {
			    // add thread to array
				(server->n_clients)++;
				(server->n_threads)++;
				server->thread = (ThreadInfo *) realloc (server->thread, sizeof(ThreadInfo) * server->n_threads);
			} else {
			    // init threads array
				if (NULL != server->thread) {
				    free(server->thread);
					server->thread = NULL;
				}
				(server->n_clients)++;
				(server->n_threads)++;
				server->thread = (ThreadInfo *) malloc (sizeof(ThreadInfo));
			}

			server->thread[server->n_threads - 1].terminated = 0;
			// Creating the thread
			if (pthread_create(&server->thread[server->n_threads - 1].id, NULL, ardaClient, server) != 0) {
				pthread_mutex_unlock(&server->client_fd_mutex);
				pthread_mutex_lock(server->mutex_print);
			    printMsg(COLOR_RED_TXT);
				printMsg(ERROR_CREATING_THREAD_MSG);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(server->mutex_print);

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
			pthread_mutex_unlock(&server->mutex);
		}
	}

	pthread_mutex_destroy(&server->mutex);
}

/*********************************************************************
* @Purpose: Gets the IP address of the client given an fd.
* @Params: in: client_fd = file descriptor of the client
* @Return: IP address of the client.
*********************************************************************/
char *SERVER_getClientIP(int client_fd) {
	struct sockaddr_in addr; 
	
	socklen_t addr_size = sizeof(struct sockaddr_in);
	getpeername(client_fd, (struct sockaddr *)&addr, &addr_size);
	
	return inet_ntoa(addr.sin_addr);
}

/*********************************************************************
* @Purpose: Runs an initialized IluvatarSon passive socket.
* @Params: in/out: iluvatarSon = instance of IluvatarSon
*		   in/out: server = initialized instance of Server
* @Return: ----
*********************************************************************/
void SERVER_runIluvatar(IluvatarSon *iluvatarSon, Server *server, pthread_mutex_t *mutex_print) {
	ServerIluvatar serverIluvatar;
	serverIluvatar.iluvatar = iluvatarSon;
	serverIluvatar.server = server;
	serverIluvatar.server->mutex_print = mutex_print;

	while (1) {
		// I need a mutex to save the client_fd in the thread before it changes to the next client
		pthread_mutex_lock(&server->client_fd_mutex);
	    // Accept (Blocks the system until a connection request arrives)
		if ((server->client_fd = accept(server->listen_fd, (struct sockaddr *) NULL, NULL)) < 0) {
			pthread_mutex_lock(serverIluvatar.server->mutex_print);
            printMsg(COLOR_RED_TXT);
			printMsg(ERROR_ACCEPTING_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(serverIluvatar.server->mutex_print);
			return;
		}
		pthread_mutex_lock(&server->mutex);
		// Getting the client IP
		server->client_ip = SERVER_getClientIP(server->client_fd);
		if (server->n_threads > 0) {
			// add thread to array
			(server->n_clients)++;
			(server->n_threads)++;
			server->thread = (ThreadInfo *) realloc (server->thread, sizeof(ThreadInfo) * server->n_threads);
		} else {
			// init threads array
			if (NULL != server->thread) {
				free(server->thread);
				server->thread = NULL;
			}

			(server->n_clients)++;
			(server->n_threads)++;
			server->thread = (ThreadInfo *) malloc (sizeof(ThreadInfo));
		}

		server->thread[server->n_threads - 1].terminated = 0;
		// Creating the thread
		if (pthread_create(&server->thread[server->n_threads - 1].id, NULL, iluvatarClient, &serverIluvatar) != 0) {
			pthread_mutex_unlock(&server->client_fd_mutex);
			pthread_mutex_lock(serverIluvatar.server->mutex_print);
			printMsg(COLOR_RED_TXT);
			printMsg(ERROR_CREATING_THREAD_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(serverIluvatar.server->mutex_print);
			// free memory
			SHAREDFUNCTIONS_freeIluvatarSon(iluvatarSon);
			free(server->thread);
			server->thread = NULL;
			// close FDs
			close(server->listen_fd);
			closeAllClientFDs(server);
			BIDIRECTIONALLIST_destroy(&server->clients);
			return;
		}
		pthread_mutex_unlock(&server->mutex);
	}

	pthread_mutex_destroy(&server->mutex);
}

/*********************************************************************
* @Purpose: Closes an initialized Arda server.
* @Params: in/out: server = instance of Server.
* @Return: ----
*********************************************************************/
void SERVER_close(Server *server) {
    int i = 0;
	pthread_mutex_lock(&server->mutex);
	// Close all client FDs
	closeAllClientFDs(server);

    if (server->listen_fd != FD_NOT_FOUND) {
        close(server->listen_fd);
    }

    BIDIRECTIONALLIST_destroy(&server->clients);
	
	// We terminate and realease the resources of the not finished threads
	for (i = 0; i < server->n_threads; i++) {
		if(server->thread[i].terminated != 1) {
			pthread_cancel(server->thread[i].id);
			pthread_join(server->thread[i].id, NULL);
			pthread_detach(server->thread[i].id);
		}
	}
	pthread_mutex_unlock(&server->mutex);
	pthread_mutex_destroy(&server->mutex);
	pthread_mutex_destroy(&server->client_fd_mutex);
	
	if (NULL != server->mutex_print) {
	pthread_mutex_destroy(server->mutex_print);
	}
}
