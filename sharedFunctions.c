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
* @Params: in: fd = the file descriptor we want to read from 
*          in/out: type = type of the frame passed by reference
*          in/out: header = header of the frame passed by reference
* @Return: Returns the data if the frame.
***********************************************************************/
char * SHAREDFUNCTIONS_readFrame(int fd, char *type, char *header) {
	char *buffer = NULL;
	char byte;
	int length = 0;
	char *data = NULL;

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
	read(fd, &byte, sizeof(char));			// MSB
	length = (int) ((byte << 8) & 0xFF00);
	read(fd, &byte, sizeof(char));			// LSB
	length += (int) (byte & 0x00FF);

	// read data (lenght bytes)
	data = (char *) malloc (sizeof(char) * (length + 1));
	read(fd, buffer, sizeof(char) * length);
	data[length] = '\0';

	return (data);
}

/**********************************************************************
 * @Purpose: Reads from a file descriptor until a given char is found.
 * @Params: in: fd = the file descriptor we want to read from 
 * 			in: type = the type of the frame
 * 			in/out: header = header of the frame passed by reference
 * 			in/out: data = data of the frame passed by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_writeFrame(int fd, char type, char *header, char *data) {
	char *buffer = NULL;
	char *frame = NULL;
	char byte = 0;
	unsigned int length = 0;
	char length_lsB = 0, length_msB = 0;

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
	
	asprintf(&frame, "%c", byte);
	// write header
	asprintf(&buffer, "[%s]", header);	
	strcat(frame, buffer);
	free(buffer);
	buffer = NULL;

	// write lenght (2 bytes)
	if(data != NULL) {
		length = strlen(data);
		length_msB = (char) ((length >> 8) & 0x00FF); 	// shift MSB
		length_lsB = (char) (length & 0x00FF);			// store LSB
		asprintf(&buffer, "%c%c", length_msB, length_lsB);
		strcat(frame, buffer);
		free(buffer);
		buffer = NULL;
		// write data (lenght bytes)
		strcat(frame, data);
	} else {
		length_msB = (char) ((length >> 8) & 0x00FF); 	// shift MSB
		length_lsB = (char) (length & 0x00FF);			// store LSB
		asprintf(&buffer, "%c%c", length_msB, length_lsB);
		strcat(frame, buffer);
		free(buffer);
		buffer = NULL;
	}

	// write entire frame
	write(fd, frame, strlen(frame));
	free(frame);
	frame = NULL;
}

/**********************************************************************
 * @Purpose: Parses the data field of a frame
 * @Params: in/out: data = the data field of the frame
 * 			in/out: username = the username of the client passed by reference
 * 			in/out: ip = the ip address of the client passed by reference
 * 			in/out: port = the port of the client passed by reference
 *  		in/out: pid = the pid of the client passed by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_parseDataFieldConnection(char *data, char *username, char *ip, int *port, pid_t *pid) {
	username = strtok(data, GPC_DATA_SEPARATOR_STR);
	ip = strtok(NULL, GPC_DATA_SEPARATOR_STR);
	*port = atoi(strtok(NULL, GPC_DATA_SEPARATOR_STR));
	*pid = atoi(strtok(NULL, GPC_DATA_SEPARATOR_STR));	
}
