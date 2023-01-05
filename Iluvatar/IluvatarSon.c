/*********************************************************************
* @Purpose: Executes an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 05/01/2023
*********************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <mqueue.h>

#include "../bidirectionallist.h"
#include "../server.h"
#include "commands.h"
#include "../sharedFunctions.h"
#include "../gpc.h"
#include "../icp.h"
#include "../client.h"

#define MIN_N_ARGS 					2
#define WELCOME_MSG 				"\n%sWelcome %s, son of Iluvatar\n"
#define ARDA_CONNECTION_DENIED_MSG	"Connection denied by server\n"
#define ILUVATARSON_OK				0
#define ILUVATARSON_KO				-1

IluvatarSon iluvatarSon;
char *iluvatar_command = NULL;
BidirectionalList users_list;
Client client;
Server server;
mqd_t qfd;
pthread_t thread_accept;
pthread_mutex_t mutex_print = PTHREAD_MUTEX_INITIALIZER;

/*********************************************************************
* @Purpose: Free memory and close any open file descriptors before
* 		 	 terminating program.
* @Params: ----
* @Return: ----
*********************************************************************/
void disconnectionManager(){
	char *buffer = NULL;

	// close thread
	pthread_cancel(thread_accept);
	pthread_join(thread_accept, NULL);
	pthread_detach(thread_accept);
	// close message queue
	mq_close(qfd);
	asprintf(&buffer, "/%d", getpid());
	mq_unlink(buffer);
	// free memory
	free(buffer);
	buffer = NULL;
    SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
	
	if (NULL != iluvatar_command) {
	    free(iluvatar_command);
		iluvatar_command = NULL;
	}

	BIDIRECTIONALLIST_destroy(&users_list);
	// close Iluvatar server
	SERVER_close(&server);
	free(server.thread);
	close(client.server_fd);
	
	if (server.client_fd > 0) {
		close(server.client_fd);
	}

	// reset command line
	printMsg(COLOR_DEFAULT_TXT);
}

/*********************************************************************
* @Purpose: Free memory when SIGINT received.
* @Params: ----
* @Return: ----
*********************************************************************/
void sigintHandler() {
	// Writing the exit frame
	if(iluvatarSon.username != NULL) {
		GPC_writeFrame(client.server_fd, 0x06, GPC_EXIT_HEADER, iluvatarSon.username, strlen(iluvatarSon.username));
	}

	disconnectionManager();
	signal(SIGINT, SIG_DFL); 
	raise(SIGINT);
}

/*********************************************************************
* @Purpose: Creates a new empty IluvatarSon.
* @Params: ----
* @Return: Returns an initialized IluvatarSon.
*********************************************************************/
IluvatarSon newIluvatarSon() {
    IluvatarSon iluvatar;

	iluvatar.username = NULL;
	iluvatar.directory = NULL;
	iluvatar.ip_address = NULL;
	iluvatar.arda_ip_address = NULL;

	return (iluvatar);
}

/*********************************************************************
* @Purpose: Reads an IluvatarSon from a given file.
* @Params: in: filename = string with the name of the file
*          in/out: iluvatar = pointer to IluvatarSon referencing
*		                      the IluvatarSon to fill with data
* @Return: Returns ILUVATARSON_OK if no errors when reading the file,
*          otherwise ILUVATARSON_KO
*********************************************************************/
int readIluvatarSon(char *filename, IluvatarSon *iluvatar) {
    char *buffer = NULL;
	int error = ILUVATARSON_KO;
	int fd = open(filename, O_RDONLY);

	if (fd > 0) {
	    iluvatar->username = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // check name
		iluvatar->username = SHAREDFUNCTIONS_removeChar(iluvatar->username, GPC_DATA_SEPARATOR);
		// directory
		iluvatar->directory = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		// Arda IP
		iluvatar->arda_ip_address = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // Arda port
		buffer = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		iluvatar->arda_port = atoi(buffer);
		free(buffer);
		// Iluvatar IP
	    iluvatar->ip_address = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
	    // Iluvatar port
		buffer = SHAREDFUNCTIONS_readUntil(fd, END_OF_LINE);
		iluvatar->port = atoi(buffer);
		free(buffer);
		// no errors
		error = ILUVATARSON_OK;
		close(fd);
	}

	return (error);
}

