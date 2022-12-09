/*********************************************************************
* @Purpose: Module that contains different functions that are shared
*           among other modules.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 18/10/2022
* @Last change: 08/12/2022
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
* @Params: in: fd = the file descriptor we want to read from 
*          in/out: type = type of the frame passed by reference
*          in/out: header = header of the frame passed by reference
* @Return: Returns the data if the frame.
***********************************************************************/
char SHAREDFUNCTIONS_readFrame(int fd, char *type, char *header, char **data) {
	char *buffer = NULL;
	char byte = 0x07;
	unsigned short length = 0;
	//char *data = NULL;

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
	header = (char *) malloc (sizeof(char) * (strlen(buffer) + 1));
	strcpy(header, buffer);
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
* @Purpose: Reads from a file descriptor until a given char is found.
* @Params: in: fd = the file descriptor we want to read from
* 			in: type = the type of the frame
* 			in/out: header = header of the frame passed by reference
* 			in/out: data = data of the frame passed by reference
* @Return: ----
**********************************************************************/
char SHAREDFUNCTIONS_writeFrame(int fd, char type, char *header, char *data) {
	char *frame = NULL;
	char byte = 0;
	unsigned short length = 0;
	char length_lsB = 0, length_msB = 0;
	int i = 0, j = 0;
	int size = 0;

	if (NULL != data) {
	    length = strlen(data);
	}

	size = 1 + ((int) strlen(header)) + 2 + 2 + length + 1;
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

	frame[i] = '\0';
	// write entire frame
	write(fd, frame, size);
	free(frame);
	frame = NULL;

	return (1);
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
void SHAREDFUNCTIONS_parseDataFieldConnection(char *data, char *username, char *ip, int *port, pid_t *pid) {
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
}

/**********************************************************************
 * @Purpose: Writes the data field of a frame when a client connects or 
 * 			 updates the clients data
 * @Params: in: blist = the list of the clients connected to the server
 * @Return: data = the data field of the frame with all the clients 
 * 				   connected
 * ********************************************************************/
char * SHAREDFUNCTIONS_getUsersFromList(BidirectionalList blist) {
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
			// reserve memory for the data field
			//data = (char *) malloc (sizeof(char) * (size + 1));
			//data[size] = '\0';
			flag_first = 0;
		} else{
			n = asprintf(&buffer, "#%s&%s&%d&%d", element.username, element.ip_network, element.port, (int) element.pid);
			size += n + 1;
			data = (char *) realloc (data, sizeof(char) * size);
			strcat(data, buffer);
		}		
		
		BIDIRECTIONALLIST_next(&blist);
	}

	return (data);
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
void SHAREDFUNCTIONS_readDataFieldUpdate(char *data, char **username, char **ip, int **port, int **pid) {
	int i = 0;
	char *buffer = NULL;
	int flag_first = 1;
	
	while(1) {
		if(flag_first) {
			asprintf(&buffer, "%s", strtok(data, GPC_USERS_SEPARATOR_STR));
		} else {
			asprintf(&buffer, "%s", strtok(NULL, GPC_USERS_SEPARATOR_STR));
		}

		// If strtok returns NULL, there are no more users
		if(buffer == NULL) {
			break;
		}

		SHAREDFUNCTIONS_parseDataFieldConnection(buffer, username[i], ip[i], port[i], pid[i]);
		free(buffer);

		i++;
	}	
}
