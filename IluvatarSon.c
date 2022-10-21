#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "commands.h"
#include "sharedFunctions.h"
/* From definitions.h
#define COLOR_DEFAULT_TXT	"\033[0m"
#define COLOR_CLI_TXT		"\033[1;0m"
#define COLOR_RED_TXT		"\033[1;31m"
*/

#define MIN_N_ARGS			     2
#define WELCOME_MSG		 	     "%sWelcome %s, son of Iluvatar\n"
#define CMD_LINE_PROMPT		     "%s%c "

// TYPEDEFS

// GLOBAL VARS

// FUNCTIONS

/**********************************************************************
* @Purpose: Frees all the dynamic memory.
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: ----
***********************************************************************/
void freeAllMem(IluvatarSon *iluvatarSon) {
    free(iluvatarSon->username);
    free(iluvatarSon->directory);
    free(iluvatarSon->ip_address);
    free(iluvatarSon->arda_ip_address);
}

/*********************************************************************
* @Purpose: Executes the IluvatarSon process.
* @Params: in: argc = number of arguments entered
*          in: argv = array of arguments entered
* @Return: Returns 0.
*********************************************************************/
int main(int argc, char* argv[]) {
    char *buffer = NULL;
	char *command = NULL;
	int n = 0;
    IluvatarSon iluvatarSon;
    //Arda arda;

	// check args
	if (MIN_N_ARGS > argc) {
	    // error
		n = asprintf(&buffer, ERROR_N_ARGS_MSG, COLOR_RED_TXT);
		write(STDOUT_FILENO, buffer, n);
		// reset buffer
		free(buffer);
	} else {
	    // read input file
		readInputFile(argv[1], &iluvatarSon);
		// welcome user
		n = asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, "Galadriel");
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// open command line
		n = asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// get command
		//command = readCommand();
		write(STDOUT_FILENO, COLOR_DEFAULT_TXT, strlen(COLOR_DEFAULT_TXT));
		// execute command

		// free mem
		if (NULL != command) {
		    free(command);
		}
	}

    //Debugging Purpose
    /*printf("USERNAME: %s\n", iluvatarSon.username);
    printf("DIRECTORY: %s\n", iluvatarSon.directory);
    printf("ARDA IP: %s\n", iluvatarSon.arda_ip_address);
    printf("ARDA PORT: %d\n", iluvatarSon.arda_port);
    printf("ILU IP: %s\n", iluvatarSon.ip_address);
    printf("ILU PORT: %d\n", iluvatarSon.port);*/

    freeAllMem(&iluvatarSon);

	return (0);
}