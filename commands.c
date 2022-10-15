#include "commands.h"

/*********************************************************************
 @Purpose: ----
 @Params: ----
 @Return: ----
*********************************************************************/
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

/*********************************************************************
* @Purpose: Separates a command to be able to identify if it is a
*           custom command.
* @Params: in: user_input = string containing the entire command
* @Return: a string containing at most CMD_CUSTOM_MAX_WORDS words.
*********************************************************************/
char * getCommand(const char *user_input) {
	char *command = (char *) malloc (sizeof(char));
	int n_words = 0, i = 0;

	if (NULL != command) {
	    for (i = 0; i < (int) strlen(user_input); i++) {
	        command[i] = user_input[i];
			if (user_input[i] == ' ') {
		        n_words++;
			}

			if (n_words == CMD_CUSTOM_MAX_WORDS) {
			    break;
			}

			command = (char *) realloc (command, sizeof(char) * (i + 2));
		}

		command[i] = '\0';
	}

	return (command);
}

/*********************************************************************
* @Purpose: Executes the command entered by the user.
* @Params: in: user_input = entire command (with args) entered by user
* @Return: 0 if EXIT command entered, otherwise 1.
*********************************************************************/
int executeCommand(char *user_input) {
    char *command = NULL;
	char *cli_message = NULL;
	int n = 0;
	int pid = -1, status = 0;

	command = getCommand(user_input);

	if (NULL != command) {
	    pid = fork();

		switch (pid) {
		    case -1:
			    // error
				break;
			case 0:
				// identify command
				if (strcasecmp(command, UPDATE_USERS_CMD) == 0) {
				    //
					n = asprintf(&cli_message, "%s\n", UPDATE_USERS_CMD);
					write(STDOUT_FILENO, cli_message, n);
					free(cli_message);
				} else if (strcasecmp(command, LIST_USERS_CMD) == 0) {
				    //
					n = asprintf(&cli_message, "%s\n", LIST_USERS_CMD);
					write(STDOUT_FILENO, cli_message, n);
					free(cli_message);
				} else if (strcasecmp(command, SEND_MSG_CMD) == 0) {
				    //
					n = asprintf(&cli_message, "%s\n", SEND_MSG_CMD);
					write(STDOUT_FILENO, cli_message, n);
					free(cli_message);
				} else if (strcasecmp(command, SEND_FILE_CMD) == 0) {
				    //
					n = asprintf(&cli_message, "%s\n", SEND_FILE_CMD);
					write(STDOUT_FILENO, cli_message, n);
					free(cli_message);
				} else if (strcasecmp(command, EXIT_CMD) == 0) {
				    //
					n = asprintf(&cli_message, "%s\n", EXIT_CMD);
					write(STDOUT_FILENO, cli_message, n);
					free(cli_message);
					// notify Iluvatar son has to stop
					free(command);
					free(user_input);
					exit(IS_EXIT_CMD);
				} else {
				    // execute as Linux command
				    execl("/bin/sh", "sh", "-c", user_input, (char *) NULL);
				}

				// free mem
				free(command);
				free(user_input);
				exit(0);
				break;
			default:
			    wait(&status);
				if (0 != WEXITSTATUS(status)) {
				    if (IS_EXIT_CMD == WEXITSTATUS(status)) {
					    free(command);
						return (1);
						//*exit_iluvatar = 1;
					} else {
					    // Invalid Linux command
					    write(STDOUT_FILENO, UNKNOWN_CMD_MSG, strlen(UNKNOWN_CMD_MSG));
					}
				}

				// free mem
				free(command);
				break;
		}	
	}

	return (0);
}
