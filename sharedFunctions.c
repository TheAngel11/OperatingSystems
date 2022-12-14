/*********************************************************************
* @Purpose: Module that contains different functions that are shared
*           among other modules.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 18/10/2022
* @Last change: 17/11/2022
*********************************************************************/
#include "sharedFunctions.h"

/*********************************************************************
* @Purpose: Read a string from a file descriptor, stopping at a given
*           char.
* @Params: in: fd = file descriptor
*          in: delimiter = char marking where to stop reading
* @Return: Returns a string.
*********************************************************************/
char * SHAREDFUNCTIONS_readUntil(int fd, char delimiter) {
    char *buffer = NULL;
	char byte = delimiter + 1;
	int i = 0, n = -1;

	while ((byte != delimiter) && (n != 0)) {
		n = read(fd, &byte, 1);
		if ((byte != delimiter) && (n != 0)) {
		    if (NULL == buffer) {
			    buffer = (char *) malloc (sizeof(char));
			}
			buffer[i] = byte;
			i++;
			buffer = (char *) realloc (buffer, sizeof(char) * (i + 1));
		}
	}

	buffer[i] = '\0';

	return (buffer);
}

/*********************************************************************
* @Purpose: Splits a string given a char.
* @Params: in: string = original string to split
*          in: delimiter = char marking split point
*		   in/out: pos = int that indicates the position at which to
*		                 start parsing the string. After split is done
*						 it is updated so that consecutive splits of
*						 the same string can be done.
* @Return: Returns a string containin a part of the original string.
*********************************************************************/
char * SHAREDFUNCTIONS_splitString(char *string, char delimiter, int *pos) {
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

/**********************************************************************
* @Purpose: Remove a char repeatedly from a string.
* @Params: in/out: string = string to modify
*          in: unwanted = char we want to remove
* @Return: ----
***********************************************************************/
char * SHAREDFUNCTIONS_removeChar(char *string, char unwanted) {
    char *output = (char *) malloc (sizeof(char));
	int i = 0, j = 0;

	for (i = 0; i < (int) strlen(string); i++) {
	    if (string[i] != unwanted) {
		    output[j] = string[i];
			j++;
			output = (char *) realloc(output, sizeof(char) * (j + 1));
		}
	}

	output[j] = '\0';
	// update string
	free(string);

	return (output);
}

/**********************************************************************
* @Purpose: Frees dynamic memory allocated for an IluvatarSon.
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing an
*                                iluvatarSon
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_freeIluvatarSon(IluvatarSon *iluvatarSon) {
    if (NULL != iluvatarSon->username) {
	    free(iluvatarSon->username);
		iluvatarSon->username = NULL;
	}
	if (NULL != iluvatarSon->directory) {
        free(iluvatarSon->directory);
		iluvatarSon->directory = NULL;
	}
	if (NULL != iluvatarSon->ip_address) {
        free(iluvatarSon->ip_address);
		iluvatarSon->ip_address = NULL;
	}
	if (NULL != iluvatarSon->arda_ip_address) {
        free(iluvatarSon->arda_ip_address);
		iluvatarSon->arda_ip_address = NULL;
	}
}

/**********************************************************************
* @Purpose: Frees dynamic memory allocated for the server Arda.
* @Params: in/out: arda = Arda pointer referencing the server Arda
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_freeArda(Arda *arda) {
	if (NULL != arda->ip_address) {
        free(arda->ip_address);
		arda->ip_address = NULL;
	}
	if (NULL != arda->directory) {
        free(arda->directory);
		arda->directory = NULL;
	}
}

/**********************************************************************
* @Purpose: Reads the network frame that the client have sent.
* @Params: in: fd 	= the file descriptor we want to read from
*          in/out: type = type of the frame passed by reference
*          in/out: header = header of the frame passed by reference
*          in/out: data = data of the frame passed by reference
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_readFrame(int fd, int *type, char *header, char *data) {
	char *buffer = NULL;
	char byte;
	int length = 0;

	// read type (1 byte)
	read(fd, &byte, sizeof(char));

	switch (byte) {
	case 'A':
		*type = 10;
		break;
	case 'B':
		*type = 11;
		break;
	case 'C':
		*type = 12;
		break;
	case 'D':
		*type = 13;
		break;
	case 'E':
		*type = 14;
		break;
	case 'F':
		*type = 15;
		break;
	default:
		*type = byte - '0';
		break;
	}

	// read header
	buffer = SHAREDFUNCTIONS_readUntil(fd, ']');

	// removes first character
	buffer = buffer + 1;
	// removes last character
	buffer[strlen(buffer) - 1] = '\0';

	header = (char *) malloc (sizeof(char) * (strlen(buffer) + 1));
	strcpy(header, buffer);
	free(buffer);

	// read lenght (2 bytes)
	read(fd, buffer, sizeof(char) * 2);
	length = atoi(buffer);
	free(buffer);

	// read data (lenght bytes)
	read(fd, buffer, sizeof(char) * length);

	data = (char *) malloc (sizeof(char) * (strlen(buffer) + 1));
	strcpy(data, buffer);
	free(buffer);
}

/**********************************************************************
 * @Purpose: Reads from a file descriptor until a given char is found.
 * @Params: in: fd = the file descriptor we want to read from
 * 			in: type = the type of the frame
 * 			in/out: header = header of the frame passed by reference
 * 			in/out: data = data of the frame passed by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_writeFrame(int fd, int type, char *header, char *data) {
	char *buffer = NULL;
	char *frame = NULL;
	int length = 0;

	// write type (1 byte)
	switch (type) {
	case 10:
		buffer = "A";
		break;
	case 11:
		buffer = "B";
		break;
	case 12:
		buffer = "C";
		break;
	case 13:
		buffer = "D";
		break;
	case 14:
		buffer = "E";
		break;
	case 15:
		buffer = "F";
		break;
	default:
		asprintf(&buffer, "%d", type);
		break;
	}

	frame = (char *) malloc (sizeof(char) * (strlen(buffer) + 1));
	strcpy(frame, buffer);

	if(buffer != NULL) {
		free(buffer);
	}

	// write header
	asprintf(&buffer, "[%s]", header);
	frame = (char *) realloc(frame, sizeof(char) * (strlen(frame) + strlen(buffer)));
	strcat(frame, buffer);
	free(buffer);

	// write lenght (2 bytes)
	if(data != NULL) {
		length = strlen(data);
		asprintf(&buffer, "%d", length);
		frame = (char *) realloc(frame, sizeof(char) * (strlen(frame) + strlen(buffer)));
		strcat(frame, buffer);
		free(buffer);
		
		// write data (lenght bytes)
		frame = (char *) realloc(frame, sizeof(char) * (strlen(frame) + strlen(data)));
		strcat(frame, data);
	} else {
		asprintf(&buffer, "%d", length);
		frame = (char *) realloc(frame, sizeof(char) * (strlen(frame) + strlen(buffer)));
		strcat(frame, buffer);
		free(buffer);
	}

	// write frame to the client fd
	write(fd, frame, strlen(frame));
}

/**********************************************************************
 * @Purpose: Parses the data field of a frame
 * @Params: in/out: data = the data field of the frame
 * 			in/out: username = the username of the client passed by 
 * 							   reference
 * 			in/out: ip = the ip address of the client passed by 
 * 						 reference
 * 			in/out: port = the port of the client passed by reference
 *  		in/out: pid = the pid of the client passed by reference
 * @Return: ----
 * ********************************************************************/
/*void SHAREDFUNCTIONS_parseDataFieldConnection(char *data, char *username, char *ip, int *port, pid_t *pid) {
	char *buffer = NULL;
	char *data_cpy = NULL; 

	data_cpy = (char *) malloc (sizeof(char) * strlen(data));

	// reserve memory for the username
	asprintf(&buffer, "%s", strtok(data_cpy, GPC_DATA_SEPARATOR_STR)); 
	username = (char *) malloc (sizeof(char) * strlen(buffer));
	strcpy(username, buffer);
	free(buffer);

	// reserve memory for the ip
	asprintf(&buffer, "%s", strtok(NULL, GPC_DATA_SEPARATOR_STR));
	ip = (char *) malloc (sizeof(char) * strlen(buffer));
	strcpy(ip, buffer);
	free(buffer);

	// reserve memory for the port
	*port = atoi(strtok(NULL, GPC_DATA_SEPARATOR_STR));

	// reserve memory for the pid
	*pid = (pid_t) atoi(strtok(NULL, GPC_DATA_SEPARATOR_STR));
}*/

/**********************************************************************
 * @Purpose: Writes the data field of a frame when a client connects or 
 * 			 updates the clients data
 * @Params: in: blist = the list of the clients connected to the server
 * @Return: data = the data field of the frame with all the clients 
 * 				   connected
 * ********************************************************************/
char * SHAREDFUNCTIONS_writeDataFieldUpdate(BidirectionalList blist) {
	char *data = NULL;
	char *buffer = NULL;
	int new_size = 0;
	int flag_first = 1;
	Element element;

	BIDIRECTIONALLIST_goToHead(&blist);

	while(BIDIRECTIONALLIST_isValid(blist)) {
		element = BIDIRECTIONALLIST_get(&blist);
		if(flag_first) {
			asprintf(&buffer, "%s&%s&%d&%d", element.username, element.ip_network, element.port, (int) element.pid);
			// reserve memory for the data field
			data = (char *) malloc (sizeof(char) * (strlen(buffer) + 1));
			data[strlen(buffer)] = '\0';
		} else{
			asprintf(&buffer, "#%s&%s&%d&%d", element.username, element.ip_network, element.port, (int) element.pid);
			new_size = strlen(data) + strlen(buffer) + 1;
			data = (char *) realloc (data, sizeof(char) * new_size);
			data[new_size] = '\0';
		}		
		
		strcat(data, buffer);
		BIDIRECTIONALLIST_next(&blist);
	}

	return data;
}

/**********************************************************************
 * @Purpose: Reads the data field of a frame when a client connects
 * 			 or updates the clients data
 * @Params: in/out: data = the data field of the frame
 * 			in/out: username = the matrix of usernames of the clients 
 * 							   passed by reference
 * 			in/out: ip = the matrix of ip addresses of the clients 
 * 						 passed by reference
 * 			in/out: port = the matrix of ports of the clients passed 
 * 						   by reference
 *  		in/out: pid = the matrix of pids of the clients passed 
 * 						  by reference
 * @Return: ----
 * ********************************************************************/
/*void SHAREDFUNCTIONS_readDataFieldUpdate(char *data, char **username, char **ip, int **port, int **pid) {
	int i = 0;
	char *buffer = NULL;
	int flag_first = 1;
	
	while(1) {
		if(flag_first) {
			asprintf(&buffer, "%s", strtok(data, GPC_DATA_SEPARATOR_USR_STR));
		} else {
			asprintf(&buffer, "%s", strtok(NULL, GPC_DATA_SEPARATOR_USR_STR));
		}

		// If strtok returns NULL, there are no more users
		if(buffer == NULL) {
			break;
		}

		SHAREDFUNCTIONS_parseDataFieldConnection(buffer, username[i], ip[i], port[i], pid[i]);
		free(buffer);

		i++;
	}	
}*/


