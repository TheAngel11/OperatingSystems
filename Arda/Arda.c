#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#include "../sharedFunctions.h"

#define DISCONNECT_ARDA_MSG     "Disconnecting Arda from all Iluvatar’s children\n"
#define CLOSING_ARDA_MSG            "Closing server\n"
#define WELCOME_MSG                 "ARDA SERVER\n"
#define READING_FILE_MSG            "Reading configuration file\n"
#define WAITING_CONNECTIONS_MSG     "Waiting for connections...\n"
#define ERROR_CREATING_SOCKET_MSG   "ERROR: Arda could not create the server socket\n"
#define ERROR_BINDING_SOCKET_MSG    "ERROR: Arda could not bind the server socket\n"
#define ERROR_LISTENING_MSG         "ERROR: Arda could not make the listen\n"
#define ERROR_LISTENING_MSG         "ERROR: Arda could not make the listen\n"
#define ERROR_ACCEPTING_MSG	        "ERROR: Arda could not accept the connection request\n"

#define ARDA_OK	    0		
#define ARDA_KO	    -1
#define MIN_N_ARGS	2
#define BACKLOG     10

Arda arda;
int listenFD;

/*********************************************************************
 @Purpose: Free memory when SIGINT received.
 @Params: ----
 @Return: ----
*********************************************************************/
void sigHandler(){
    close(listenFD);
    // TODO: Potser fer un close del clientFD
    freeArda(&arda);
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
        arda->ip_address = readUntil(fd, END_OF_LINE);
	    // Arda port
        buffer = readUntil(fd, END_OF_LINE);
		arda->port = atoi(buffer);
		free(buffer);
		// directory
		arda->directory = readUntil(fd, END_OF_LINE);
		// no errors
		error = ARDA_OK;
		close(fd);
	}

	return (error);
}

int main(int argc, char** argv){
    int clientFD = -1;
    int read_ok = ARDA_KO;
    struct sockaddr_in server;

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
        free(&arda);
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

    // Running the server
    while(1) {
        printMsg(WAITING_CONNECTIONS_MSG);
        
        // Accept (Blocks the system until a connection request or petition arrives)
        if ((clientFD = accept(listenFD, (struct sockaddr*) NULL, NULL)) < 0){
            printMsg(COLOR_RED_TXT);
            printMsg(ERROR_ACCEPTING_MSG);
            printMsg(COLOR_DEFAULT_TXT);
        } else {
            
            // Connection request
            //TODO: Afegir a la llista de clients
            //TODO: Creacio de thread
            
            // User list petition
            //TODO: Enviar la llista de usuaris
            
            // Exit petition
            //TODO: Eliminar de la llista de clients
            //TODO: Tancar el thread

        }

    }

    return(0);
}