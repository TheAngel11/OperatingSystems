/*********************************************************************
* @Purpose: Module that contains functions to manage the custom
*           commands for an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 06/01/2023
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

	// We reserve memory for the hostname
	hostname = (char *) malloc(strlen(host->h_name) + 1);
	// We copy the hostname to the reserved memory
	strcpy(hostname, host->h_name);
	return (hostname);
}

/*********************************************************************
* @Purpose: Gets a list of users given a string containing the users
*           and their data.
* @Params: in: users = string containing the users and their data.
*          in: length = length of the string.
* @Return: Returns a bidirectional list of users.
*********************************************************************/
BidirectionalList COMMANDS_getListFromString(char *users, int length) {
    BidirectionalList list = BIDIRECTIONALLIST_create();
	Element user;
	char *buffer = NULL;
	char *tmp = NULL;
	int i = 0, j = 0;

	while (i < length) {
	    // get single user
		buffer = SHAREDFUNCTIONS_splitString(users, GPC_USERS_SEPARATOR, &i);
		user.username = SHAREDFUNCTIONS_splitString(buffer, GPC_DATA_SEPARATOR, &j);
		user.ip_network = SHAREDFUNCTIONS_splitString(buffer, GPC_DATA_SEPARATOR, &j);
		tmp = SHAREDFUNCTIONS_splitString(buffer, GPC_DATA_SEPARATOR, &j);
		user.port = atoi(tmp);
		free(tmp);
		tmp = NULL;
		tmp = SHAREDFUNCTIONS_splitString(buffer, GPC_DATA_SEPARATOR, &j);
		user.pid = atoi(tmp);
		free(tmp);
		tmp = NULL;
		// add to list
		BIDIRECTIONALLIST_addAfter(&list, user);
		// next user
		free(buffer);
		buffer = NULL;
		free(user.username);
		user.username = NULL;
		free(user.ip_network);
		user.ip_network = NULL;
		j = 0;
	}

	return (list);
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
 * @Purpose: Finds a user in a list of users.
 * @Params: in: users = list of users.
 * 			in: username = username of the user to find.
 * @Return: Returns the user if found, otherwise returns an empty user.
 *********************************************************************/
Element findUserByList(BidirectionalList users, char *username) {
	Element user;

	if (!BIDIRECTIONALLIST_isEmpty(users)) {
	    BIDIRECTIONALLIST_goToHead(&users);

		while (BIDIRECTIONALLIST_isValid(users)) {
	        user = BIDIRECTIONALLIST_get(&users);

			if (0 == strcasecmp(user.username, username)) {
				return (user);
			}

			free(user.username);
			free(user.ip_network);
			user.username = NULL;
			user.ip_network = NULL;
			BIDIRECTIONALLIST_next(&users);
		}
	}
	
	return (user);
}

/*********************************************************************
* @Purpose: Sends a message to a user using sockets.
* @Params: in: iluvatar = iluvatar son.
* 		   in: e = element with the user to send the message.
* 		   in: msg = message to send.
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if the message was sent successfully, otherwise returns 1 or -1.
*********************************************************************/
char socketsSendMsg(IluvatarSon iluvatar, Element e, char *msg, pthread_mutex_t *mutex) {
	Client client;
	char *data = NULL;
	char *header = NULL;
	char type = 0x07;

	data = GPC_sendMessage(iluvatar.username, msg);
	
	if (data != NULL) {
		// Open socket
		client = CLIENT_init(e.ip_network, e.port);
		
		// Check client
		if (FD_NOT_FOUND == client.server_fd) {
			raise(SIGINT);
		}
		
		// Send the message
		GPC_writeFrame(client.server_fd, 0x03, GPC_SEND_MSG_HEADER_IN, data, strlen(data));
		free(data);
		data = NULL;
		// Get the answer
		GPC_readFrame(client.server_fd, &type, &header, NULL);

		if (0 == strcmp(header, GPC_HEADER_MSGKO)) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg("ERROR: Message have not been correctly sent\n");
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			free(header);
			header = NULL;
			close(client.server_fd);
			return (-1);
		} else if (0 == strcmp(header, GPC_HEADER_MSGOK)) {
		    pthread_mutex_lock(mutex);
			printMsg("Message correctly sent\n");
			pthread_mutex_unlock(mutex);
		}

		close(client.server_fd);
		free(header);
		header = NULL;
	} else {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: This is not a valid message to send\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
	}

	return (0);
}

