/*********************************************************************
* @Purpose: Module for the General Protocol Communications.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 11/12/2022
* @Last change: 11/12/2022
*********************************************************************/
#include "gpc.h"

/**********************************************************************
* @Purpose: Reads a frame sent through the network.
* @Params: in: fd = file descriptor to read from.
*          in/out: type = type of frame received.
*          in/out: header = header to get from frame.
*          in/out: data = data to get from frame.
* @Return: Returns 1.
***********************************************************************/
char GPC_readFrame(int fd, char *type, char **header, char **data) {
	char *buffer = NULL;
	char byte = 0x07;
	unsigned short length = 0;

	// read type (1 byte)
	read(fd, &byte, sizeof(char));

	switch (byte) {
	    case 'A':
		    *type = 0x0A;
			break;
		case 'B':
		    *type = 0x0B;
			break;
		case 'C':
		    *type = 0x0C;
			break;
		case 'D':
		    *type = 0x0D;
			break;
		case 'E':
		    *type = 0x0E;
			break;
		case 'F':
		    *type = 0x0F;
			break;
		default:
		    *type = byte - '0';
			break;
	}
	
	// skip '['
	read(fd, &byte, sizeof(char));
	
	// read header
	buffer = SHAREDFUNCTIONS_readUntil(fd, ']');
	*header = strdup(buffer);
	free(buffer);
	buffer = NULL;
	// read lenght (2 bytes)
	read(fd, &length, 2);

	// read data (lenght bytes)
	if (0 < length) {
	    *data = (char *) malloc (sizeof(char) * (length + 1));
		read(fd, *data, sizeof(char) * length);
		(*data)[length] = '\0';
	}

	return (1);
}

/**********************************************************************
* @Purpose: Write a frame to the given file descriptor.
* @Params: in: fd = file descriptor to write.
* 		   in: type = type of frame to send.
* 		   in: header = header of frame to send.
* 		   in: data = data to send.
* @Return: Returns 1.
**********************************************************************/
char GPC_writeFrame(int fd, char type, char *header, char *data) {
	char *frame = NULL;
 	char byte = 0;
	unsigned short length = 0;
	char length_lsB = 0, length_msB = 0;
	int i = 0, j = 0;
	int size = 0;

	if (NULL != data) {
	    length = strlen(data);
	}

	size = 1 + ((int) strlen(header)) + 2 + 2 + length;
	frame = (char *) malloc (sizeof(char) * size);

	// write type (1 byte)
	switch (type) {
	    case 0x0A:
		    byte = 'A';
			break;
		case 0x0B:
		    byte = 'B';
			break;
		case 0x0C:
		    byte = 'C';
			break;
		case 0x0D:
		    byte = 'D';
			break;
		case 0x0E:
		    byte = 'E';
			break;
		case 0x0F:
		    byte = 'F';
			break;
		default:
		    byte = type + '0';
			break;
	}
	
	i = sprintf(frame, "%c[%s]", byte, header);

	// write lenght (2 bytes)
	if (data != NULL) {
		length_msB = (char) ((length >> 8) & 0x00FF); 	// shift MSB
		length_lsB = (char) (length & 0x00FF);			// store LSB
		frame[i] = length_lsB;
		frame[i + 1] = length_msB;
		i += 2;
		// write data (length bytes)
		for (j = 0; j < length; j++) {
		    frame[i] = data[j];
			i++;
		}
	} else {
		frame[i] = 0;
		frame[i + 1] = 0;
		i += 2;
	}

	// write entire frame
	write(fd, frame, size);
	free(frame);
	frame = NULL;

	return (1);
}

/**********************************************************************
* @Purpose: Given the data of a frame containing the attributes of a
*           user, parses the user and stores it in list element.
* @Params: in: data = data containing the name, IP, port and PID of the
*              user.
* 		   in/out: e = instance of Element to store user.
* @Return: Returns 1.
**********************************************************************/
void GPC_parseUserFromFrame(char *data, Element *e) {
    char *buffer = NULL;
	int i = 0;

	// get username
	e->username = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
	// get IP
	e->ip_network = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
	// get port
	buffer = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
	e->port = atoi(buffer);
	free(buffer);
	buffer = NULL;
	// get PID
	buffer = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
	e->pid = atoi(buffer);
	free(buffer);
	buffer = NULL;
}

