/*********************************************************************
* @Purpose: Module that contains different functions that are shared
*           among other modules.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 18/10/2022
* @Last change: 05/01/2023
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
		    if (-1 == n) {
			    return (NULL);
			}

		    if (NULL == buffer) {
			    buffer = (char *) malloc (sizeof(char));
			}

			buffer[i] = byte;
			i++;
			buffer = (char *) realloc (buffer, sizeof(char) * (i + 1));
		}
	}

	if (i == 0) {
		return (NULL);
	}

	if (NULL != buffer) {
	    buffer[i] = '\0';
	}

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
* @Purpose: Writes the data field of a frame when a client connects or 
*			updates the clients data
* @Params: in: blist = the list of the clients connected to the server
* @Return: data = the data field of the frame with all the clients 
* 				  connected
**********************************************************************/
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
* @Purpose: Get the MD5SUM of the given file.
* @Params: in: filename = name of the file to get the MD5SUM.
* @Return: Returns the MD5SUM of the file.
**********************************************************************/
char * SHAREDFUNCTIONS_getMD5Sum(char *filename) {
	char *md5sum = NULL;
	int fd_pipe[2];
	int status = 0;
	int pid = 0;

	// create pipe
	if (pipe(fd_pipe) == -1) {
		printMsg("ERROR: The Pipe to check the MD5SUM of the file could not be created\n");
		return (NULL);
	}

	pid = fork();
	
	if (0 == pid) {
		// parent process, executes md5sum linux command
		close(fd_pipe[0]);
		dup2(fd_pipe[1], 1);
		execlp("md5sum", "md5sum", filename, NULL);
		close(fd_pipe[1]);
		exit(1);
	} else if (pid > 0) {
		// child process, gets md5sum from parent
		close(fd_pipe[1]);
		md5sum = SHAREDFUNCTIONS_readUntil(fd_pipe[0], ' ');
		waitpid(pid, &status, 0);
		close(fd_pipe[0]);
	} else {
		// error creating fork
		printMsg("ERROR: The fork to check the MD5SUM of the file could not be created\n");
		return (NULL);
	}

	return (md5sum);
}
