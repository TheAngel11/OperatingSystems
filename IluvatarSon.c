#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "definitions.h"
#include "commands.h"

/* From definitions.h
#define COLOR_DEFAULT_TXT	"\033[0m"
#define COLOR_CLI_TXT		"\033[1;0m"
#define COLOR_RED_TXT		"\033[1;31m"
*/

#define ERROR_N_ARGS_MSG	"%sERROR: Not enough arguments\n"
#define ERROR_OPEN_FILE_MSG	"%sERROR: File could not be opened\n"
#define ERROR_PARSING_LINE_MSG	"%sERROR: Line of file could not be parsed\n"


#define END_OF_LINE		    '\n'
#define OK                  0
#define KO                  -1
#define MIN_N_ARGS			2
#define WELCOME_MSG			"%sWelcome %s, son of Iluvatar\n"
#define CMD_LINE_PROMPT		"%s%c "

// TYPEDEFS

// GLOBAL VARS

// FUNCTIONS
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
* @Purpose: Frees all the dynamic memory.
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: ----
***********************************************************************/
void freeAllMem(IluvatarSon *iluvatarSon, Arda *arda) {
    free(iluvatarSon->username);
    free(iluvatarSon->directory);
    free(iluvatarSon->ip_address);
    free(arda->ip_address);
}

/*********************************************************************
* @Purpose: Parses a line of the file introduced by the user
* @Params: line = The line of the file we want to parse
* @Params: buffer = the char pointer with the content of the line
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: OK if the content of the line had been saved and KO if not
*********************************************************************/
int parseLine(int line, char *buffer, IluvatarSon *iluvatarSon, Arda *arda, int i) {
    switch (line) {
        case 1:
            iluvatarSon->username = (char *) malloc(sizeof(char) * i);
            if (NULL != iluvatarSon->username)
                strCopy(buffer, iluvatarSon->username);
            else
                return KO;
            break;

        case 2:
            iluvatarSon->directory = (char *) malloc(sizeof(char) * i);
            if (NULL != iluvatarSon->directory)
                strCopy(buffer, iluvatarSon->directory);
            else
                return KO;
            break;

        case 3:
            arda->ip_address = (char *) malloc(sizeof(char) * i);
            if (NULL != arda->ip_address)
                strCopy(buffer, arda->ip_address);
            else
                return KO;
            break;

        case 4:
            arda->port = atoi(buffer);
            break;

        case 5:
            iluvatarSon->ip_address = (char *) malloc(sizeof(char) * i);
            if (NULL != iluvatarSon->ip_address)
                strCopy(buffer, iluvatarSon->ip_address);
            else
                return KO;
            break;

        case 6:
            iluvatarSon->port = atoi(buffer);
            break;

        default:
            return KO;
    }

    return OK;
}

/*********************************************************************
* @Purpose: Reads the file .dat that the user introduced
* @Params: filename = char pointer with the filename that the user introduced
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: ----
*********************************************************************/
void readInputFile(char *filename, IluvatarSon *iluvatarSon, Arda *arda) {
    char *buffer = NULL;
    char *writing_buff = NULL;
    int i = 0, line = 1;
    int n = 0;
    char byte;
    int fd = open(filename, O_RDONLY);

    if(fd > 0) {
        // Read until EOF
        n = read(fd, &byte, 1);
        while(0 != n) {
            // Read line
            while(END_OF_LINE != byte) {
                // Check EOF
                if (0 == n) {
                    break;
                }

                if (buffer == NULL) {
                    buffer = (char *) malloc(sizeof(char));
                } else {
                    buffer = (char *) realloc(buffer, sizeof(char) * (i + 1));
                }

                if (buffer != NULL) {
                    buffer[i] = byte;
                    i++;
                    // Read next byte
                    n = read(fd, &byte, 1);
                } else {
                    break;
                }
            }

            // parse string line
            int parse_result = parseLine(line, buffer, iluvatarSon, arda, i);

            if (parse_result == KO) {
                n = asprintf(&writing_buff, ERROR_PARSING_LINE_MSG, COLOR_RED_TXT);
                write (STDOUT_FILENO, writing_buff, n);
                // reset buffer
                free(writing_buff);
            }

            // Reading next line
            n = read(fd, &byte, 1);
            // Initializing next line
            line++;
            i = 0;
            free(buffer);
            buffer = NULL;
        }

        close(fd);
    } else {
        n = asprintf(&writing_buff, ERROR_OPEN_FILE_MSG, COLOR_RED_TXT);
        write (STDOUT_FILENO, writing_buff, n);
        // reset buffer
        free(writing_buff);
    }
}

/*********************************************************************
* @Purpose: Executes the IluvatarSon process.
* @Params: in: argc = number of arguments entered
*          in: argv = array of arguments entered
* @Return: Returns 0.
*********************************************************************/
int main(int argc, char* argv[]) {
    char *buffer = NULL;
	char *command = NULL;
	int n = 0;
    IluvatarSon iluvatarSon;
    Arda arda;

	// check args
	if (MIN_N_ARGS > argc) {
	    // error
		n = asprintf(&buffer, ERROR_N_ARGS_MSG, COLOR_RED_TXT);
		write(STDOUT_FILENO, buffer, n);
		// reset buffer
		free(buffer);
	} else {
	    // read input file
		readInputFile(argv[1], &iluvatarSon, &arda);
		// welcome user
		n = asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, "Galadriel");
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// open command line
		n = asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		write(STDOUT_FILENO, buffer, n);
		free(buffer);
		// get command
		command = readCommand();
		write(STDOUT_FILENO, COLOR_DEFAULT_TXT, strlen(COLOR_DEFAULT_TXT));
		// execute command

		// free mem
		if (NULL != command) {
		    free(command);
		}
	}

    /*//Debugging Purpose
    printf("USERNAME: %s\n", iluvatarSon.username);
    printf("DIRECTORY: %s\n", iluvatarSon.directory);
    printf("ARDA IP: %s\n", arda.ip_address);
    printf("ARDA PORT: %d\n", arda.port);
    printf("ILU IP: %s\n", iluvatarSon.ip_address);
    printf("ILU PORT: %d\n", iluvatarSon.port);*/

    freeAllMem(&iluvatarSon, &arda);

	return (0);
}