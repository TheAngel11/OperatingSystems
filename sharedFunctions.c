#include "sharedFunctions.h"

/*********************************************************************
* @Purpose: Read a string from a file descriptor, stopping at a given
*           char.
* @Params: in: fd = file descriptor
*          in: delimiter = char marking where to stop reading
* @Return: Returns a string.
*********************************************************************/
char * readUntil(int fd, char delimiter) {
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

/**********************************************************************
* @Purpose: Copies a string.
* @Params: in: source = string to copy
*          in/out: dest = string where to copy source
* @Return: ----
***********************************************************************/
void strCopy(char *source, char *dest) {
    int i = 0;

    for (i = 0; i < (int) strlen(source); i++) {
        dest[i] = source[i];
    }

    dest[i] = '\0';
}

/**********************************************************************
* @Purpose: Remove a char repeatedly from a string.
* @Params: in/out: string = string to modify
*          in: unwanted = char we want to remove
* @Return: ----
***********************************************************************/
char * removeChar(char *string, char unwanted) {
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
void freeIluvatarSon(IluvatarSon *iluvatarSon) {
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
