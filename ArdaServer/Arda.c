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

#define DISCONNECT_ARDA_MSG             "Disconnecting Arda from all Iluvatar’s children\n"
#define CLOSING_ARDA_MSG                "Closing server\n\n"
#define WELCOME_MSG                     "\nARDA SERVER\n"
#define READING_FILE_MSG                "Reading configuration file\n"
#define WAITING_CONNECTIONS_MSG         "Waiting for connections...\n\n"
#define ERROR_CREATING_SOCKET_MSG       "ERROR: Arda could not create the server socket\n"
#define ERROR_BINDING_SOCKET_MSG        "ERROR: Arda could not bind the server socket\n"
#define ERROR_LISTENING_MSG             "ERROR: Arda could not make the listen\n"
#define ERROR_LISTENING_MSG             "ERROR: Arda could not make the listen\n"
#define ERROR_ACCEPTING_MSG	            "ERROR: Arda could not accept the connection request\n"
#define ERROR_CREATING_THREAD_MSG       "ERROR: Arda could not create the thread\n"
#define ERROR_TYPE_NOT_IMPLEMENTED_MSG  "ERROR: That type of frame has not been implemented yet\n"
#define NEW_LOGIN_MSG                   "New login: %s, IP: %s, port: %d, PID %d\n"
#define UPDATING_LIST_MSG               "Updating user's list\n"
#define SENDING_LIST_MSG                "Sending user's list\n"
#define RENPONSE_SENT_LIST_MSG          "Response sent\n\n"
#define PETITION_UPDATE_MSG             "New petition: %s demands the user's list\nSending user's list to %s\n\n"
#define PETITION_EXIT_MSG               "New exit petition: %s has left Arda\n"
#define ARDA_OK	                        0		
#define ARDA_KO	                        1
#define MIN_N_ARGS	                    2
#define BACKLOG                         10
#define FD_NOT_FOUND                    -1

Arda arda;
int listenFD = FD_NOT_FOUND;
BidirectionalList blist;

/*********************************************************************
 * @Purpose: Closes all the file descriptors of the clients.
 * @Params: ----
 * @Return: ----
 * *********************************************************************/
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
void sigIntHandler(){    
    if(listenFD != FD_NOT_FOUND){
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
void * threadClient(void *c_fd) {
    int clientFD = *((int *) c_fd);
    int type = -1;
    char *header = NULL;
    char *data = NULL;
    char *username = NULL;
    char *ip = NULL;
    int port = 0;
    pid_t pid = 0;
    int checked = 0;
    char *buffer = NULL;
    Element element;    

    while(1) {
        type = -1;
        header = NULL;
        data = NULL;

        SHAREDFUNCTIONS_readFrame(clientFD, &type, header, data);

        switch (type) {
            // Connection request
            case 1:
                // Preparing the element to add to the list
                SHAREDFUNCTIONS_parseDataFieldConnection(data, username, ip, &port, &pid);
                element.clientFD = clientFD;
                element.username = username;
                element.ip_network = ip;
                element.port = port;
                element.pid = pid;
                element.clientFD = clientFD;

                // Printing the new login
                asprintf(&buffer, NEW_LOGIN_MSG, username, ip, port, pid);
                printMsg(buffer);
                free(buffer);

                printMsg(UPDATING_LIST_MSG);
                // Adding the client to the list
                BIDIRECTIONALLIST_addAfter(&blist, element);
                
                printMsg(SENDING_LIST_MSG);
                // Write connexion frame
                if (BIDIRECTIONALLIST_getErrorCode(blist) == LIST_NO_ERROR) {
                    data = SHAREDFUNCTIONS_writeDataFieldUpdate(blist);
                    SHAREDFUNCTIONS_writeFrame(clientFD, 1, CONOK, data);   
                } else {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 1, CONKO, NULL);
                }
                
                printMsg(RENPONSE_SENT_LIST_MSG);
                break;
            
            // Update list petition            
            case 2:
                // data is the username
                asprintf(&buffer, PETITION_UPDATE_MSG, data, data);
                printMsg(buffer);
                free(buffer);
                
                data = SHAREDFUNCTIONS_writeDataFieldUpdate(blist);
                SHAREDFUNCTIONS_writeFrame(clientFD, 2, LIST_RESPONSE, data);     
                break;
            
            // Types not implemented yet
            case 3: case 4: case 5: case 8:
                printMsg(COLOR_RED_TXT);
		        printMsg(ERROR_TYPE_NOT_IMPLEMENTED_MSG);
                printMsg(COLOR_DEFAULT_TXT);
                break;

            // Exit petition
            case 6:
                // data is the username
                asprintf(&buffer, PETITION_EXIT_MSG, data);
                printMsg(buffer);
                free(buffer);
                
                printMsg(UPDATING_LIST_MSG);
                // Removing client from the list
                BIDIRECTIONALLIST_goToHead(&blist);
                // 1 - Searching the client
                while(strcmp(BIDIRECTIONALLIST_get(&blist).username, data) != 0) {
                    BIDIRECTIONALLIST_next(&blist);
                    checked = 1;
                }

                if(checked) {
                    // 2 - Removing the client
                    BIDIRECTIONALLIST_remove(&blist);
                }

                printMsg(SENDING_LIST_MSG);
                // 3 - writing the exit frame
                if ((BIDIRECTIONALLIST_getErrorCode(blist) == LIST_NO_ERROR) && checked) {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 6, CONOK, NULL);             
                } else {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 6, CONKO, NULL);
                }             
                printMsg(RENPONSE_SENT_LIST_MSG);

                // 4 - Closing the client connection
                close(clientFD);
                return NULL;
                break;
            
            // Wrong type
            default:
                // Send error frame
                SHAREDFUNCTIONS_writeFrame(clientFD, 7, UNKNOWN, NULL);
                break;
        }
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    int clientFD = FD_NOT_FOUND;
    char *buffer = NULL;
    int read_ok = ARDA_KO;
    pthread_t thread;
    struct sockaddr_in server;
    
    if(MIN_N_ARGS > argc) {
        printMsg(COLOR_RED_TXT);
		printMsg(ERROR_N_LESS_ARGS_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        return -1;

    } else if(MIN_N_ARGS < argc) {
        printMsg(COLOR_RED_TXT);
		printMsg(ERROR_N_MORE_ARGS_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        return -1;
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
        return -1;
    }
    
    // Creating the server socket
    if((listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_CREATING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        return -1;
    }

    // Clear and assign the values to the server structure
    bzero(&server, sizeof(server));
    server.sin_port = htons(arda.port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding server socket (Assigning IP and port to the socket)
    if (bind(listenFD, (struct sockaddr*) &server, sizeof(server)) < 0) {
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_BINDING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        close(listenFD);
        return -1;
    }

    // Listening (Transforms the server active socket into a passive socket)
    if (listen(listenFD, BACKLOG) < 0){
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_LISTENING_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        SHAREDFUNCTIONS_freeArda(&arda);
        close(listenFD);
        return -1;
    }

    // Creating the bidirectional list
    blist = BIDIRECTIONALLIST_create();

    // Running the server
    while(1) {
        printMsg(WAITING_CONNECTIONS_MSG);
        
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
                return -1;
            }
        }
    }

    return(0);
}
