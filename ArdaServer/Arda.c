/*********************************************************************
* @Purpose: Runs the server for the sons of Iluvatar.
* @Authors: Angel Garcia Gascon
*           Claudia Lajara Silvosa
* @Date:
* @Last change: 09/12/2022
*********************************************************************/
#define _GNU_SOURCE 1
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <strings.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <pthread.h>

#include "../sharedFunctions.h"
#include "../bidirectionallist.h"

#define DISCONNECT_ARDA_MSG         	"\nDisconnecting Arda from all Iluvatar's children\n"
#define CLOSING_ARDA_MSG            	"Closing server\n"
#define WELCOME_MSG                 	"\nARDA SERVER\n"
#define READING_FILE_MSG            	"Reading configuration file\n"
#define WAITING_CONNECTIONS_MSG     	"Waiting for connections...\n\n"
#define ERROR_BINDING_SOCKET_MSG    	"ERROR: Arda could not bind the server socket\n"
#define ERROR_LISTENING_MSG         	"ERROR: Arda could not make the listen\n"
#define ERROR_ACCEPTING_MSG	        	"ERROR: Arda could not accept the connection request\n"
#define ERROR_CREATING_THREAD_MSG   	"ERROR: Arda could not create the thread\n"
#define ERROR_TYPE_NOT_IMPLEMENTED_MSG	"ERROR: That type of frame has not been implemented yet\n"
#define NEW_LOGIN_MSG                   "New login: %s, IP: %s, port: %d, PID %d\n"
#define UPDATING_LIST_MSG               "Updating user's list\n"
#define SENDING_LIST_MSG                "Sending user's list\n"
#define RENPONSE_SENT_LIST_MSG          "Response sent\n\n"
#define PETITION_UPDATE_MSG             "New petition: %s demands the user's list\nSending user's list to %s\n\n"
#define PETITION_EXIT_MSG               "New exit petition: %s has left Arda\n"
#define ERROR_N_ARGS_MSG            	"ERROR: not enough arguments\n"

#define ARDA_OK	    					0
#define ARDA_KO	    					-1
#define MIN_N_ARGS						2
#define BACKLOG     					10
#define FD_NOT_FOUND                    -1

Arda arda;
int listenFD = FD_NOT_FOUND;
BidirectionalList blist;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/*********************************************************************
* @Purpose: Closes all the file descriptors of the clients.
* @Params: ----
* @Return: ----
*********************************************************************/
void closeAllClientFD() {
    int clientFD;

    BIDIRECTIONALLIST_goToHead(&blist);

    while (BIDIRECTIONALLIST_isValid(blist)) {
        clientFD = BIDIRECTIONALLIST_get(&blist).clientFD;
        close(clientFD);
        BIDIRECTIONALLIST_next(&blist);
    } 
}

/*********************************************************************
* @Purpose: Free memory when SIGINT received.
* @Params: ----
* @Return: ----
*********************************************************************/
void sigIntHandler() {   
    if (listenFD != FD_NOT_FOUND) {
        close(listenFD);
    }

    closeAllClientFD(); 
    BIDIRECTIONALLIST_destroy(&blist);
    SHAREDFUNCTIONS_freeArda(&arda);
    
    printMsg(DISCONNECT_ARDA_MSG);
    printMsg(CLOSING_ARDA_MSG);
    signal(SIGINT, SIG_DFL);
    raise(SIGINT);
}

/*********************************************************************
* @Purpose: Creates an empty Arda server.
* @Params: ----
* @Return: Returns an initialized of the server Arda.
*********************************************************************/
Arda newArda() {
    Arda arda;
	arda.directory = NULL;
    arda.port = 0;
	arda.ip_address = NULL;
	return (arda);
}

/*********************************************************************
* @Purpose: Reads an Arda from a given file.
* @Params: in: filename = string with the name of the file
*          in/out: arda = pointer to Arda referencing
*		                      the Arda to fill with data
* @Return: Returns ARDA_OK if no errors when reading the file, 
*          otherwise ARDA_KO
*********************************************************************/
int readArda(char *filename, Arda *arda) {
    char *buffer = NULL;
	int error = ARDA_KO;
	int fd = open(filename, O_RDONLY);

	if (fd > 0) {
	    // Arda ip
        arda->ip_address = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // Arda port
        buffer = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		arda->port = atoi(buffer);
		free(buffer);
		// directory
		arda->directory = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		// no errors
		error = ARDA_OK;
		close(fd);
	}

	return (error);
}