/*********************************************************************
* @Purpose: Sends a file to a user using sockets.
* @Params: in: iluvatar = iluvatar son.
* 			in: e = element with the user to send the file.
* 			in: filename = filename to send.
* @Return: Returns 0 if the file was sent successfully, otherwise returns 1 or -1.
*********************************************************************/
char socketsSendFile(IluvatarSon iluvatar, Element e, char *filename, pthread_mutex_t *mutex) {
	char *filename_path = NULL;
	int file_size = 0;
	char *md5sum = NULL;
	Client client;
	char *data = NULL;
	char *header = NULL;
	char type = 0x07;
	int fd_file = FD_NOT_FOUND;

	asprintf(&filename_path, ".%s/%s", iluvatar.directory, filename);
	fd_file = open(filename_path, O_RDONLY);

	// check file FD
	if (fd_file == FD_NOT_FOUND) {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: File not found (fd)\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		return (-1);
	}

	// get file size
	file_size = (int) lseek(fd_file, 0, SEEK_END);
	lseek(fd_file, 0, SEEK_SET);

	if (file_size == 0) {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: File is empty\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		return (-1);
	}

	// get MD5SUM
	md5sum = GPC_getMD5Sum(filename_path);
	// Prepare the data to send
	data = GPC_sendFile(iluvatar.username, filename, file_size, md5sum);
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

		// Send the information message
		GPC_writeFrame(client.server_fd, 0x04, GPC_SEND_FILE_INFO_HEADER_IN, data, strlen(data));
		free(data);
		data = NULL;

		// Send the file
		while (file_size > GPC_FILE_MAX_BYTES) {
			data = (char *) malloc(sizeof(char)  * GPC_FILE_MAX_BYTES);
			read(fd_file, data, GPC_FILE_MAX_BYTES);
			GPC_writeFrame(client.server_fd, 0x04, GPC_SEND_FILE_DATA_HEADER_IN, data, GPC_FILE_MAX_BYTES);			
			free(data);
			data = NULL;
			file_size -= GPC_FILE_MAX_BYTES;
		}

		data = (char *) malloc(sizeof(char)  * file_size);
		read(fd_file, data, file_size);
		GPC_writeFrame(client.server_fd, 0x04, GPC_SEND_FILE_DATA_HEADER_IN, data, file_size);
		free(data);
		data = NULL;
		close(fd_file);

		// Get the md5sum answer
		GPC_readFrame(client.server_fd, &type, &header, NULL);
		if (0 == strcmp(header, GPC_SEND_FILE_HEADER_KO_OUT) || type != 0x05) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg("ERROR: The file sent has lost its integrity\n");
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			free(header);
			header = NULL;	
			close(client.server_fd);
			return (-1);
		} else if (0 == strcmp(header, GPC_SEND_FILE_HEADER_OK_OUT) && type == 0x05) {
		    pthread_mutex_lock(mutex);
			printMsg("File correctly sent\n");
			pthread_mutex_unlock(mutex);
		}

		close(client.server_fd);
		free(header);
		header = NULL;
	} else {
	    pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: This is not a valid file to send\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
	}

	return (0);
}