/**********************************************************************
* @Purpose: Given the data of a frame containing the users, updates
*           list of users with the given ones.
* @Params: in/out: list = list of users.
* 		   in: users = data of the frame with the users' attributes.
* @Return: Returns 1.
**********************************************************************/
char GPC_updateUsersList(BidirectionalList *list, char *users) {
    Element element;
	char *user = NULL;
	int i = 0;

	// reset list
	BIDIRECTIONALLIST_makeEmpty(list);
	BIDIRECTIONALLIST_goToTail(list);

	// add users
	do {
	    user = SHAREDFUNCTIONS_splitString(users, GPC_USERS_SEPARATOR, &i);
		GPC_parseUserFromFrame(user, &element);
		free(user);
		user = NULL;
		BIDIRECTIONALLIST_addAfter(list, element);
		free(element.username);
		element.username = NULL;
		free(element.ip_network);
		element.ip_network = NULL;
	} while (i < (int) strlen(users));

	return (1); 
}

/**********************************************************************
* @Purpose: Given the origin user and the message, creates the data field 
*			of a send message frame.
* @Params: in/out: originUser = the user who sends the message
*	.	   in/out: message = the message that the origin user sends
* @Return: Returns data of the frame or NULL if there is no data.
**********************************************************************/
char * GPC_sendMessage(char *originUser, char *message) {
	char *data = NULL;
	
	if(strcmp(message, "") == 0) {
		return NULL; 
	}

	// TODO Remove the first and last character of the message ("") ??
	message = message + 1;
	message[strlen(message) - 1] = '\0';

	// given the origin user and the message, creates the data field 
	// of a send message frame
	data = (char *) malloc (sizeof(char) * (strlen(originUser) + strlen(message) + 2));
	sprintf(data, "%s%c%s", originUser, GPC_DATA_SEPARATOR, message);
	return data;
} 

/**********************************************************************
* @Purpose: Given the data of a send message frame, finds the origin user and the message 
* @Params: in/out: data = the user who sends the message
* 		   in/out: originUser = the user who sends the message
*		   in/out: message = the message that the origin user sends
* @Return: Returns 1.
**********************************************************************/
void GPC_parseSendMessage(char *data, char **originUser, char **message) {
	int i = 0;
	//data is in the format: originUser + GPC_DATA_SEPARATOR + message
	*originUser = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
	*message = SHAREDFUNCTIONS_splitString(data, GPC_DATA_SEPARATOR, &i);
} 

/**********************************************************************
* @Purpose: Turns a list of users into a string following the GPC
*           format so that it can be sent in a frame.
* @Params: in: blist = list of the clients connected to the server.
* @Return: Returns a string containing the data in GPC format.
**********************************************************************/
char * GPC_getUsersFromList(BidirectionalList blist) {
	char *data = NULL;
	char *buffer = NULL;
	int size = 0, n = 0;
	int flag_first = 1;
	Element element;

	BIDIRECTIONALLIST_goToHead(&blist);

	while (BIDIRECTIONALLIST_isValid(blist)) {
		element = BIDIRECTIONALLIST_get(&blist);

		if (flag_first) {
			size = asprintf(&data, "%s&%s&%d&%d", element.username, element.ip_network, element.port, (int) element.pid);
			flag_first = 0;
		} else {
			n = asprintf(&buffer, "#%s&%s&%d&%d", element.username, element.ip_network, element.port, (int) element.pid);
			size += n + 1;
			data = (char *) realloc (data, sizeof(char) * size);
			strcat(data, buffer);
		}
		
		BIDIRECTIONALLIST_next(&blist);
		free(element.username);
		element.username = NULL;
		free(element.ip_network);
		element.ip_network = NULL;
	}

	return (data);
}