/*********************************************************************
* @Purpose: Creates the thread for the client that has connected.
* @Params: in: c_fd = file descriptor of the client
* @Return: ----
*********************************************************************/
void *threadClient(void *c_fd) {
    int clientFD = *((int *) c_fd);
    char type = 0x07;
    char *header = NULL;
    char *data = NULL;
    int checked = 0;
    char *buffer = NULL;
    Element element;

    while (1) {
        SHAREDFUNCTIONS_readFrame(clientFD, &type, &header, &data);

        switch (type) {
            // Connection request
            case 0x01:
				// get username, IP, port and PID
				parseUserFromFrame(data, &element);
                // get clientFD
				element.clientFD = clientFD;
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
                pthread_mutex_lock(&mutex);
                // Adding the client to the list (critical region)
                BIDIRECTIONALLIST_addAfter(&blist, element);
                pthread_mutex_unlock(&mutex);

                printMsg(SENDING_LIST_MSG);
                // Write connexion frame
                if (blist.error == LIST_NO_ERROR) {
					data = SHAREDFUNCTIONS_getUsersFromList(blist);
					SHAREDFUNCTIONS_writeFrame(clientFD, 0x01, GPC_HEADER_CONOK, data);   
                } else {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 0x01, GPC_HEADER_CONKO, NULL);
                }
                
                printMsg(RENPONSE_SENT_LIST_MSG);
                break;
            
            // Update list petition            
            case 0x02:
                // data is the username
                asprintf(&buffer, PETITION_UPDATE_MSG, data, data);
                printMsg(buffer);
                free(buffer);

                // We don't need to apply mutex because the list is not modified
				data = SHAREDFUNCTIONS_getUsersFromList(blist);
                SHAREDFUNCTIONS_writeFrame(clientFD, 0x02, GPC_UPDATE_USERS_HEADER_OUT, data);     
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
                pthread_mutex_lock(&mutex);
                // Critical region
                BIDIRECTIONALLIST_goToHead(&blist);
                // 1 - Searching the client
                while (strcmp(BIDIRECTIONALLIST_get(&blist).username, data) != 0) {
                    BIDIRECTIONALLIST_next(&blist);
                    checked = 1;
                }

                if (checked) {
                    // 2 - Removing the client (critical region)
                    BIDIRECTIONALLIST_remove(&blist);
                }                
                pthread_mutex_unlock(&mutex); 
                
                printMsg(SENDING_LIST_MSG);
                // 3 - writing the exit frame
                //if ((blist.error == LIST_NO_ERROR) && checked) {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 0x06, GPC_HEADER_CONOK, NULL);             
                //} else {
                //    SHAREDFUNCTIONS_writeFrame(clientFD, 0x06, GPC_HEADER_CONKO, NULL);
                //}             
                printMsg(RENPONSE_SENT_LIST_MSG);

                // 4 - Closing the client connection
				if (NULL != data) {
				    free(data);
				    data = NULL;
				}
                close(clientFD);
                return NULL;
            // Wrong type
            default:
                // Send error frame
                SHAREDFUNCTIONS_writeFrame(clientFD, 0x07, GPC_UNKNOWN_CMD_HEADER, NULL);
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

int main(int argc, char* argv[]) {
    int clientFD = FD_NOT_FOUND;
    char *buffer = NULL;
    int read_ok = ARDA_KO;
    pthread_t thread;
    struct sockaddr_in server;
    
    if (MIN_N_ARGS > argc) {
        printMsg(COLOR_RED_TXT);
		printMsg(ERROR_N_LESS_ARGS_MSG);
        printMsg(COLOR_DEFAULT_TXT);
		return (-1);
    } else if (MIN_N_ARGS < argc) {
        printMsg(COLOR_RED_TXT);
		printMsg(ERROR_N_MORE_ARGS_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        return (-1);
    }

    // Mantaining stable the program when Ctrl + C occurs
    signal(SIGINT, sigIntHandler);
    
    arda = newArda();

    printMsg(WELCOME_MSG);
    printMsg(READING_FILE_MSG);
	read_ok = readArda(argv[1], &arda);
    
	if (ARDA_KO == read_ok) {
        printMsg(COLOR_RED_TXT);
        asprintf(&buffer, ERROR_OPENING_FILE, argv[1]);
        printMsg(buffer);
        free(buffer);
        SHAREDFUNCTIONS_freeArda(&arda);
        return (-1);
    }
    
    // Creating the server socket
    if ((listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_CREATING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        return (-1);
    }

    // Clear and assign the values to the server structure
    bzero(&server, sizeof(server));
    server.sin_port = htons(arda.port);
    server.sin_family = AF_INET;
	
	if (inet_pton(AF_INET, arda.ip_address, &server.sin_addr) < 0) {
	    printMsg("Error IP\n");
	}

    // Binding server socket (Assigning IP and port to the socket)
    if (bind(listenFD, (struct sockaddr*) &server, sizeof(server)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_BINDING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        close(listenFD);
        return (-1);
    }

    // Listening (Transforms the server active socket into a passive socket)
    if (listen(listenFD, BACKLOG) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_LISTENING_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        close(listenFD);
        return (-1);
    }

    // Creating the bidirectional list
    blist = BIDIRECTIONALLIST_create();
    printMsg(WAITING_CONNECTIONS_MSG);

    // Running the server
    while (1) {
        // Accept (Blocks the system until a connection request arrives)
        if ((clientFD = accept(listenFD, (struct sockaddr*) NULL, NULL)) < 0) {
            printMsg(COLOR_RED_TXT);
            printMsg(ERROR_ACCEPTING_MSG);
            printMsg(COLOR_DEFAULT_TXT);
        } else {
            // Creating the thread
            if (pthread_create(&thread, NULL, threadClient, &clientFD) != 0) {
                printMsg(COLOR_RED_TXT);
                printMsg(ERROR_CREATING_THREAD_MSG);
                printMsg(COLOR_DEFAULT_TXT);
                SHAREDFUNCTIONS_freeArda(&arda);
                close(listenFD);
                closeAllClientFD();
                BIDIRECTIONALLIST_destroy(&blist);
                return (-1);
            }
        }
    }

    return (0);
}
