//
#include "client.h"

Client CLIENT_init(char *ip, int port) {
    Client c;
	struct sockaddr_in server;

	c.server_fd = FD_NOT_FOUND;

	// config socket
	if ((c.server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_CREATING_SOCKET_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		return (c);
	}

	bzero(&server, sizeof(server)); 
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (inet_pton(AF_INET, ip, &server.sin_addr) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_IP_CONFIGURATION_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		// finish execution
		close(c.server_fd);
		c.server_fd = FD_NOT_FOUND;
		return (c);
	}

	// connect to server
	if (connect(c.server_fd, (struct sockaddr*) &server, sizeof(server)) < 0) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ERROR_SERVER_CONNECTION_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		// finish execution
		close(c.server_fd);
		c.server_fd = FD_NOT_FOUND;
		return (c);
	}

	return (c);
}
