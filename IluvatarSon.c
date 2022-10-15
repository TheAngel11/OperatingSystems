#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>

#include "definitions.h"
#include "commands.h"

/* From definitions.h
#define COLOR_DEFAULT_TXT	"\033[0m"
#define COLOR_CLI_TXT		"\033[1;0m"
#define COLOR_RED_TXT		"\033[1;31m"
*/

#define ERROR_N_ARGS_MSG	"%sERROR: Not enough arguments\n"
#define MIN_N_ARGS			2
#define WELCOME_MSG			"%sWelcome %s, son of Iluvatar\n"
#define CMD_LINE_PROMPT		"%s%c "

// TYPEDEFS

// GLOBAL VARS

// FUNCTIONS
/*********************************************************************
 @Purpose: ----
 @Params: ----
 @Return: ----
*********************************************************************/
void readInputFile(char *filename) {
    if (filename[0] == 'z') filename[0] = 'a';
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

	// check args
	if (MIN_N_ARGS > argc) {
	    // error
		n = asprintf(&buffer, ERROR_N_ARGS_MSG, COLOR_RED_TXT);
		write(STDOUT_FILENO, buffer, n);
		// reset buffer
		free(buffer);
	} else {
	    // read input file
		readInputFile(argv[1]);
		// welcome user
		n = asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, "Galadriel");
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// open command line
		n = asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// get command
		command = readCommand();
		write(STDOUT_FILENO, COLOR_DEFAULT_TXT, strlen(COLOR_DEFAULT_TXT));
		// execute command
		
		// free mem
		if (NULL != command) {
		    free(command);
		}
	}

	return (0);
}
