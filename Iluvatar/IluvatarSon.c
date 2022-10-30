/*********************************************************************
* @Purpose: Executes an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 23/10/2022
*********************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "commands.h"
#include "../sharedFunctions.h"

#define MIN_N_ARGS 		2
#define WELCOME_MSG 	"\n%sWelcome %s, son of Iluvatar\n"
#define CMD_LINE_PROMPT	"%s%c "
#define ILUVATARSON_OK	0		
#define ILUVATARSON_KO	-1

IluvatarSon iluvatarSon;
char *iluvatar_command = NULL;

/*********************************************************************
 @Purpose: Free memory when SIGINT received.
 @Params: ----
 @Return: ----
*********************************************************************/
void sigintHandler() {
    freeIluvatarSon(&iluvatarSon);
	if (NULL != iluvatar_command) {
	    free(iluvatar_command);
	}
	
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
	    iluvatar->username = readUntil(fd, END_OF_LINE);
	    // check name
		iluvatar->username = removeChar(iluvatar->username, GPC_DATA_SEPARATOR);
		// directory
		iluvatar->directory = readUntil(fd, END_OF_LINE);
		// Arda IP
		iluvatar->arda_ip_address = readUntil(fd, END_OF_LINE);
	    // Arda port
		buffer = readUntil(fd, END_OF_LINE);
		iluvatar->arda_port = atoi(buffer);
		free(buffer);
		// Iluvatar IP
	    iluvatar->ip_address = readUntil(fd, END_OF_LINE);
	    // Iluvatar port
		buffer = readUntil(fd, END_OF_LINE);
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
	
	iluvatarSon = newIluvatarSon();
	// Configure SIGINT
	signal(SIGINT, sigintHandler);
	
	// check args
	if (MIN_N_ARGS > argc) {
	    // error
		asprintf(&buffer, ERROR_N_ARGS_MSG, COLOR_RED_TXT);
		printMsg(buffer);
		free(buffer);
	} else {
	    // read input file
		read_ok = readIluvatarSon(argv[1], &iluvatarSon);
		
		if (ILUVATARSON_KO == read_ok) {
		    // print error
			printMsg(COLOR_RED_TXT);
			asprintf(&buffer, ERROR_OPENING_FILE, argv[1]);
			printMsg(buffer);
			free(buffer);
			freeIluvatarSon(&iluvatarSon);
			return (0);
		}

		// welcome user
		asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, iluvatarSon.username);
		printMsg(buffer);
		free(buffer);
		
		// get commands
		while (!exit_program) {
			// open command line
		    asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		    printMsg(buffer);
		    free(buffer);
		    // get command
			command = readUntil(STDIN_FILENO, CMD_END_BYTE);
		    printMsg(COLOR_DEFAULT_TXT);
		    // execute command
		    exit_program = executeCommand(command, &iluvatarSon);

		    // free mem
		    if (NULL != command) {
		        free(command);
		    }
		}
	}

    freeIluvatarSon(&iluvatarSon);

	return (0);
}
