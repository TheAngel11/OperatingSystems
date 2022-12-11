/*********************************************************************
* @Purpose: Executes an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 11/12/2022
*********************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../bidirectionallist.h"
#include "commands.h"
#include "../sharedFunctions.h"
#include "../gpc.h"

#define MIN_N_ARGS 					2
#define WELCOME_MSG 				"\n%sWelcome %s, son of Iluvatar\n"
#define CMD_LINE_PROMPT				"%s%c "
#define ARDA_CONNECTION_DENIED_MSG	"Connection denied by server\n"
#define ILUVATARSON_OK				0
#define ILUVATARSON_KO				-1

IluvatarSon iluvatarSon;
char *iluvatar_command = NULL;
int fd_arda = -1;
BidirectionalList users_list;

/*********************************************************************
* @Purpose: Free memory when SIGINT received.
* @Params: ----
* @Return: ----
*********************************************************************/
void sigintHandler() {
    SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
	if (NULL != iluvatar_command) {
	    free(iluvatar_command);
	}

	close(fd_arda);
	printMsg(COLOR_DEFAULT_TXT);
	
	BIDIRECTIONALLIST_destroy(&users_list);

	signal(SIGINT, SIG_DFL);
	raise(SIGINT);
}

/*********************************************************************
* @Purpose: Creates a new empty IluvatarSon.
* @Params: ----
* @Return: Returns an initialized IluvatarSon.
*********************************************************************/
IluvatarSon newIluvatarSon() {
    IluvatarSon iluvatar;

	iluvatar.username = NULL;
	iluvatar.directory = NULL;
	iluvatar.ip_address = NULL;
	iluvatar.arda_ip_address = NULL;

	return (iluvatar);
}

/*********************************************************************
* @Purpose: Reads an IluvatarSon from a given file.
* @Params: in: filename = string with the name of the file
*          in/out: iluvatar = pointer to IluvatarSon referencing
*		                      the IluvatarSon to fill with data
* @Return: Returns ILUVATARSON_OK if no errors when reading the file, 
*          otherwise ILUVATARSON_KO
*********************************************************************/
int readIluvatarSon(char *filename, IluvatarSon *iluvatar) {
    char *buffer = NULL;
	int error = ILUVATARSON_KO;
	int fd = open(filename, O_RDONLY);

	if (fd > 0) {
	    iluvatar->username = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // check name
		iluvatar->username = SHAREDFUNCTIONS_removeChar(iluvatar->username, GPC_DATA_SEPARATOR);
		// directory
		iluvatar->directory = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		// Arda IP
		iluvatar->arda_ip_address = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // Arda port
		buffer = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		iluvatar->arda_port = atoi(buffer);
		free(buffer);
		// Iluvatar IP
	    iluvatar->ip_address = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // Iluvatar port
		buffer = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		iluvatar->port = atoi(buffer);
		free(buffer);
		// no errors
		error = ILUVATARSON_OK;
		close(fd);
	}

	return (error);
}

/*********************************************************************
* @Purpose: Executes the IluvatarSon process.
* @Params: in: argc = number of arguments entered
*          in: argv = array of arguments entered
* @Return: Returns 0.
*********************************************************************/
int main(int argc, char* argv[]) {
    char *buffer = NULL;
	char *command = iluvatar_command;
	int exit_program = 0, read_ok = ILUVATARSON_KO;
	struct sockaddr_in server;
	char *header = NULL;
	char type = 0x07;
	
	iluvatarSon = newIluvatarSon();
	users_list = BIDIRECTIONALLIST_create();
	// Configure SIGINT
	signal(SIGINT, sigintHandler);
	
	// check args
	if (MIN_N_ARGS != argc) {
	    // error
		printMsg(COLOR_RED_TXT);
		
		if (MIN_N_ARGS > argc) {
		    printMsg(ERROR_N_LESS_ARGS_MSG);
		} else {
		    printMsg(ERROR_N_MORE_ARGS_MSG);
		}

		printMsg(COLOR_DEFAULT_TXT);
	} else {
		// read input file
		read_ok = readIluvatarSon(argv[1], &iluvatarSon);
		
		if (ILUVATARSON_KO == read_ok) {
		    // print error
			printMsg(COLOR_RED_TXT);
			asprintf(&buffer, ERROR_OPENING_FILE, argv[1]);
			printMsg(buffer);
			free(buffer);
			printMsg(COLOR_DEFAULT_TXT);
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			return (0);
		}
		
/* START TEST */
        // config socket
		if ((fd_arda = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	        printMsg(COLOR_RED_TXT);
			printMsg(ERROR_CREATING_SOCKET_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			return (1);
		}

		bzero(&server, sizeof(server));
		server.sin_family = AF_INET;
		server.sin_port = htons(iluvatarSon.arda_port);
		if (inet_pton(AF_INET, iluvatarSon.arda_ip_address, &server.sin_addr) < 0) {
	        printMsg(COLOR_RED_TXT);
			printMsg(ERROR_IP_CONFIGURATION_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			// finish execution
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			close(fd_arda);
			return (1);
		}

		// connect to server
		if (connect(fd_arda, (struct sockaddr*) &server, sizeof(server)) < 0) {
	        printMsg(COLOR_RED_TXT);
			printMsg(ERROR_SERVER_CONNECTION_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			// finish execution
			BIDIRECTIONALLIST_destroy(&users_list);
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			close(fd_arda);
			return (1);
		}
/* END TEST */

		// notify connection to Arda
		asprintf(&buffer, "%s%c%s%c%d%c%d", iluvatarSon.username, GPC_DATA_SEPARATOR, iluvatarSon.ip_address, GPC_DATA_SEPARATOR, iluvatarSon.port, GPC_DATA_SEPARATOR, getpid());
		GPC_writeFrame(fd_arda, 0x01, GPC_CONNECT_SON_HEADER, buffer);
		free(buffer);
		buffer = NULL;
		// wait for answer
		GPC_readFrame(fd_arda, &type, &header, &buffer);

		// check connection
		if (0 == strcmp(header, GPC_HEADER_CONKO)) {
		    printMsg(COLOR_RED_TXT);
			printMsg(ARDA_CONNECTION_DENIED_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			// free mem
			free(buffer);
			buffer = NULL;
			free(header);
			header = NULL;
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			BIDIRECTIONALLIST_destroy(&users_list);
			close(fd_arda);

			return (1);
		}
		
		free(header);
		header = NULL;
		// update list of users
		GPC_updateUsersList(&users_list, buffer);
		free(buffer);
		buffer = NULL;
		// welcome user
		asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, iluvatarSon.username);
		printMsg(buffer);
		free(buffer);
		buffer = NULL;
		
		// get commands
		while (!exit_program) {
			// open command line
		    asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		    printMsg(buffer);
		    free(buffer);
			buffer = NULL;
		    // get command
			command = SHAREDFUNCTIONS_readUntil(STDIN_FILENO, CMD_END_BYTE);
		    printMsg(COLOR_DEFAULT_TXT);
		    // execute command
		    exit_program = COMMANDS_executeCommand(command, &iluvatarSon, fd_arda, &users_list);

		    // free mem
		    if (NULL != command) {
		        free(command);
				command = NULL;
		    }
		}
	}

    SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
	BIDIRECTIONALLIST_destroy(&users_list);
	close(fd_arda);

	return (0);
}