/*********************************************************************
* @Purpose: Sends a file to a user using message queues.
* @Params: in: qfd = message queue file descriptor
* 		   in: filename = name of the file to send
* 		   in: iluvatar = instance of IluvatarSon
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if the file was sent successfully, otherwise returns 1 or -1.
*********************************************************************/
char mqSendFile(mqd_t qfd, char *filename, IluvatarSon iluvatar, pthread_mutex_t *mutex) {
	char *filename_path = NULL;
	int file_size = 0;
	char *md5sum = NULL;
	char *message_send = NULL;
	char *message_recv = NULL;
	int fd_file = FD_NOT_FOUND;

	asprintf(&filename_path, ".%s/%s", iluvatar.directory, filename);
	fd_file = open(filename_path, O_RDONLY);

	// check file FD
	if (fd_file == FD_NOT_FOUND) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: File not found (fd)\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		return (-1);
	}

	// get file size
	file_size = (int) lseek(fd_file, 0, SEEK_END);
	lseek(fd_file, 0, SEEK_SET);
	
	if (file_size == 0) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: File is empty\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		return (-1);
	}

	// Get the MD5SUM
	md5sum = GPC_getMD5Sum(filename_path);
	// Prepare the message to send
	message_send = GPC_createNeighborMessageFileInfo(iluvatar.username, filename, file_size, md5sum);
	free(filename_path);
	filename_path = NULL;
	free(md5sum);
	md5sum = NULL;
	
	// Send the file info message
	if (mq_send(qfd, message_send, strlen(message_send) + 1, 0) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The message could not be sent\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		return (-1);
	}

	free(message_send);
	message_send = NULL;

	// Send the file in fragments if bigger than ICP_FILE_MAX_BYTES
	while (file_size > GPC_FILE_MAX_BYTES) {
		message_send = (char *) malloc(sizeof(char)  * GPC_FILE_MAX_BYTES);
		read(fd_file, message_send, GPC_FILE_MAX_BYTES);
		
		if (mq_send(qfd, message_send, GPC_FILE_MAX_BYTES, 0) == -1) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg("ERROR: The message could not be sent\n");
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			free(message_send);
			message_send = NULL;
			return (-1);
		}
		
		free(message_send);
		message_send = NULL;
		file_size -= GPC_FILE_MAX_BYTES;
	}
	
	// Send the last part of the file
	message_send = (char *) malloc(sizeof(char) * file_size);
	read(fd_file, message_send, file_size);

	if (mq_send(qfd, message_send, file_size, 0) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The message could not be sent\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		free(message_send);
		message_send = NULL;
		return (-1);
	}

	free(message_send);
	message_send = NULL;
	close(fd_file);
	
	struct mq_attr attr;
	// get the attributes of the queue
	if (mq_getattr(qfd, &attr) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The attributes of the queue could not be obtained\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		perror("mq_getattr");
		return (-1);
	}
	
	// Receive the answer
	/*message_recv = (char *) malloc((attr.mq_msgsize + 1) * sizeof(char));	//TODO: S'ha de crear un semafor de sincronitzacio (ho explico a iluvatarSon linia 488 aprox)
	if(mq_receive(qfd, message_recv, attr.mq_msgsize, NULL) == -1) {	//Todo: Change the 8 to a constant (BÉ DEL FILE_OK O FILE_KO)
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The message could not be received\n");
		printMsg(COLOR_DEFAULT_TXT);
		perror("woefdhaspfdc");
		free(message_recv);
		message_recv = NULL;
		return (-1);
	}*/

	/*if(0 == strcmp(message_recv, "FILE_OK")) {
		printMsg("File correctly sent\n");
	} else {
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The file sent has lost its integrity\n");
		printMsg(COLOR_DEFAULT_TXT);
		free(message_recv);
		message_recv = NULL;
		return (-1);
	}*/

	if(message_recv != NULL) {
		free(message_recv);
		message_recv = NULL;
	}

	if(message_send != NULL) {
		free(message_send);
		message_send = NULL;
	}
	
	return (0);
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
char executeCustomCommand(int id, int fd_dest, IluvatarSon iluvatar, BidirectionalList *clients, char **command, pthread_mutex_t *mutex) {
    char *buffer = NULL;
	char *message_send = NULL;
	char *header = NULL;
	mqd_t qfd;
	char returnCode = 0;
	Element e;
	char *destHostname = NULL;
	char *originHostname = NULL;

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
		    // send frame to count new message
		case IS_SEND_FILE_CMD:
			// Find the user
			e = findUserByList(*clients, command[2]);
			if (e.username != NULL && e.ip_network != NULL) {
				if (strchr(command[3], '.') != NULL || id == IS_SEND_MSG_CMD) {
					// We get the hostnames to compare them
					originHostname = getHostnameByIP(iluvatar.ip_address);
					destHostname = getHostnameByIP(e.ip_network);

					// We get the hostname of the origin and destination to check if they are in the same machine
					if (strcmp(destHostname, originHostname) == 0) {
						// If they are in the same machine, we use message queues
						// We check if the user is sending a message to himself
						if (strcasecmp(e.username, iluvatar.username) != 0) {
							// Open the queue
							asprintf(&buffer, "/%d", e.pid);
							qfd = mq_open(buffer, O_RDWR);
							free(buffer);
							buffer = NULL;

							switch (id) {
								case IS_SEND_MSG_CMD:
									message_send = GPC_createNeighborMessageMsg(iluvatar.username, command[3]);
									
									// We send the message
									if (mq_send(qfd, message_send, strlen(message_send) + 1, 0) == -1) {
										printMsg(COLOR_RED_TXT);
										printMsg("ERROR: The message could not be sent\n");
										printMsg(COLOR_DEFAULT_TXT);
									} else {
										printMsg("Message correctly sent\n"); 
									}
									free(message_send);
									message_send = NULL;
									break;
								case IS_SEND_FILE_CMD:
									returnCode = mqSendFile(qfd, command[3], iluvatar, mutex);
									break;
							}				
							mq_close(qfd);
							
						} else {
							printMsg(COLOR_RED_TXT);
							printMsg("ERROR: You can not send a message to yourself\n");
							printMsg(COLOR_DEFAULT_TXT);
						}

					} else {
						// If they are in different machines, we use sockets
						switch (id) {
							case IS_SEND_MSG_CMD:
								returnCode = socketsSendMsg(iluvatar, e, command[3], mutex);
								break;

							case IS_SEND_FILE_CMD:
								returnCode = socketsSendFile(iluvatar, e, command[3], mutex);
								break;
						}
					}
					free(originHostname);
					free(destHostname);
					free(e.username);
					free(e.ip_network);
				
					if(returnCode == -1 || returnCode == 1) {
						return returnCode;
					}
				} else {
					printMsg(COLOR_RED_TXT);
					printMsg("ERROR: You need to send a file with an extension!\n");
					printMsg(COLOR_DEFAULT_TXT);
				}

			} else {
				printMsg(COLOR_RED_TXT);
				printMsg("ERROR: User not found\n");
				printMsg(COLOR_DEFAULT_TXT);
			}

			break;
		default:
		    // exit command
			GPC_writeFrame(fd_dest, 0x06, GPC_EXIT_HEADER, iluvatar.username, strlen(iluvatar.username));
			break;
	}

	if (NULL != header) {
	    free(header);
		header = NULL;
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
int COMMANDS_executeCommand(char *user_input, IluvatarSon *iluvatar, int fd_arda, BidirectionalList *users_list, pthread_mutex_t *mutex) {
	char **command = NULL;
	char error = 0;
	int n_args = 0, cmd_id = 0;
	int pid = -1, status = 0;

	n_args = getCmdArgs(user_input, &command);
	// identify command
	cmd_id = identifyCommand(command, n_args, mutex);

	if ((ERROR_CMD_ARGS != cmd_id) && (IS_NOT_CUSTOM_CMD != cmd_id)) {
	    // execute custom command
		error = executeCustomCommand(cmd_id, fd_arda, *iluvatar, users_list, command, mutex);

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
