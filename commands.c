#include "commands.h"

// MOVE to sharedFuncts
#define printMsg(x) write(0, x, strlen(x))

// MOVE to sharedFuncts
char * readUntil(int fd, char delimiter) {
    char *buffer = (char *) malloc (sizeof(char));
	char byte = delimiter + 1;
	int i = 0;

	while (byte != delimiter) {
	    read(fd, &byte, 1);
		if (byte != delimiter) {
		    buffer[i] = byte;
			i++;
			buffer = (char *) realloc (buffer, sizeof(char) * (i + 1));
		}
	}

	buffer[i] = '\0';

	return (buffer);
}

// MOVE to sharedFuncts
char * splitString(char *string, char delimiter, int *pos) {
     char *output = (char *) malloc (sizeof(char));
	 int i = 0;

	 if (NULL == output) {
	     return NULL;
	 }

	 while (*pos < (int) strlen(string)) {
	     if (string[*pos] == delimiter) {
		     output[i] = '\0';
			 // skip delimiter
			 (*pos)++;
			 return (output);
		 }

		 output[i] = string[*pos];
		 output = (char *) realloc (output, sizeof(char) * (i + 2));		 
		 (*pos)++;
		 i++;
	 }

	 output[i] = '\0';

	 return (output);
}

/*********************************************************************
 @Purpose: ----
 @Params: ----
 @Return: ----
*********************************************************************/
char* readCommand() {
	char *command = NULL;
	
	command = readUntil(STDIN_FILENO, CMD_END_BYTE);

	return (command);
}

// separate command in args
int getCmdArgs(char *input, char ***args) {
	int n_args = 0, i = 0;

	if (NULL != *args) {
	    free(*args);
		*args = NULL;
	}

	*args = (char **) malloc (sizeof(char *));
	if (NULL == *args) {
	    return (n_args);
	}

	while (i < (int) strlen(input)) {
	    (*args)[n_args] = splitString(input, ' ', &i);
		n_args++;
		if (i < (int) strlen(input)) {
		    (*args) = (char **) realloc (*args, sizeof(char *) * (n_args + 1));
		}
	}

	return (n_args);
}

int identifyCommand(char **args, int n_args) {
    char * concat_args = NULL;

	if (EXIT_N_ARGS == n_args) {
	    if (0 == strcasecmp(args[0], EXIT_CMD)) {
		    return (IS_EXIT_CMD);
		} else {
		    return (IS_NOT_CUSTOM_CMD);
		}
	} else {
	    concat_args = (char *) malloc (sizeof(char) * (strlen(args[0]) + strlen(args[1]) + 2));
		concat_args[0] = '\0';
		strcat(concat_args, args[0]);
		strcat(concat_args, " ");
		strcat(concat_args, args[1]);
	   
		if ((0 == strcasecmp(concat_args, UPDATE_USERS_CMD)) && (n_args == UPDATE_USERS_N_ARGS)) {
		    free(concat_args);
			return (IS_UPDATE_USERS_CMD);
		} else if ((0 == strcasecmp(concat_args, LIST_USERS_CMD)) && (n_args == LIST_USERS_N_ARGS)) {
		    free(concat_args);
		    return (IS_LIST_USERS_CMD);
		} else if ((0 == strcasecmp(concat_args, SEND_MSG_CMD)) && (n_args == SEND_MSG_N_ARGS)) {
		    free(concat_args);
		    return (IS_SEND_MSG_CMD);
		} else if ((0 == strcasecmp(concat_args, SEND_FILE_CMD)) && (n_args == SEND_FILE_N_ARGS)) {
		    free(concat_args);
		    return (IS_SEND_FILE_CMD);
		} else {
		    free(concat_args);
		    return (IS_NOT_CUSTOM_CMD);
		}
	}
}

void executeCustomCommand(int id) {
    char *buffer = NULL;

	switch (id) {
	    case IS_UPDATE_USERS_CMD:
		    asprintf(&buffer, "%s\n", UPDATE_USERS_CMD);
			printMsg(buffer);
			free(buffer);
			break;
		case IS_LIST_USERS_CMD:
		    asprintf(&buffer, "%s\n", LIST_USERS_CMD);
			printMsg(buffer);
			free(buffer);
			break;
		case IS_SEND_MSG_CMD:
		    asprintf(&buffer, "%s\n", SEND_MSG_CMD);
			printMsg(buffer);
			free(buffer);
			break;
		case IS_SEND_FILE_CMD:
		    asprintf(&buffer, "%s\n", SEND_FILE_CMD);
			printMsg(buffer);
			free(buffer);
			break;
		default:
		    // exit command
		    asprintf(&buffer, "%s\n", EXIT_CMD);
			printMsg(buffer);
			free(buffer);
			break;
	}
}

void freeMemCmd(char ***args, int *n_args) {
    int i = 0;

	for (i = 0; i < *n_args; i++) {
	    if (NULL != (*args)[i]) {
		    free((*args)[i]);
			(*args)[i] = NULL;
		}
	}

	free(*args);
	*args = NULL;
	*n_args = 0;
}

/*********************************************************************
* @Purpose: Executes the command entered by the user.
* @Params: in: user_input = entire command (with args) entered by user
* @Return: 0 if EXIT command entered, otherwise 1.
*********************************************************************/
int executeCommand(char *user_input) {
    char **command = NULL;
	int n_args = 0, cmd_id = 0;
	int pid = -1, status = 0;

	n_args = getCmdArgs(user_input, &command);

	if (0 < n_args) {
	    pid = fork();

		switch (pid) {
		    case -1:
			    // error
				break;
			case 0:
				// identify command
				cmd_id = identifyCommand(command, n_args);

				if (IS_NOT_CUSTOM_CMD == cmd_id) {
				    // execute as Linux command
				    execl("/bin/sh", "sh", "-c", user_input, (char *) NULL);
				} else {
				    // execute custom command
					executeCustomCommand(cmd_id);
					if (cmd_id == IS_EXIT_CMD) {
					    freeMemCmd(&command, &n_args);
						free(user_input);
						exit(IS_EXIT_CMD);
					}
				}

				// free mem
				freeMemCmd(&command, &n_args);
				free(user_input);
				exit(0);
				break;
			default:
			    wait(&status);
				if (0 != WEXITSTATUS(status)) {
				    if (IS_EXIT_CMD == WEXITSTATUS(status)) {
						freeMemCmd(&command, &n_args);
						return (1);
					} else {
					    // Invalid Linux command
					    write(STDOUT_FILENO, UNKNOWN_CMD_MSG, strlen(UNKNOWN_CMD_MSG));
					}
				}

				// free mem
				freeMemCmd(&command, &n_args);
				break;
		}	
	}

	return (0);
}
