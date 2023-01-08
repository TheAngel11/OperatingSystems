/*********************************************************************
* @Purpose: Executes an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 08/01/2023
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
#include "../semaphore_v2.h"

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
semaphore sem_mq;			// synchronization semaphore to wait for qfd answers when frame sent
//semaphore sem_fd_mq;		// mutual exclusion semaphore to prevent accessing qfd at same time
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

	// destroy semaphore
	if (1 <= BIDIRECTIONALLIST_getNumberOfElements(users_list)) {
	    SEM_destructor(&sem_mq);
//		SEM_destructor(&sem_fd_mq);
	}
	
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
	if (iluvatarSon.username != NULL) {
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
* @Purpose: Initializes semaphores if only 1 user.
* @Params: ----
* @Return: ----
*********************************************************************/
void initSemaphores() {
    // check number of users
	if (1 <= BIDIRECTIONALLIST_getNumberOfElements(users_list)) {
	    SEM_init(&sem_mq, 0);
//		SEM_init(&sem_fd_mq, 1);
	}
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
		if (iluvatarSon.username != NULL) {
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
		is_exit = COMMANDS_executeCommand(iluvatar_command, &iluvatarSon, client.server_fd, &users_list, &sem_mq, &mutex_print);
		free(iluvatar_command);
		iluvatar_command = NULL;
	}
	
	// reopen command line
	openCLI();

	return (is_exit);
}

/*********************************************************************
* @Purpose: Gets an ICP frame sent between IluvatarSons in the same
*           machine.
* @Params: in/out: attr = attributes of the message queue
* @Return: Returns 0 if no errors occurred, otherwise 1.
*********************************************************************/
char getLocalFrame(struct mq_attr *attr) {
	char *frame = NULL;
	char *type = NULL;
	char *buffer = NULL;
	int n = 0;

	// reset command line
	pthread_mutex_lock(&mutex_print);
	printMsg(COLOR_DEFAULT_TXT);
	pthread_mutex_unlock(&mutex_print);
	// get ICP frame
	frame = (char *) malloc((attr->mq_msgsize + 1) * sizeof(char));
	n = mq_receive(qfd, frame, attr->mq_msgsize, NULL);
		
	if (-1 == n) {
		pthread_mutex_lock(&mutex_print);
		printMsg(COLOR_RED_TXT);
		printMsg(ERROR_RECEIVING_MSG_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(&mutex_print);
		// free memory
		free(frame);
		frame = NULL;
		return (1);
	} else {
		buffer = strdup(frame);
		type = strtok(buffer, "&");

		if (strcmp(type, "msg") == 0) {
		    // show received message
			ICP_receiveMsg(frame, &mutex_print);
		} else if (strcmp(type, "file") == 0) {
			// save received file
			if (ICP_READ_FRAME_ERROR == ICP_receiveFile(&frame, iluvatarSon.directory, attr, qfd, &sem_mq, &mutex_print)) {
			    // free memory
				if (NULL != frame) {
				    free(frame);
				    frame = NULL;
				}
				
				if (NULL != buffer) {
				    free(buffer);
				    buffer = NULL;
				}

				// reopen command line
				openCLI();
				return (1);
			}
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

	// free memory
	if (NULL != frame) {
	    free(frame);
		frame = NULL;
	}

	if (NULL != buffer) {
	    free(buffer);
		buffer = NULL;
	}

	return (0);
}

/*********************************************************************
* @Purpose: Executes the IluvatarSon process.
* @Params: in: argc = number of arguments entered
*          in: argv = array of arguments entered
* @Return: Returns 0.
*********************************************************************/
int main(int argc, char* argv[]) {
	struct mq_attr attr;
    char *buffer = NULL;
	int exit_program = 0, read_ok = ILUVATARSON_KO;
	char *header = NULL;
	fd_set read_fds;

	iluvatarSon = newIluvatarSon();
	// Configure SIGINT
	signal(SIGINT, sigintHandler);
	// Init synchronization semaphore for message queue
	SEM_constructor_with_name(&sem_mq, ftok("IluvatarSon.c", 'a'));
//	SEM_constructor_with_name(&sem_fd_mq, ftok("IluvatarSon.c", 'b'));

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

		initSemaphores();
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
				// Arda server replied to sent frame
				exit_program = CLIENT_manageArdaServerAnswer(&client, &users_list, &mutex_print);
			} else if (FD_ISSET(STDIN_FILENO, &read_fds)) {
				// reads and executes the command, then prepares the prompt for next command
				exit_program = manageUserPrompt();
			} else {
			    // mutual exclusion		//TODO: probably unnecessary, debugging purposes
//				SEM_wait(&sem_fd_mq);

				if (FD_ISSET(qfd, &read_fds)) {
				    // received message or file from another Iluvatar in same machine
					exit_program = getLocalFrame(&attr);
				}

//				SEM_signal(&sem_fd_mq); //TODO: debugging purposes
			}

			if (header != NULL) {
				free(header);
				header = NULL;
			}
			
			if (buffer != NULL) {
				free(buffer);
				buffer = NULL;
			}
		}	
	}

	raise(SIGINT);
	return (0);
}
