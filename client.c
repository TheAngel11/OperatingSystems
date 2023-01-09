/*********************************************************************
* @Purpose: Module to manage clients of a Server (see server module).
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 10/12/2022
* @Last change: 09/01/2023
*********************************************************************/
#include "client.h"

Client CLIENT_init(char *ip, int port) {
    Client c;
	struct sockaddr_in server;

	c.server_fd = FD_NOT_FOUND;

	// config socket
	if ((c.server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_CREATING_SOCKET_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		return (c);
	}

	bzero(&server, sizeof(server)); 
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &server.sin_addr) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_IP_CONFIGURATION_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		// finish execution
		close(c.server_fd);
		c.server_fd = FD_NOT_FOUND;
		return (c);
	}

	// connect to server
	if (connect(c.server_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_SERVER_CONNECTION_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		// finish execution
		close(c.server_fd);
		c.server_fd = FD_NOT_FOUND;
		return (c);
	}

	return (c);
}

/*********************************************************************
* @Purpose: Gets a list of users given a string containing the users
*           and their data.
* @Params: in: users = string containing the users and their data.
*          in: length = length of the string.
* @Return: Returns a bidirectional list of users.
*********************************************************************/
BidirectionalList getListFromString(char *users, int length) {
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

char CLIENT_manageArdaServerAnswer(Client *c, BidirectionalList *users_list, pthread_mutex_t *mutex) {
	char *buffer = NULL;
	char *header = NULL;
	char type = 0x07;
	char ret_value = 0;

	// If the read frame returns 0, it means that the connection has been closed
	if (GPC_readFrame(c->server_fd, &type, &header, &buffer) == 0) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(ARDA_CONNECTION_CLOSED_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		ret_value = 1;
	} else if ((0 == strcmp(header, GPC_UPDATE_USERS_HEADER_OUT)) && (0x02 == type)) {
		// Manage UPDATE USERS
		BIDIRECTIONALLIST_destroy(users_list);
		*users_list = getListFromString(buffer, (int) strlen(buffer));
		free(buffer);
		buffer = NULL;
	} else if ((0 == strcmp(header, GPC_HEADER_CONOK)) && (0x06 == type)) {
		// Manage EXIT
		pthread_mutex_lock(mutex);
		printMsg(COLOR_DEFAULT_TXT);
		printMsg(EXIT_ARDA_MSG);
		pthread_mutex_unlock(mutex);
		ret_value = 1;
	} else {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(ERROR_DISCONNECT_ILUVATAR_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
	}

	// free memory
	if (NULL != buffer) {
	    free(buffer);
		buffer = NULL;
	}

	if (NULL != header) {
	    free(header);
		header = NULL;
	}

	return (ret_value);
}

char CLIENT_sendMsg(Client *c, char **data, pthread_mutex_t *mutex) {
	char *header = NULL;
	char type = 0x07;
	char *buffer = NULL;

	// check frame
	if (GCP_FRAME_OK == GCP_checkFrameFormat(GCP_SEND_MSG_TYPE, GCP_SEND_MSG_HEADER, *data)) {
	    // Send the message
		GPC_writeFrame(c->server_fd, GCP_SEND_MSG_TYPE, GCP_SEND_MSG_HEADER, *data, strlen(*data));
		free(*data);
		*data = NULL;
	} else {
	    // error
		asprintf(&buffer, GCP_WRONG_FORMAT_ERROR_MSG, GCP_SEND_MSG_TYPE, GCP_SEND_MSG_HEADER);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(buffer);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(*data);
		*data = NULL;
		free(buffer);
		buffer = NULL;
		return (1);
	}
	
	// Get the answer
	GPC_readFrame(c->server_fd, &type, &header, NULL);

	if (0 == strcmp(header, GPC_HEADER_MSGKO)) {
	    // print error message
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: Message have not been correctly sent\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory and close socket
		free(header);
		header = NULL;
		close(c->server_fd);
		return (1);
	} else if (0 == strcmp(header, GPC_HEADER_MSGOK)) {
	    pthread_mutex_lock(mutex);
		printMsg("Message correctly sent\n");
		pthread_mutex_unlock(mutex);
	}

	// close socket and free memory
	close(c->server_fd);
	free(header);
	header = NULL;
	return (0);
}

char CLIENT_sendFile(Client *c, char **data, int *fd_file, int file_size, pthread_mutex_t *mutex) {
	char *header = NULL;
	char type = 0x07;
	char *buffer = NULL;

	// check frame
	if (GCP_FRAME_KO == GCP_checkFrameFormat(GCP_SEND_FILE_TYPE, GCP_SEND_FILE_INFO_HEADER, *data)) {
	    // error
		asprintf(&buffer, GCP_WRONG_FORMAT_ERROR_MSG, GCP_SEND_FILE_TYPE, GCP_SEND_FILE_INFO_HEADER);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(buffer);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(*data);
		*data = NULL;
		free(buffer);
		buffer = NULL;
		close(*fd_file);
		return (1);
	}
	
	// Send the information message
	if (GCP_WRITE_KO == GPC_writeFrame(c->server_fd, GCP_SEND_FILE_TYPE, GCP_SEND_FILE_INFO_HEADER, *data, strlen(*data))) {
	    free(*data);
		*data = NULL;
		close(*fd_file);
		return (1);
	}
	
	free(*data);
	*data = NULL;

	// Send the file
	while (file_size > GPC_FILE_MAX_BYTES) {
		*data = (char *) malloc(sizeof(char) * (GPC_FILE_MAX_BYTES + 1));
		read(*fd_file, *data, GPC_FILE_MAX_BYTES);
		(*data)[GPC_FILE_MAX_BYTES] = '\0';
		
		if (GCP_WRITE_KO == GPC_writeFrame(c->server_fd, GCP_SEND_FILE_TYPE, GCP_SEND_FILE_DATA_HEADER, *data, GPC_FILE_MAX_BYTES)) {
		    free(*data);
			*data = NULL;
			close(*fd_file);
			return (1);
		}
		
		free(*data);
		*data = NULL;
		file_size -= GPC_FILE_MAX_BYTES;
	}

	*data = (char *) malloc(sizeof(char) * (file_size + 1));
	read(*fd_file, *data, file_size);
	(*data)[file_size] = '\0';
	
	if (GCP_WRITE_KO == GPC_writeFrame(c->server_fd, GCP_SEND_FILE_TYPE, GCP_SEND_FILE_DATA_HEADER, *data, file_size)) {
	    free(*data);
		*data = NULL;
		close(*fd_file);
		return (1);
	}
	
	// free memory and close file
	free(*data);
	*data = NULL;
	close(*fd_file);
	// Get the md5sum answer
	GPC_readFrame(c->server_fd, &type, &header, NULL);

	if (0 == strcmp(header, GPC_SEND_FILE_HEADER_KO_OUT) || type != 0x05) {
	    // print error message
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The file sent has lost its integrity\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory and close socket
		free(header);
		header = NULL;
		close(c->server_fd);
		return (1);
	} else if (0 == strcmp(header, GPC_SEND_FILE_HEADER_OK_OUT) && type == 0x05) {
	    pthread_mutex_lock(mutex);
		printMsg("File correctly sent\n");
		pthread_mutex_unlock(mutex);
	}

	// close socket and free memory
	close(c->server_fd);
	free(header);
	header = NULL;
	return (0);
}
