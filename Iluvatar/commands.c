/*********************************************************************
* @Purpose: Module that contains functions to manage the custom
*           commands for an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 17/11/2022
*********************************************************************/
#include "commands.h"

/*********************************************************************
* @Purpose: Separates a command into its arguments.
* @Params: in: input = string containing the entire command
*          in/out: args = array of strings to store the arguments of
*		                  the command (input)
* @Return: Returns the number of arguments of the command.
*********************************************************************/
int getCmdArgs(char *input, char ***args) {
	int n_args = 0, i = 0;
	char *tmp_arg = NULL;

	if (NULL != *args) {
	    free(*args);
		*args = NULL;
	}

	*args = (char **) malloc (sizeof(char *));
	if (NULL == *args) {
	    return (n_args);
	}

	while (i < (int) strlen(input)) {
		if (CMD_MSG_SEPARATOR == input[i]) {
		    i++;
			tmp_arg = SHAREDFUNCTIONS_splitString(input, CMD_MSG_SEPARATOR, &i);
			(*args)[n_args] = (char *) malloc (sizeof(char) * (strlen(tmp_arg) + 3));
			(*args)[n_args][0] = CMD_MSG_SEPARATOR;
			(*args)[n_args][1] = '\0';
			strcat((*args)[n_args], tmp_arg);
			free(tmp_arg);
			tmp_arg = NULL;
			// add last " if it was in original input
			if ('"' == input[i - 1]) {
			    asprintf(&tmp_arg, "%c", CMD_MSG_SEPARATOR);
				strcat((*args)[n_args], tmp_arg);
				free(tmp_arg);
				tmp_arg = NULL;
			}
			
			n_args++;
		} else {
		    (*args)[n_args] = SHAREDFUNCTIONS_splitString(input, ' ', &i);
			n_args++;
		}
		
		// skip ' '
		while ((input[i] == ' ') && (i < (int) strlen(input))) {
		    i++;
		}
		
		// next argument
		if (i < (int) strlen(input)) {
		    (*args) = (char **) realloc (*args, sizeof(char *) * (n_args + 1));
		}
	}

	(*args) = (char **) realloc (*args, sizeof(char *) * (n_args + 1));
	(*args)[n_args] = NULL;

	return (n_args);
}

/*********************************************************************
* @Purpose: Identifies the custom command or if it is not a custom one.
* @Params: in: args = array of strings containing the arguments of the
*                     command
*		   in: n_args = number of arguments
* @Return: Returns the ID of the custom command or IS_NOT_CUSTOM_CMD.
*********************************************************************/
int identifyCommand(char **args, int n_args) {
    char *concat_args = NULL;

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
		} else if (0 == strcasecmp(concat_args, SEND_MSG_CMD)) {
		    if (n_args == SEND_MSG_N_ARGS) {
				// check "s
				if (('"' != args[n_args - 1][0]) || ('"' != args[n_args - 1][strlen(args[n_args - 1]) - 1])) {
				    printMsg(COLOR_RED_TXT);
					printMsg(ERROR_SEND_MSG_FORMAT);
					printMsg(COLOR_DEFAULT_TXT);
					free(concat_args);
					return (ERROR_CMD_ARGS);
				}
			} else {
			    // print error msg
				printMsg(COLOR_RED_TXT);
				
				if (n_args < SEND_MSG_N_ARGS) {
				    printMsg(ERROR_SEND_MSG_LESS_ARGS);
				} else {
				    printMsg(ERROR_SEND_MSG_MORE_ARGS);
				}

				printMsg(COLOR_DEFAULT_TXT);
				free(concat_args);
				return (ERROR_CMD_ARGS);
			}

		    free(concat_args);
		    return (IS_SEND_MSG_CMD);
		} else if (0 == strcasecmp(concat_args, SEND_FILE_CMD)) {
		    if (n_args != SEND_FILE_N_ARGS) {
			    // print error msg
				printMsg(COLOR_RED_TXT);

				if (n_args < SEND_FILE_N_ARGS) {
				    printMsg(ERROR_SEND_FILE_LESS_ARGS);
				} else {
				    printMsg(ERROR_SEND_FILE_MORE_ARGS);
				}

				printMsg(COLOR_DEFAULT_TXT);
		        free(concat_args);
				return (ERROR_CMD_ARGS);
			}
		    
			free(concat_args);
		    return (IS_SEND_FILE_CMD);
		} else {
		    free(concat_args);
		    return (IS_NOT_CUSTOM_CMD);
		}
	}
}

/*********************************************************************
* @Purpose: Executes a custom command given its ID. Currently only
*           prints the selected command.
* @Params: in: id = ID of the custom command to execute
* @Return: ----
*********************************************************************/
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

/*********************************************************************
* @Purpose: Frees allocated memory for the command.
* @Params: in/out: args = array of strings containing the command's
*                         arguments
*          in/out: n_args = number of arguments
* @Return: ----
*********************************************************************/
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
int COMMANDS_executeCommand(char *user_input, IluvatarSon *iluvatar) {
    char **command = NULL;
	int n_args = 0, cmd_id = 0;
	int pid = -1, status = 0;

	n_args = getCmdArgs(user_input, &command);
	// identify command
	cmd_id = identifyCommand(command, n_args);
	
	if ((ERROR_CMD_ARGS != cmd_id) && (IS_NOT_CUSTOM_CMD != cmd_id)) {
	    // execute custom command
		executeCustomCommand(cmd_id);
		
		if (cmd_id == IS_EXIT_CMD) {
			freeMemCmd(&command, &n_args);
			SHAREDFUNCTIONS_freeIluvatarSon(iluvatar);
			return (1);
		}
	} else if (IS_NOT_CUSTOM_CMD == cmd_id) {
	    pid = fork();

		switch (pid) {
		    case -1:
			    // error
				printMsg(COLOR_RED_TXT);
				printMsg(ERROR_CREATING_CHILD);
				printMsg(COLOR_DEFAULT_TXT);
				break;
			case 0:
				// execute as Linux command
				status = execvp(command[0], command);

				// free mem
				freeMemCmd(&command, &n_args);
				SHAREDFUNCTIONS_freeIluvatarSon(iluvatar);
				free(user_input);
				exit(status);
				break;
			default:
			    wait(&status);
				if (0 != WEXITSTATUS(status)) {
					// Invalid Linux command
					printMsg(UNKNOWN_CMD_MSG);
				}
				break;
		}	
	}
	
	// free mem
	freeMemCmd(&command, &n_args);

	return (0);
}
