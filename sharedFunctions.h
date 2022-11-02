#ifndef _SHAREDFUNCTIONS_H_
#define _SHAREDFUNCTIONS_H_

#define _GNU_SOURCE 1
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "definitions.h"

/* Messages */
#define ERROR_N_ARGS_MSG	     "ERROR: Not enough arguments\n"

/* Constants */
#define READ_FILE_OK             0
#define READ_FILE_KO             -1

#define printMsg(x) write(1, x, strlen(x))

/*********************************************************************
* @Purpose: Read a string from a file descriptor, stopping at a given
*           char.
* @Params: in: fd = file descriptor
*          in: delimiter = char marking where to stop reading
* @Return: Returns a string.
*********************************************************************/
char * SHAREDFUNCTIONS_readUntil(int fd, char delimiter);

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
char * SHAREDFUNCTIONS_splitString(char *string, char delimiter, int *pos);

/**********************************************************************
* @Purpose: Copies a string.
* @Params: in: source = string to copy
*          in/out: dest = string where to copy source
* @Return: ----
*********************************************************************/
void SHAREDFUNCTIONS_strCopy(char *source, char *dest);

/**********************************************************************
* @Purpose: Remove a char repeatedly from a string.
* @Params: in/out: string = string to modify
*          in: unwanted = char we want to remove
* @Return: ----
***********************************************************************/
char * SHAREDFUNCTIONS_removeChar(char *string, char unwanted);

/**********************************************************************
* @Purpose: Frees dynamic memory allocated for an IluvatarSon.
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing an 
*                                iluvatarSon
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_freeIluvatarSon(IluvatarSon *iluvatarSon);

#endif
