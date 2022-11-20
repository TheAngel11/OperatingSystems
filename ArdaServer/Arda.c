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

#define DISCONNECT_ARDA_MSG         "Disconnecting Arda from all Iluvatar’s children\n"
#define CLOSING_ARDA_MSG            "Closing server\n"
#define WELCOME_MSG                 "ARDA SERVER\n"
#define READING_FILE_MSG            "Reading configuration file\n"
#define WAITING_CONNECTIONS_MSG     "Waiting for connections...\n"
#define ERROR_CREATING_SOCKET_MSG   "ERROR: Arda could not create the server socket\n"
#define ERROR_BINDING_SOCKET_MSG    "ERROR: Arda could not bind the server socket\n"
#define ERROR_LISTENING_MSG         "ERROR: Arda could not make the listen\n"
#define ERROR_LISTENING_MSG         "ERROR: Arda could not make the listen\n"
#define ERROR_ACCEPTING_MSG	        "ERROR: Arda could not accept the connection request\n"
#define ERROR_CREATING_THREAD_MSG   "ERROR: Arda could not create the thread\n"
#define ERROR_N_ARGS_MSG            "ERROR: not enough arguments\n"

#define ARDA_OK	    0		
#define ARDA_KO	    -1
#define MIN_N_ARGS	2
#define BACKLOG     10

Arda arda;
int listenFD;
BidirectionalList blist;

/*********************************************************************
* @Purpose: Free memory when SIGINT received.
* @Params: ----
* @Return: ----
*********************************************************************/
void sigHandler(){
    close(listenFD);
    // TODO: fer un close de tots els clients fd i acaba els threads
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
* @Params: in: fd = file descriptor of the client
* @Return: ----
*********************************************************************/
void * threadClient(void *element) {
    Element *list_info = (Element *) element;
    int clientFD = list_info->clientFD;

    // Adding the client to the list
    BIDIRECTIONALLIST_addAfter(&blist, *list_info);

    // Write connexion frame
    /*if (BIDIRECTIONALLIST_getErrorCode(blist) == LIST_NO_ERROR) {
        //SHAREDFUNCTIONS_writeFrame(clientFD, 1, "CONOK", );       //TODO: Enviar frame posant a data tota la info dels clients
    } else {
        SHAREDFUNCTIONS_writeFrame(clientFD, 1, "CONKO", NULL);
    }*/
    

    while(1) {
        int type = -1;
        char *header = NULL;
        char *data = NULL;
        SHAREDFUNCTIONS_readFrame(clientFD, &type, header, data);

        switch (type) {
            // Update list petition            
            case 2:
                //SHAREDFUNCTIONS_writeFrame(clientFD, 2, "LIST_RESPONSE", );       //TODO: Enviar frame posant a data tota la info dels clients
                break;
            
            case 3: case 4: case 5: case 8:
                // Types not implemented
                break;

            // Exit petition
            case 6:
                // Removing client from the list
                BIDIRECTIONALLIST_goToHead(&blist);
                // 1 - Searching the client
                while(BIDIRECTIONALLIST_get(&blist).clientFD != clientFD) {
                    BIDIRECTIONALLIST_next(&blist);
                }

                // 2 - Removing the client
                BIDIRECTIONALLIST_remove(&blist);
                
                // 3 - writing the exit frame
                /*if (BIDIRECTIONALLIST_getErrorCode(blist) == LIST_NO_ERROR) {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 6, "CONOK", NULL);
                
                } else {
                    SHAREDFUNCTIONS_writeFrame(clientFD, 6, "CONKO", NULL);
                }*/
                    
                
                

                // 4 - Closing the client connection
                close(clientFD);
                
                return NULL;

                break;

            default:
                // Send error frame
                SHAREDFUNCTIONS_writeFrame(clientFD, 7, "UNKNOWN", NULL);
                break;
        }
    }
    
    return NULL;
}

int main(int argc, char** argv){
    int clientFD = -1;
    char *buffer = NULL;
    int read_ok = ARDA_KO;
    pthread_t thread;
    Element element;
    struct sockaddr_in server;
    char *username = NULL;
    char *ip = NULL;
    int port;
    pid_t pid;
    arda = newArda();

    if(MIN_N_ARGS > argc){
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_N_ARGS_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        return -1;
    }

    // Mantaining stable the program when Ctrl + C occurs
    signal(SIGINT, sigHandler);

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
    if((listenFD = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_CREATING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        //TODO: Deixar tot estable abans de fer el return
        return -1;
    }

    // Clear and assign the values to the server structure
    bzero(&server, sizeof(server));
    server.sin_port = htons(arda.port);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);

    // Binding server socket (Assigning IP and port to the socket)
    if (bind(listenFD, (struct sockaddr*) &server, sizeof(server)) < 0){
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_BINDING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        //TODO: Deixar tot estable abans de fer el return
        return -1;
    }

    // Listening (Transforms the server active socket into a passive socket)
    if (listen(listenFD, BACKLOG) < 0){
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_LISTENING_MSG);
        printMsg(COLOR_DEFAULT_TXT);
        //TODO: Deixar tot estable abans de fer el return
        return -1;
    }

    // Creating the bidirectional list
    blist = BIDIRECTIONALLIST_create();
    // Running the server
    while(1) {
        printMsg(WAITING_CONNECTIONS_MSG);
        
        // Accept (Blocks the system until a connection request arrives)
        if ((clientFD = accept(listenFD, (struct sockaddr*) NULL, NULL)) < 0){
            printMsg(COLOR_RED_TXT);
            printMsg(ERROR_ACCEPTING_MSG);
            printMsg(COLOR_DEFAULT_TXT);
        } else {
            int type = -1;
            char *header = NULL;
            char *data = NULL;
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

                    // Creating the thread
                    if (pthread_create(&thread, NULL, threadClient, &element) != 0) {
                        printMsg(COLOR_RED_TXT);
                        printMsg(ERROR_CREATING_THREAD_MSG);
                        printMsg(COLOR_DEFAULT_TXT);
                        return -1;
                    }

                    pthread_join(thread, NULL);
                    break;

                //default:
                    // TODO: Send error message if in a connection request the type is not 1
            }
        }
    }

    return(0);
}