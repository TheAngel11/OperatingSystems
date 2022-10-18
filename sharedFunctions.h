#ifndef _SHAREDFUNCTIONS_H_
#define _SHAREDFUNCTIONS_H_

#define _GNU_SOURCE
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "definitions.h"

// CONSTANTS
#define ERROR_N_ARGS_MSG	     "%sERROR: Not enough arguments\n"
#define ERROR_OPEN_FILE_MSG	     "%sERROR: File could not be opened\n"
#define ERROR_PARSING_LINE_MSG	 "%sERROR: Line of file could not be parsed\n"

#define END_OF_LINE		         '\n'
#define READ_FILE_OK             0
#define READ_FILE_KO             -1

void strCopy(char *source, char *dest);

void readInputFile(char *filename, IluvatarSon *iluvatarSon);

#endif