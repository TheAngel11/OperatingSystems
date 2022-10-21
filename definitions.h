#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

#include <string.h>
#include <stdlib.h>

// CONSTANTS
#define COLOR_DEFAULT_TXT	"\033[0m"
#define COLOR_CLI_TXT		"\033[1;36m"
#define COLOR_RED_TXT		"\033[1;31m"

typedef struct {
    char *username;
	char *directory;
    char *arda_ip_address;
    int arda_port;
	char *ip_address;
	int port;
} IluvatarSon;

typedef struct {
    char *ip_address;
	int port;
} Arda;

#endif