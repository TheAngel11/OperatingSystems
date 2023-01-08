/*********************************************************************
* @Purpose: Module that contains functions to manage the custom
*           commands for an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 08/01/2023
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
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns the ID of the custom command or IS_NOT_CUSTOM_CMD.
*********************************************************************/
int identifyCommand(char **args, int n_args, pthread_mutex_t *mutex) {
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
				    pthread_mutex_lock(mutex);
					printMsg(COLOR_RED_TXT);
					printMsg(ERROR_SEND_MSG_FORMAT);
					printMsg(COLOR_DEFAULT_TXT);
					pthread_mutex_unlock(mutex);
					free(concat_args);
					return (ERROR_CMD_ARGS);
				}
			} else {
			    // print error msg
				pthread_mutex_lock(mutex);
				printMsg(COLOR_RED_TXT);

				if (n_args < SEND_MSG_N_ARGS) {
				    printMsg(ERROR_SEND_MSG_LESS_ARGS);
				} else {
				    printMsg(ERROR_SEND_MSG_MORE_ARGS);
				}

				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(mutex);
				free(concat_args);
				return (ERROR_CMD_ARGS);
			}

		    free(concat_args);
		    return (IS_SEND_MSG_CMD);
		} else if (0 == strcasecmp(concat_args, SEND_FILE_CMD)) {
		    if (n_args != SEND_FILE_N_ARGS) {
			    // print error msg
				pthread_mutex_lock(mutex);
				printMsg(COLOR_RED_TXT);

				if (n_args < SEND_FILE_N_ARGS) {
				    printMsg(ERROR_SEND_FILE_LESS_ARGS);
				} else {
				    printMsg(ERROR_SEND_FILE_MORE_ARGS);
				}

				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(mutex);
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
* @Purpose: Get the hostname of a given IP.
* @Params: in: ip_address = the ip address of the host.
* @Return: Returns a string containing the hostname of the given IP.
*********************************************************************/
char * getHostnameByIP(char *ip_address) {
	char *hostname = NULL;
	struct in_addr ip;
	struct hostent *host;

	inet_aton(ip_address, &ip);
	host = gethostbyaddr(&ip, sizeof(ip), AF_INET);
	// copy hostname
	hostname = (char *) malloc(strlen(host->h_name) + 1);
	strcpy(hostname, host->h_name);
	return (hostname);
}

/*********************************************************************
* @Purpose: Prints a list of the connected users.
* @Params: in: users = list of connected users
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: ----
*********************************************************************/
void printUsersList(BidirectionalList users, pthread_mutex_t *mutex) {
	char *buffer = NULL;
	char *hostname = NULL;
	Element user;
	int n = 0, i = 1;

	// show number of users
	n = BIDIRECTIONALLIST_getNumberOfElements(users);
	asprintf(&buffer, LIST_USERS_N_USERS_MSG, n);
	pthread_mutex_lock(mutex);
	printMsg(buffer);
	pthread_mutex_unlock(mutex);
	free(buffer);
	buffer = NULL;

	if (n > 0) {
		// print list of users
		if(!BIDIRECTIONALLIST_isEmpty(users)) {
			BIDIRECTIONALLIST_goToHead(&users);

			while (BIDIRECTIONALLIST_isValid(users)) {
				// get user
				user = BIDIRECTIONALLIST_get(&users);
				hostname = getHostnameByIP(user.ip_network);
				asprintf(&buffer, "%d. %s %s %d %s %d\n", i, user.username, user.ip_network, user.port, hostname, user.pid);
				// show user
				pthread_mutex_lock(mutex);
				printMsg(buffer);
				pthread_mutex_unlock(mutex);
				free(buffer);
				buffer = NULL;
				free(hostname);
				hostname = NULL;
				// next user
				BIDIRECTIONALLIST_next(&users);
				i++;
				// free element
				free(user.username);
				user.username = NULL;
				free(user.ip_network);
				user.ip_network = NULL;
			}
		}

		pthread_mutex_lock(mutex);
		printMsg("\n");
		pthread_mutex_unlock(mutex);
	}
}

/*********************************************************************
* @Purpose: Searches a user in a list of users.
* @Params: in: users = list of users
* 		   in: username = username of the user to find
* @Return: Returns USER_FOUND if user is found, otherwise
*          USER_NOT_FOUND.
*********************************************************************/
char searchUserInList(BidirectionalList users, char *username, Element *user) {
	if (!BIDIRECTIONALLIST_isEmpty(users)) {
	    BIDIRECTIONALLIST_goToHead(&users);

		while (BIDIRECTIONALLIST_isValid(users)) {
	        *user = BIDIRECTIONALLIST_get(&users);

			// check user
			if (0 == strcasecmp(user->username, username)) {
				return (USER_FOUND);
			}

			// next user
			free(user->username);
			user->username = NULL;
			free(user->ip_network);
			user->ip_network = NULL;
			BIDIRECTIONALLIST_next(&users);
		}
	}
	
	// free memory
	if (NULL != user->username) {
	    free(user->username);
		user->username = NULL;
	}

	if (NULL != user->ip_network) {
	    free(user->ip_network);
		user->ip_network = NULL;
	}

	return (USER_NOT_FOUND);
}

/*********************************************************************
* @Purpose: Sends a message to a user using sockets.
* @Params: in: iluvatar = iluvatar son.
* 		   in: e = element with the user to send the message.
* 		   in: msg = message to send.
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if message was sent successfully, otherwise 1.
*********************************************************************/
char socketsSendMsg(char *sender, Element e, char *msg, pthread_mutex_t *mutex) {
	Client client;
	char *data = NULL;

	// check message is not empty
	if (strlen(msg) > 2) {
	    // remove message delimiters (")
		msg = msg + 1;
		msg[strlen(msg) - 1] = '\0';
		// prepare data to send
		data = (char *) malloc (sizeof(char) * (strlen(sender) + strlen(msg) + 2));
		sprintf(data, "%s%c%s", sender, GPC_DATA_SEPARATOR, msg);
	}

	if (data != NULL) {
		// Open socket
		client = CLIENT_init(e.ip_network, e.port);
		
		// Check client
		if (FD_NOT_FOUND == client.server_fd) {
			raise(SIGINT);
		}
		
		// Send the message
		return (CLIENT_sendMsg(&client, &data, mutex));
	} else {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_MSG_INVALID_MSG_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
	}

	return (1);
}

/*********************************************************************
* @Purpose: Sends a file to a user using sockets.
* @Params: in: iluvatar = iluvatar son.
* 			in: e = element with the user to send the file.
* 			in: filename = filename to send.
* @Return: Returns 0 if the file was sent successfully, otherwise 1.
*********************************************************************/
char socketsSendFile(char *username, Element e, char *filename, char *directory, pthread_mutex_t *mutex) {
	char *filename_path = NULL;
	int file_size = 0;
	char *md5sum = NULL;
	Client client;
	char *data = NULL;
	int fd_file = FD_NOT_FOUND;

	asprintf(&filename_path, ".%s/%s", directory, filename);
	fd_file = open(filename_path, O_RDONLY);

	// check file FD
	if (fd_file == FD_NOT_FOUND) {
	    asprintf(&data, SEND_FILE_OPEN_FILE_ERROR, filename);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(data);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(data);
		data = NULL;
		free(filename_path);
		filename_path = NULL;
		return (1);
	}

	// get file size
	file_size = (int) lseek(fd_file, 0, SEEK_END);
	lseek(fd_file, 0, SEEK_SET);

	if (file_size == 0) {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_FILE_EMPTY_FILE_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		return (1);
	}

	// get MD5SUM
	md5sum = SHAREDFUNCTIONS_getMD5Sum(filename_path);
	// Prepare the data to send
	asprintf(&data, "%s%c%s%c%d%c%s", username, GPC_DATA_SEPARATOR, filename, GPC_DATA_SEPARATOR, file_size, GPC_DATA_SEPARATOR, md5sum);
	free(md5sum);
	md5sum = NULL;
	free(filename_path);
	filename_path = NULL;

	if (data != NULL) {
		// Open socket
		client = CLIENT_init(e.ip_network, e.port);
		// Check client
		if (FD_NOT_FOUND == client.server_fd) {
			close(fd_file);
			free(data);
			data = NULL;
			raise(SIGINT);
		}

		// Send file frames
		return (CLIENT_sendFile(&client, &data, &fd_file, file_size, mutex));
	} else {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_FILE_INVALID_FILE_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
	}

	return (0);
}

/*********************************************************************
* @Purpose: Checks whether user is in the same machine as origin user.
* @Params: in: origin_ip = string containing the IP of the origin user
*          in: destination_ip = string containing the IP of the
*              destination user
* @Return: Returns IS_REMOTE_USER if users are in different machines,
*          otherwise IS_LOCAL_USER.
*********************************************************************/
char checkUserIP(char *origin_ip, char *destination_ip) {
    char *origin_hostname = NULL;
	char *dest_hostname = NULL;

	origin_hostname = getHostnameByIP(origin_ip);
	dest_hostname = getHostnameByIP(destination_ip);
	
	if (0 == strcmp(origin_hostname, dest_hostname)) {
	    free(origin_hostname);
		origin_hostname = NULL;
		free(dest_hostname);
		dest_hostname = NULL;
		return (IS_LOCAL_USER);
	}

	free(origin_hostname);
	origin_hostname = NULL;
	free(dest_hostname);
	dest_hostname = NULL;
	return (IS_REMOTE_USER);
}

/*********************************************************************
* @Purpose: Send a message to another IluvatarSon.
* @Params: in: clients = list of users of the sender
*          in: dest_username = string containing the username of the
*		       destination IluvatarSon
*		   in: message = string containing the message to send
*		   in: origin_username = string containing the username of the
*		       sender
*		   in: origin_ip = string with the IP address of the sender
*		   in/out: mutex = screen mutex to prevent writing to screen
*		           simultaneously
* @Return: Returns SEND_MSG_OK if no errors occurred, otherwise
*          SEND_MSG_KO.
*********************************************************************/
char sendMsgCommand(BidirectionalList clients, char *dest_username, char *message, char *origin_username, char *origin_ip, pthread_mutex_t *mutex) {
	Element e;
	char *buffer = NULL;

	// search destination user
	if (USER_FOUND == searchUserInList(clients, dest_username, &e)) {
		// check if remote user
		if (IS_REMOTE_USER == checkUserIP(origin_ip, e.ip_network)) {
		    // send message
			if (0 != socketsSendMsg(origin_username, e, message, mutex)) {
				// free memory
				free(e.username);
				e.username = NULL;
				free(e.ip_network);
				e.ip_network = NULL;
			    return (SEND_MSG_KO);
			}
					
			// free memory
			free(e.username);
			e.username = NULL;
			free(e.ip_network);
			e.ip_network = NULL;
		} else {
	        // check destination user is not origin user
			if (0 == strcmp(origin_username, e.username)) {
		        pthread_mutex_lock(mutex);
				printMsg(COLOR_RED_TXT);
				printMsg(SEND_MSG_ERROR_SAME_USER);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(mutex);
				// free memory
				free(e.username);
				e.username = NULL;
				free(e.ip_network);
				e.ip_network = NULL;
				return (SEND_MSG_KO);
			}

			// free memory
			free(e.username);
			e.username = NULL;
			free(e.ip_network);
			e.ip_network = NULL;

			// send message
			if (0 != ICP_sendMsg(e.pid, message, origin_username, mutex)) {
			    return (SEND_MSG_KO);
			}
		}

		return (SEND_MSG_OK);
	} else {
	    // show error message for unfound user
		asprintf(&buffer, USER_NOT_FOUND_ERROR_MSG, dest_username);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(buffer);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(buffer);
		buffer = NULL;
	}

	return (SEND_MSG_KO);
}

/*********************************************************************
* @Purpose: Send a file to another IluvatarSon.
* @Params: in: clients = list of users of the sender
*          in: dest_username = string containing the username of the
*		       destination IluvatarSon
*		   in: file = string containing the name of the file to send
*		   in: directory = string containing the directory of the file
*		   in: origin_username = string containing the username of the
*		       sender
*		   in: origin_ip = string with the IP address of the sender
*		   in/out: mutex = screen mutex to prevent writing to screen
*		           simultaneously
* @Return: ----
*********************************************************************/
void sendFileCommand(BidirectionalList clients, char *dest_username, char *file, char *directory,
                     char *origin_username, char *origin_ip, semaphore *sem_mq, pthread_mutex_t *mutex) {
	Element e;
	char *buffer = NULL;

	// search destination user
	if (USER_FOUND == searchUserInList(clients, dest_username, &e)) {
		// check if remote user
		if (IS_REMOTE_USER == checkUserIP(origin_ip, e.ip_network)) {
		    // send file
			if (0 != socketsSendFile(origin_username, e, file, directory, mutex)) {
				// free memory
				free(e.username);
				e.username = NULL;
				free(e.ip_network);
				e.ip_network = NULL;
			    return;
			}
					
			// free memory
			free(e.username);
			e.username = NULL;
			free(e.ip_network);
			e.ip_network = NULL;
		// user in same machine
		} else {
	        // check destination user is not origin user
			if (0 == strcmp(origin_username, e.username)) {
		        pthread_mutex_lock(mutex);
				printMsg(COLOR_RED_TXT);
				printMsg(SEND_MSG_ERROR_SAME_USER);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(mutex);
				// free memory
				free(e.username);
				e.username = NULL;
				free(e.ip_network);
				e.ip_network = NULL;
				return;
			}

			// free memory
			free(e.username);
			e.username = NULL;
			free(e.ip_network);
			e.ip_network = NULL;	
			// send file
			ICP_sendFile(e.pid, file, directory, origin_username, sem_mq, mutex);
		}
	} else {
	    // show error message for unfound user
		asprintf(&buffer, USER_NOT_FOUND_ERROR_MSG, dest_username);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(buffer);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(buffer);
		buffer = NULL;
	}
}

/*********************************************************************
* @Purpose: Executes a custom command given its ID. Currently only
*           prints the selected command.
* @Params: in: id = ID of the custom command to execute
*          in: fd_dest = file descriptor to send frames
*          in: iluvatar = IluvatarSon that executes command
*          in/out: clients = list of clients
*          in/out: command = string containing the command to execute
* @Return: ----
*********************************************************************/
char executeCustomCommand(int id, int fd_dest, IluvatarSon iluvatar, BidirectionalList *clients, char **command, semaphore *sem_mq, pthread_mutex_t *mutex) {
	switch (id) {
	    case IS_UPDATE_USERS_CMD:
			pthread_mutex_lock(mutex);
			printMsg(UPDATE_USERS_SUCCESS_MSG);
			pthread_mutex_unlock(mutex);
			// request list
			GPC_writeFrame(fd_dest, 0x02, GPC_UPDATE_USERS_HEADER_IN, iluvatar.username, strlen(iluvatar.username));
			break;
		case IS_LIST_USERS_CMD:
			printUsersList(*clients, mutex);
			break;
		case IS_SEND_MSG_CMD:
		    if (SEND_MSG_OK == sendMsgCommand(*clients, command[2], command[3], iluvatar.username, iluvatar.ip_address, mutex)) {
			    //TODO: send frame to count new message
			}

			break;
		case IS_SEND_FILE_CMD:
		    sendFileCommand(*clients, command[2], command[3], iluvatar.directory, iluvatar.username, iluvatar.ip_address, sem_mq, mutex);
			break;
		default:
		    // exit command
			GPC_writeFrame(fd_dest, 0x06, GPC_EXIT_HEADER, iluvatar.username, strlen(iluvatar.username));
			break;
	}

	return (0);
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
*          in/out: iluvatar = IluvatarSon issuing command
*		   in: fd_arda = Arda's file descriptor (connected to server)
* @Return: 0 if EXIT command entered, otherwise 1.
*********************************************************************/
int COMMANDS_executeCommand(char *user_input, IluvatarSon *iluvatar, int fd_arda, BidirectionalList *users_list, semaphore *sem_mq, pthread_mutex_t *mutex) {
	char **command = NULL;
	char error = 0;
	int n_args = 0, cmd_id = 0;
	int pid = -1, status = 0;

	n_args = getCmdArgs(user_input, &command);
	// identify command
	cmd_id = identifyCommand(command, n_args, mutex);

	if ((ERROR_CMD_ARGS != cmd_id) && (IS_NOT_CUSTOM_CMD != cmd_id)) {
	    // execute custom command
		error = executeCustomCommand(cmd_id, fd_arda, *iluvatar, users_list, command, sem_mq, mutex);

		if ((cmd_id == IS_EXIT_CMD) && !error) {
			freeMemCmd(&command, &n_args);
			//SHAREDFUNCTIONS_freeIluvatarSon(iluvatar);
			return (1);
		}
	} else if (IS_NOT_CUSTOM_CMD == cmd_id) {
	    pid = fork();

		switch (pid) {
		    case -1:
			    // error
				pthread_mutex_lock(mutex);
				printMsg(COLOR_RED_TXT);
				printMsg(ERROR_CREATING_CHILD);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(mutex);
				break;
			case 0:
				// execute as Linux command
				status = execvp(command[0], command);

				// free mem
				freeMemCmd(&command, &n_args);
				SHAREDFUNCTIONS_freeIluvatarSon(iluvatar);
				// Free memory and close file descriptors
				raise(SIGINT);
				break;
			default:
				wait(&status);
				
				if (status != 0) {		
				    // Invalid Linux command
					pthread_mutex_lock(mutex);
					printMsg(UNKNOWN_CMD_MSG);
					pthread_mutex_unlock(mutex);
					GPC_writeFrame(fd_arda, 0x07, GPC_UNKNOWN_CMD_HEADER, NULL, 0);
				}

				break;
		}
	}

	// free mem
	freeMemCmd(&command, &n_args);

	return (0);
}
