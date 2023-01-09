/*********************************************************************
* @Purpose: Runs the server for the sons of Iluvatar.
* @Authors: Angel Garcia Gascon
*           Claudia Lajara Silvosa
* @Date: 30/10/2022
* @Last change: 09/01/2023
*********************************************************************/
#define _GNU_SOURCE 1 
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "../sharedFunctions.h"
#include "../bidirectionallist.h"
#include "../server.h"

#define DISCONNECT_ARDA_MSG		"\nDisconnecting Arda from all Iluvatar's children\n"
#define N_MSG_ARDA_MSG			"%d Messages have been sent through the network\n"
#define CLOSING_ARDA_MSG        "Closing server\n"
#define WELCOME_MSG             "\nARDA SERVER\n"
#define READING_FILE_MSG        "Reading configuration file\n"
#define WAITING_CONNECTIONS_MSG "Waiting for connections...\n\n"
#define ERROR_N_ARGS_MSG        "ERROR: not enough arguments\n"
#define ERROR_OPENING_SOCKET_MSG "ERROR: the socket could not be opened\n"

#define ARDA_OK	    			0
#define ARDA_KO	    			-1
#define MIN_N_ARGS				2
#define N_MSG_FILE_PATH			"./totalMessages.txt"
#define N_MSG_EOF				'#'

Arda arda;
Server server;

/*********************************************************************
* @Purpose: Write total messages.
* @Params: ----
* @Return: ----
*********************************************************************/
void updateTotalMsg() {
    int fd = open(N_MSG_FILE_PATH, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	char *buffer = NULL;

	if (fd != -1) {
	    asprintf(&buffer, "%d%c\n", server.n_msg, N_MSG_EOF);
		write(fd, buffer, strlen(buffer));
		free(buffer);
		buffer = NULL;
		close(fd);
	}
}

/*********************************************************************
* @Purpose: Free memory and close any open file descriptors before
*           terminating program.
* @Params: ----
* @Return: ----
*********************************************************************/
void sigIntHandler() {   
    char *buf = NULL;

	updateTotalMsg();
	asprintf(&buf, N_MSG_ARDA_MSG, server.n_msg);
	SERVER_close(&server);
	SHAREDFUNCTIONS_freeArda(&arda); 
    
	if (server.thread != NULL) {
        free(server.thread); 
	    server.thread = NULL;
    }
	
	printMsg(DISCONNECT_ARDA_MSG);
	printMsg(buf);
	free(buf);
	buf = NULL;
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
* @Purpose: Creates an Arda server that accepts connections from
*           IluvatarSon processes and runs it.
* @Params: in: argc = number of arguments received.
*          in: argv = array of arguments.
* @Return: Returns 0 if no errors occured, otherwise -1.
*********************************************************************/
int main(int argc, char* argv[]) {
	char *buffer = NULL;
    int fd_n_msg = -1, n_msg = 0;
	int read_ok = ARDA_KO;
    
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
        printMsg(COLOR_DEFAULT_TXT);
        free(buffer);
        SHAREDFUNCTIONS_freeArda(&arda);
        return (-1);
    }
    
	// get total messages
	if (-1 != (fd_n_msg = open(N_MSG_FILE_PATH, O_RDONLY))) {
	    buffer = SHAREDFUNCTIONS_readUntil(fd_n_msg, N_MSG_EOF);
		n_msg = atoi(buffer);
		free(buffer);
		buffer = NULL;
		
		if (-1 != fd_n_msg) {
		    close(fd_n_msg);
		}
	}
	
	// Open passive socket
	server = SERVER_init(arda.ip_address, arda.port, n_msg);

	if ((FD_NOT_FOUND == server.listen_fd) || (LIST_NO_ERROR != server.clients.error)) {
	    SHAREDFUNCTIONS_freeArda(&arda);
        SERVER_close(&server);
        printMsg(COLOR_RED_TXT);
        printMsg(ERROR_OPENING_SOCKET_MSG);
        printMsg(COLOR_DEFAULT_TXT);
		return (-1);
	}

    // Running the server
    printMsg(WAITING_CONNECTIONS_MSG);
	SERVER_runArda(&arda, &server);

    return (0);
}
