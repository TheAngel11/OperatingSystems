/*********************************************************************
* @Purpose: Executes an IluvatarSon.
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 07/10/2022
* @Last change: 11/12/2022
*********************************************************************/
#define _GNU_SOURCE 1
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "../bidirectionallist.h"
#include "../server.h"
#include "commands.h"
#include "../sharedFunctions.h"
#include "../gpc.h"
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
 * *******************************************************************/
void disconnectionManager(){
	char *buffer = NULL;

	pthread_cancel(thread_accept);
	pthread_join(thread_accept, NULL);
	pthread_detach(thread_accept);
	mq_close(qfd);
	asprintf(&buffer, "/%d", getpid());
	mq_unlink(buffer);
	free(buffer);
    SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
	if (NULL != iluvatar_command) {
	    free(iluvatar_command);
	}
	BIDIRECTIONALLIST_destroy(&users_list);

	SERVER_close(&server);
	free(server.thread);
	close(client.server_fd);
	if(server.client_fd > 0) {
		close(server.client_fd);
	}

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
 * @Purpose: Manages the connections with other IluvatarSons.
 * @Params: in: iluvatar = pointer to IluvatarSon referencing the
 * 					  IluvatarSon to manage
 * @Return: Returns NULL
 *********************************************************************/
void *iluvatarAccept() {
	// Running the server
	SERVER_runIluvatar(&iluvatarSon, &server, &mutex_print);
	return NULL;
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
	char *filename = NULL;
	char *filename_path = NULL;
	int file_size = 0;
	char *md5sum = NULL;
	int file_fd = -1;
	//char *md5sum_verification = "FILE_KO";			// TODO: es estàtic (maybe hauria de ser reservant memòria)
	struct mq_attr attr;
    char *buffer = NULL;
	char *command = iluvatar_command;
	int exit_program = 0, read_ok = ILUVATARSON_KO;
	char *header = NULL;
	char type = 0x07;
	fd_set read_fds;

	iluvatarSon = newIluvatarSon();
	users_list = BIDIRECTIONALLIST_create();
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
			printMsg(COLOR_DEFAULT_TXT);
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			return (0);
		}

		// Open active socket
		client = CLIENT_init(iluvatarSon.arda_ip_address, iluvatarSon.arda_port);

		if (FD_NOT_FOUND == client.server_fd) {
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			BIDIRECTIONALLIST_destroy(&users_list);
			return (-1);
		}

		// notify connection to Arda
		asprintf(&buffer, "%s%c%s%c%d%c%d", iluvatarSon.username, GPC_DATA_SEPARATOR, iluvatarSon.ip_address, GPC_DATA_SEPARATOR, iluvatarSon.port, GPC_DATA_SEPARATOR, getpid());
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
		
		// Open passive socket
		server = SERVER_init(iluvatarSon.ip_address, iluvatarSon.port);

		if ((FD_NOT_FOUND == server.listen_fd) || (LIST_NO_ERROR != server.clients.error)) {
	    	SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);
			close(client.server_fd);
			BIDIRECTIONALLIST_destroy(&users_list);
			return (-1);
		}

		// Creating the thread to accept connections
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
		// Up to here, iluvatarSon has different more than one thread, so we need to protect the STDIN

		free(header);
		header = NULL;
		// update list of users
		GPC_updateUsersList(&users_list, buffer);
		free(buffer);
		buffer = NULL;

		// Creating the queue
		asprintf(&buffer, "/%d", getpid());
		qfd = mq_open(buffer, O_RDWR | O_CREAT | O_EXCL, 0600, NULL);
		free(buffer);
		buffer = NULL;

		if (qfd == (mqd_t) -1) {  
			pthread_mutex_lock(&mutex_print);		//TODO: Potser s'hauria de cridar a un raise(SIGINT) o disconnectManager() per estalviar linies
			printMsg(COLOR_RED_TXT);
			printMsg(ERROR_CREATING_MQ_MSG);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(&mutex_print);
			// free memory
			SHAREDFUNCTIONS_freeIluvatarSon(&iluvatarSon);			
			free(server.thread);	
			server.thread = NULL;
			// close FDs
			close(server.listen_fd);
			closeAllClientFDs(&server);
			BIDIRECTIONALLIST_destroy(&server.clients);
			pthread_cancel(thread_accept);
			pthread_join(thread_accept, NULL);
			pthread_detach(thread_accept);
			return (-1);
		}
		// we get the attributes of the queue
		mq_getattr(qfd, &attr);

		// welcome user
		asprintf(&buffer, WELCOME_MSG, COLOR_DEFAULT_TXT, iluvatarSon.username);
		pthread_mutex_lock(&mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(&mutex_print);
		free(buffer);
		buffer = NULL;

		// open command line
		asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
		pthread_mutex_lock(&mutex_print);
		printMsg(buffer);
		pthread_mutex_unlock(&mutex_print);
		free(buffer);
		buffer = NULL;
		
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
				if(GPC_readFrame(client.server_fd, &type, &header, &buffer) == 0) {
					pthread_mutex_lock(&mutex_print);
					printMsg(COLOR_RED_TXT);
					printMsg(ARDA_CONNECTION_CLOSED_MSG);
					printMsg(COLOR_DEFAULT_TXT);
					pthread_mutex_unlock(&mutex_print);
					exit_program = 1;
				} else if (strcmp(header, GPC_UPDATE_USERS_HEADER_OUT) == 0 && type == 0x02){
					// Manage UPDATE USERS
					BIDIRECTIONALLIST_destroy(&users_list);
					users_list = COMMANDS_getListFromString(buffer, (int) strlen(buffer));
					free(buffer);
					buffer = NULL;
				} else if(strcmp(header, GPC_HEADER_CONOK) == 0 && type == 0x06) {
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
				// get command
				command = SHAREDFUNCTIONS_readUntil(STDIN_FILENO, CMD_END_BYTE);
				pthread_mutex_lock(&mutex_print);
				printMsg(COLOR_DEFAULT_TXT);
				pthread_mutex_unlock(&mutex_print);
				// execute command
				if(NULL != command) {
					exit_program = COMMANDS_executeCommand(command, &iluvatarSon, client.server_fd, &users_list);	//TODO: passar-li el mutex i protegir tots els STDIN de commands.c
					free(command);
					command = NULL;
				}
				// reopen command line
				asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
				pthread_mutex_lock(&mutex_print);
				printMsg(buffer);
				pthread_mutex_unlock(&mutex_print);
				free(buffer);
				buffer = NULL;
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

					if(strcmp(mq_type, "msg") == 0) {
						// It is a send message message
						GPC_parseCreateNeighborMessageMsg(message_receive, &origin_user, &msg);

						asprintf(&buffer, MSG_NEIGHBOURS_RECIEVED_MSG, origin_user, msg);
						pthread_mutex_lock(&mutex_print);
						printMsg(buffer);
						pthread_mutex_unlock(&mutex_print);
						free(buffer);
						buffer = NULL;

					} else if(strcmp(mq_type, "file") == 0) {
						// It is a send file message
						//md5sum_verification = "FILE_KO";

						GPC_parseCreateNeighborMessageFileInfo(message_receive, &origin_user, &filename, &file_size, &md5sum);
						free(message_receive);
						message_receive = NULL;

						asprintf(&filename_path, ".%s/%s", iluvatarSon.directory, filename);
						
						file_fd = open(filename_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);		//TODO: revisar si està bé posar aquests permisos (a l'igual que al send file de diferents maquines)
						while(file_size > GPC_FILE_MAX_BYTES && !exit_program) {
							// Read the message
							message_receive = (char *) malloc((attr.mq_msgsize + 1) * sizeof(char));	//TODO: S'hauria de canviar a la mida justa, però si li GPC_FILE_MAX_BYTES file_size + 1 peta el valgrind
							if (mq_receive(qfd, message_receive, attr.mq_msgsize, NULL) == -1) {
								pthread_mutex_lock(&mutex_print);
								printMsg(COLOR_RED_TXT);
								printMsg(ERROR_RECEIVING_MSG_MSG);
								printMsg(COLOR_DEFAULT_TXT);
								pthread_mutex_unlock(&mutex_print);
								perror("mq_receive");
								exit_program = 1;
							} else {
								if(message_receive != NULL) {
									write(file_fd, message_receive, GPC_FILE_MAX_BYTES);
								}
							}
							
							free(message_receive);
							message_receive = NULL;
							file_size -= GPC_FILE_MAX_BYTES;
						}

						// Read the last message
						message_receive = (char *) malloc((attr.mq_msgsize + 1) * sizeof(char));	//TODO: S'hauria de canviar a la mida justa, però si li envio file_size + 1 peta el valgrind

						if (mq_receive(qfd, message_receive, attr.mq_msgsize, NULL) == -1) {
							pthread_mutex_lock(&mutex_print);
							printMsg(COLOR_RED_TXT);
							printMsg(ERROR_RECEIVING_MSG_MSG);
							printMsg(COLOR_DEFAULT_TXT);
							pthread_mutex_unlock(&mutex_print);
							exit_program = 1; 
						} else {
							if(message_receive != NULL) {
								write(file_fd, message_receive, file_size);
							}
						}

						free(message_receive);
						message_receive = NULL;
						close(file_fd);

						// We check the md5sum
						buffer = GPC_getMD5Sum(filename_path);
						free(filename_path);

						if(strcmp(buffer, md5sum) == 0) {
						//	md5sum_verification = "FILE_OK";
							free(buffer);
							asprintf(&buffer, FILE_NEIGHBOURS_RECIEVED_MSG, origin_user, filename);
							pthread_mutex_lock(&mutex_print);
							printMsg(buffer);
							pthread_mutex_unlock(&mutex_print);
							free(buffer);
							buffer = NULL;						
						} 

						//Sending a message to the origin user saying the md5sum verification result			//TODO: s'ha de crear un semàfor de sincronització per a que aquest missatge el rebi el que ha enviat el fitxer a commands.c, i no aquest, que l'ha rebut (ja que si descomento això i la rebuda a commands.c, el select d'aquí rep el missatge que he enviat aquí i no s'envia a qui li vull enviar)
						/*if(mq_send(qfd, md5sum_verification, strlen(md5sum_verification) + 1,  0) == -1) {
							pthread_mutex_lock(&mutex_print);
							printMsg(COLOR_RED_TXT);
							printMsg("ERROR: The message could not be sent\n");
							printMsg(COLOR_DEFAULT_TXT);
							pthread_mutex_unlock(&mutex_print);
							exit_program = 1; 
						}*/

						free(message_receive);
						message_receive = NULL;

					} else {
						pthread_mutex_lock(&mutex_print);
						printMsg(COLOR_RED_TXT);
						printMsg("ERROR: Unknown message type\n");
						printMsg(COLOR_DEFAULT_TXT);	
						pthread_mutex_unlock(&mutex_print);			
					}

					// reopen command line
					asprintf(&buffer, CMD_LINE_PROMPT, COLOR_CLI_TXT, CMD_ID_BYTE);
					pthread_mutex_lock(&mutex_print);
					printMsg(buffer);
					pthread_mutex_unlock(&mutex_print);
					free(buffer);
					buffer = NULL;
				}
			}

			if(header != NULL) {
				free(header);
				header = NULL;
			}
			if(buffer != NULL) {
				free(buffer);
				buffer = NULL;
			}
			if(origin_user != NULL) {
				free(origin_user);
				origin_user = NULL;
			}
			if(msg != NULL) {
				free(msg);
				msg = NULL;
			}
			if(message_receive != NULL) {
				free(message_receive);
				message_receive = NULL;
			}
			if(mq_type != NULL) {
				free(mq_type);
				mq_type = NULL;
			}
		}	
	}

	raise(SIGINT);
	return (0);
}