/*********************************************************************
* @Purpose: Send connection frame to Arda server and gets the current
*           list of users.
* @Params: ----
* @Return: Returns 1 if an error occurred, otherwise 0.
*********************************************************************/
char connectToArda() {
	char *buffer = NULL;
	char *header = NULL;
	char type = 0x07;

	// notify connection to Arda
	asprintf(&buffer, "%s%c%s%c%d%c%d", iluvatarSon.username, GPC_DATA_SEPARATOR,
	                                    iluvatarSon.ip_address, GPC_DATA_SEPARATOR,
										iluvatarSon.port, GPC_DATA_SEPARATOR, getpid());
	GPC_writeFrame(client.server_fd, 0x01, GPC_CONNECT_SON_HEADER, buffer, strlen(buffer));
	free(buffer);
	buffer = NULL;
	// wait for answer
	GPC_readFrame(client.server_fd, &type, &header, &buffer);

	// check connection
	if (0 == strcmp(header, GPC_HEADER_CONKO)) {
	    printMsg(COLOR_RED_TXT);
		printMsg(ARDA_CONNECTION_DENIED_MSG);
		printMsg(COLOR_DEFAULT_TXT);

		// Writing the exit frame
		if(iluvatarSon.username != NULL) {
			GPC_writeFrame(client.server_fd, 0x06, GPC_EXIT_HEADER, iluvatarSon.username, strlen(iluvatarSon.username));
		}

		// free mem
		free(buffer);
		buffer = NULL;
		free(header);
		header = NULL;
		SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
		BIDIRECTIONALLIST_destroy(&users_list);
		close(client.server_fd);
		return (1);
	}

	free(header);
	header = NULL;
	// update list of users
	GPC_updateUsersList(&users_list, buffer);
	free(buffer);
	buffer = NULL;
	return (0);
}

/*********************************************************************
* @Purpose: Manages the connections with other IluvatarSons.
* @Params: ----
* @Return: Returns NULL
*********************************************************************/
void *iluvatarAccept() {
	// Running the server
	SERVER_runIluvatar(&iluvatarSon, &server, &mutex_print);
	return NULL;
}

/*********************************************************************
* @Purpose: Opens the command line for the user.
* @Params: ----
* @Return: ----
*********************************************************************/
void openCLI() {
	char *buffer = NULL;

	asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
	pthread_mutex_lock(&mutex_print);
	printMsg(buffer);
	pthread_mutex_unlock(&mutex_print);
	free(buffer);
	buffer = NULL;
}

/*********************************************************************
* @Purpose: Opens command line for user and executes the entered
*           commands.
* @Params: ----
* @Return: Returns 1 when EXIT command is entered, otherwise 0.
*********************************************************************/
int manageUserPrompt() {
	int is_exit = 0;

	// get command
	iluvatar_command = SHAREDFUNCTIONS_readUntil(STDIN_FILENO, CMD_END_BYTE);
	pthread_mutex_lock(&mutex_print);
	printMsg(COLOR_DEFAULT_TXT);
	pthread_mutex_unlock(&mutex_print);
	
	// execute command
	if (NULL != iluvatar_command) {
		is_exit = COMMANDS_executeCommand(iluvatar_command, &iluvatarSon, client.server_fd, &users_list);	//TODO: passar-li el mutex i protegir tots els STDIN de commands.c
		free(iluvatar_command);
		iluvatar_command = NULL;
	}
	
	// reopen command line
	openCLI();

	return (is_exit);
}

