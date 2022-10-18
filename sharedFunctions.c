#include "sharedFunctions.h"

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

/*********************************************************************
* @Purpose: Parses a line of the file introduced by the user
* @Params: line = The line of the file we want to parse
* @Params: buffer = the char pointer with the content of the line
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: OK if the content of the line had been saved and KO if not
*********************************************************************/
int parseLine(int line, char *buffer, IluvatarSon *iluvatarSon) {
    switch (line) {
        case 1:
            iluvatarSon->username = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            if (NULL != iluvatarSon->username)
                strCopy(buffer, iluvatarSon->username);
            else
                return READ_FILE_KO;
            break;

        case 2:
            iluvatarSon->directory = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            if (NULL != iluvatarSon->directory)
                strCopy(buffer, iluvatarSon->directory);
            else
                return READ_FILE_KO;
            break;

        case 3:
            iluvatarSon->arda_ip_address = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            if (NULL != iluvatarSon->arda_ip_address)
                strCopy(buffer, iluvatarSon->arda_ip_address);
            else
                return READ_FILE_KO;
            break;

        case 4:
            iluvatarSon->arda_port = atoi(buffer);
            break;

        case 5:
            iluvatarSon->ip_address = (char *) malloc(sizeof(char) * (strlen(buffer) + 1));
            if (NULL != iluvatarSon->ip_address)
                strCopy(buffer, iluvatarSon->ip_address);
            else
                return READ_FILE_KO;
            break;

        case 6:
            iluvatarSon->port = atoi(buffer);
            break;

        default:
            return READ_FILE_KO;
    }

    return READ_FILE_OK;
}

/*********************************************************************
* @Purpose: Reads the file .dat that the user introduced
* @Params: filename = char pointer with the filename that the user introduced
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing a iluvatarSon
* @Params: in/out: arda = Arda pointer referencing a iluvatarSon
* @Return: ----
*********************************************************************/
void readInputFile(char *filename, IluvatarSon *iluvatarSon) {
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
            buffer = (char *) malloc(sizeof(char));
            while(END_OF_LINE != byte) {
                // Check EOF
                if (0 == n) {
                    break;
                }

                /*if (buffer == NULL) {
                    buffer = (char *) malloc(sizeof(char));
                } else {
                    buffer = (char *) realloc(buffer, sizeof(char) * (i + 1));
                }*/

                if (buffer != NULL) {
                    buffer[i] = byte;
                    i++;
                    // Read next byte
                    n = read(fd, &byte, 1);
                    buffer = (char *) realloc(buffer, sizeof(char) * (i + 1));
                } else {
                    break;
                }
            }

            // parse string line
            buffer[i] = '\0';
            int parse_result = parseLine(line, buffer, iluvatarSon);

            if (parse_result == READ_FILE_KO) {
                n = asprintf(&writing_buff, ERROR_PARSING_LINE_MSG, COLOR_RED_TXT);
                write (STDOUT_FILENO, writing_buff, n);
                // reset buffer
                free(writing_buff);
				writing_buff = NULL;
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
		writing_buff = NULL;
    }
}