#include "commands.h"

char* readCommand() {
	char *command = NULL;
	int n = 0, i = 0;

	command = (char *) malloc (sizeof(char));
	if (NULL == command) {
	    // error
		return (NULL);
	}

	// wait for input
	do {
	    n = read(STDIN_FILENO, &command[i], 1);
	} while (n <= 0);

	// read line
    do {
	    i++;
		command = (char *) realloc (command, sizeof(char) * (i + 1));
		n = read(STDIN_FILENO, &command[i], 1);
	} while (CMD_END_BYTE != command[i]);

	command[i] = '\0';

	return (command);
}