/*********************************************************************
* @Purpose: Executes the IluvatarSon process.
* @Params: in: argc = number of arguments entered
*          in: argv = array of arguments entered
* @Return: Returns 0.
*********************************************************************/
int main(int argc, char* argv[]) {
	char *message_receive = NULL;
	char *mq_type = NULL;
	char *origin_user = NULL;
	char *msg = NULL;
	struct mq_attr attr;
    char *buffer = NULL;
	int exit_program = 0, read_ok = ILUVATARSON_KO;
	char *header = NULL;
	char type = 0x07;
	fd_set read_fds;

	iluvatarSon = newIluvatarSon();
	// Configure SIGINT
	signal(SIGINT, sigintHandler);

	// check args
	if (MIN_N_ARGS != argc) {
	    // error
		printMsg(COLOR_RED_TXT);

		if (MIN_N_ARGS > argc) {
		    printMsg(ERROR_N_LESS_ARGS_MSG);
		} else {
		    printMsg(ERROR_N_MORE_ARGS_MSG);
		}

		printMsg(COLOR_DEFAULT_TXT);
	} else {
		// read input file
		read_ok = readIluvatarSon(argv[1], &iluvatarSon);

		if (ILUVATARSON_KO == read_ok) {
		    // print error
			printMsg(COLOR_RED_TXT);
			asprintf(&buffer, ERROR_OPENING_FILE, argv[1]);
			printMsg(buffer);
			free(buffer);
			buffer = NULL;
			printMsg(COLOR_DEFAULT_TXT);
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			return (-1);
		}

		users_list = BIDIRECTIONALLIST_create();
		// Open active socket (connection with Arda)
		client = CLIENT_init(iluvatarSon.arda_ip_address, iluvatarSon.arda_port);

		if (FD_NOT_FOUND == client.server_fd) {
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			BIDIRECTIONALLIST_destroy(&users_list);
			return (-1);
		}

		if (0 != connectToArda()) {
			return (-1);
		}
		
		// Open passive socket (prepare Iluvatar server)
		server = SERVER_init(iluvatarSon.ip_address, iluvatarSon.port);

		if ((FD_NOT_FOUND == server.listen_fd) || (LIST_NO_ERROR != server.clients.error)) {
	    	SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			close(client.server_fd);
			BIDIRECTIONALLIST_destroy(&users_list);
			return (-1);
		}

		// Create thread to accept connections
		if (pthread_create(&thread_accept, NULL, iluvatarAccept, NULL) != 0) {
			printMsg(COLOR_RED_TXT);
			printMsg(ERROR_CREATING_THREAD_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			// free memory
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			free(server.thread);
			server.thread = NULL;
			// close FDs
			close(server.listen_fd);
			closeAllClientFDs(&server);
			BIDIRECTIONALLIST_destroy(&server.clients);
			return (-1);
		}

		// From here, iluvatarSon has more than one thread, so we need to protect the STDIN
		// Create queue
		asprintf(&buffer, "/%d", getpid());
		qfd = mq_open(buffer, O_RDWR | O_CREAT | O_EXCL, 0600, NULL);
		free(buffer);
		buffer = NULL;

		if (qfd == (mqd_t) -1) {  
			pthread_mutex_lock(&mutex_print);
			printMsg(COLOR_RED_TXT);
			printMsg(ERROR_CREATING_MQ_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(&mutex_print);
			// finish execution and disconnect from Arda
			raise(SIGINT);
		}

		// get attributes of the queue
		mq_getattr(qfd, &attr);

		// welcome user
		asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, iluvatarSon.username);
		pthread_mutex_lock(&mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(&mutex_print);
		free(buffer);
		buffer = NULL;

		// open command line
		openCLI();
		FD_ZERO (&read_fds);
		
		// get commands
		while (!exit_program) {
			// add FDs to set
			FD_SET(STDIN_FILENO, &read_fds);
			FD_SET(client.server_fd, &read_fds);
			FD_SET(qfd, &read_fds);

			// wait for input
			if (select(MAX_FD_SET_SIZE, &read_fds, NULL, NULL, NULL) < 0) {
				pthread_mutex_lock(&mutex_print);
				printMsg(COLOR_RED_TXT);
				printMsg(ERROR_SELECT_MSG);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(&mutex_print);
				exit_program = 1;
			} else if (FD_ISSET(client.server_fd, &read_fds)) {				
				// If the read frame returns 0, it means that the connection has been closed
				if (GPC_readFrame(client.server_fd, &type, &header, &buffer) == 0) {
					pthread_mutex_lock(&mutex_print);
					printMsg(COLOR_RED_TXT);
					printMsg(ARDA_CONNECTION_CLOSED_MSG);
					printMsg(COLOR_DEFAULT_TXT);
					pthread_mutex_unlock(&mutex_print);
					exit_program = 1;
				} else if ((0 == strcmp(header, GPC_UPDATE_USERS_HEADER_OUT)) && (0x02 == type)){
					// Manage UPDATE USERS
					BIDIRECTIONALLIST_destroy(&users_list);
					users_list = COMMANDS_getListFromString(buffer, (int) strlen(buffer));
					free(buffer);
					buffer = NULL;
				} else if ((0 == strcmp(header, GPC_HEADER_CONOK)) && (0x06 == type)) {
					// Manage EXIT
					pthread_mutex_lock(&mutex_print);
					printMsg(COLOR_DEFAULT_TXT);
					printMsg(EXIT_ARDA_MSG);
					pthread_mutex_unlock(&mutex_print);
					exit_program = 1;
				} else {
					pthread_mutex_lock(&mutex_print);
					printMsg(COLOR_RED_TXT);
					printMsg(ERROR_DISCONNECT_ILUVATAR_MSG);
					printMsg(COLOR_DEFAULT_TXT);
					pthread_mutex_unlock(&mutex_print);
				}
				
			} else if (FD_ISSET(STDIN_FILENO, &read_fds)) {
				// reads and executes the command, then prepares the prompt for next command
				exit_program = manageUserPrompt();
			} else if (FD_ISSET(qfd, &read_fds)) {
				pthread_mutex_lock(&mutex_print);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(&mutex_print);

				message_receive = (char *) malloc((attr.mq_msgsize + 1) * sizeof(char));
				if (mq_receive(qfd, message_receive, attr.mq_msgsize, NULL) == -1) {
					pthread_mutex_lock(&mutex_print);
					printMsg(COLOR_RED_TXT);
					printMsg(ERROR_RECEIVING_MSG_MSG);
					printMsg(COLOR_DEFAULT_TXT);
					pthread_mutex_unlock(&mutex_print);
					exit_program = 1; 
				} else {
					buffer = strdup(message_receive);
					mq_type = strtok(buffer, "&");

					if (strcmp(mq_type, "msg") == 0) {
						// show received message
						ICP_receiveMsg(message_receive, &mutex_print);
					} else if (strcmp(mq_type, "file") == 0) {
						// save received file
						exit_program = ICP_receiveFile(&message_receive, iluvatarSon.directory, &attr, qfd, &mutex_print);
					} else {
						pthread_mutex_lock(&mutex_print);
						printMsg(COLOR_RED_TXT);
						printMsg("ERROR: Unknown message type\n");
						printMsg(COLOR_DEFAULT_TXT);	
						pthread_mutex_unlock(&mutex_print);			
					}

					// reopen command line
					openCLI();
				}
			}

			if (header != NULL) {
				free(header);
				header = NULL;
			}
			
			if (buffer != NULL) {
				free(buffer);
				buffer = NULL;
			}

			if (origin_user != NULL) {
				free(origin_user);
				origin_user = NULL;
			}
			
			if (msg != NULL) {
				free(msg);
				msg = NULL;
			}

			if (message_receive != NULL) {
				free(message_receive);
				message_receive = NULL;
			}
		}	
	}

	raise(SIGINT);
	return (0);
}
