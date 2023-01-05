/*********************************************************************
* @Purpose: Manages communication between processes in same machine
*           and defines the Internal Communication Protocol (ICP).
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 04/01/2023
* @Last change: 05/01/2023
*********************************************************************/
#include "icp.h"

/**********************************************************************
* @Purpose: Receives a message from another process in the same machine
*           and prints it.
* @Params: in: frame = ICP frame containing the message and the name
*              of the sender
* 		   in/out: mutex = screen mutex to prevent printing to screen
*                  by different users at the same time
* @Return: ----
**********************************************************************/
void ICP_receiveMsg(char *frame, pthread_mutex_t *mutex) {
	char *msg = NULL;
	char *origin_user = NULL;
	char *buffer = NULL;
	int i = 0;

	// parse frame
	buffer = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	free(buffer);
	buffer = NULL;
	origin_user = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	msg = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	// show information
	asprintf(&buffer, MSG_NEIGHBOURS_RECEIVED_MSG, origin_user, msg);
	pthread_mutex_lock(mutex);
	printMsg(buffer);
	pthread_mutex_unlock(mutex);
	// free memory
	free(buffer);
	buffer = NULL;
	free(msg);
	msg = NULL;
	free(origin_user);
	origin_user = NULL;
}

/**********************************************************************
* @Purpose: Given a message SEND FILE from one IluvatarSon to another IluvatarSon
* 			 that are in the same machine, finds the origin user, the filename, the file size and the md5sum.
* @Params: in/out: message = the message that the origin user sends
* 		    in/out: origin_user = the user who sends the message
* 		    in/out: filename = the name of the file that the origin user sends
* 		    in/out: file_size = the size of the file that the origin user sends
* 		    in/out: md5sum = the md5sum of the file that the origin user sends
**********************************************************************/
void parseCreateNeighborMessageFileInfo(char *message, char **origin_user, char **filename, int *file_size, char **md5sum) {
	int i = 0;	
	char *aux = NULL;

	// first we get rid of the "file" part
	aux = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	free(aux);
	aux = NULL;
	// get information about user and file
	*origin_user = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	*filename = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	aux = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	*file_size = atoi(aux);
	free(aux);
	aux = NULL;
	*md5sum = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
}

/**********************************************************************
* @Purpose: Read a frame containing a file or part of a file and copy
*           the contents into another file.
* @Params: in: file_fd = file descriptor of the file to write
*          in: qfd = file descriptor of the message queue from which
*		       to read frames
*		   in/out: frame = string to store frame containing the file
*		           or part of it
*		   in: msg_size = size of Message in message queue
*		   in: file_size = number of bytes to read from the file
*		   in/out: mutex = screen mutex to prevent writing to screen
*		           simultaneously
* @Return: Returns the MD5SUM of the file.
**********************************************************************/
char readFileFrame(int file_fd, int qfd, char **frame, int msg_size, int file_size, pthread_mutex_t *mutex) {
	*frame = (char *) malloc((msg_size + 1) * sizeof(char));	//TODO: S'hauria de canviar a la mida justa, però si li envio file_size + 1 peta el valgrind

	if (NULL == *frame) {
	    return (ICP_READ_FRAME_ERROR);
	}
	
	if (mq_receive(qfd, *frame, msg_size, NULL) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(ERROR_RECEIVING_MSG_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(*frame);
		*frame = NULL;
		return (ICP_READ_FRAME_ERROR);
	} else {
		if (*frame != NULL) {
			write(file_fd, *frame, file_size);
		}
	}

	free(*frame);
	*frame = NULL;
	return (ICP_READ_FRAME_NO_ERROR);
}

char ICP_receiveFile(char **frame, char *directory, struct mq_attr *attr, int qfd, pthread_mutex_t *mutex) {
	char *buffer = NULL;
	char *origin_user = NULL;
	char *filename = NULL;
	char *filename_path = NULL;
	char *md5sum = NULL;
	int file_size = 0;
	int file_fd = -1;

	parseCreateNeighborMessageFileInfo(*frame, &origin_user, &filename, &file_size, &md5sum);
	free(*frame);
	*frame = NULL;
	asprintf(&filename_path, ".%s/%s", directory, filename);
	file_fd = open(filename_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	
	while (file_size > ICP_FILE_MAX_BYTES) {
		// Read the message
		if (ICP_READ_FRAME_ERROR == readFileFrame(file_fd, qfd, frame, attr->mq_msgsize, ICP_FILE_MAX_BYTES, mutex)) {
		    return (1);
		}

		file_size -= ICP_FILE_MAX_BYTES;
	}

	// Read the last message
	if (ICP_READ_FRAME_ERROR == readFileFrame(file_fd, qfd, frame, attr->mq_msgsize, file_size, mutex)) {
	    return (1);
	}
	
	close(file_fd);
	// check md5sum
	buffer = SHAREDFUNCTIONS_getMD5Sum(filename_path);
	free(filename_path);
	filename_path = NULL;

	if (strcmp(buffer, md5sum) == 0) {
		// md5sum_verification = "FILE_OK";
		free(buffer);
		buffer = NULL;
		asprintf(&buffer, FILE_NEIGHBOURS_RECIEVED_MSG, origin_user, filename);
		pthread_mutex_lock(mutex);
		printMsg(buffer);
		pthread_mutex_unlock(mutex);
		free(buffer);
		buffer = NULL;
	}

	// free memory
	free(filename);
	filename = NULL;
	free(origin_user);
	origin_user = NULL;
	free(md5sum);
	md5sum = NULL;

	//Sending a message to the origin user saying the md5sum verification result
	//TODO: s'ha de crear un semàfor de sincronització per a que aquest missatge el rebi el que ha enviat el fitxer a commands.c, i no aquest, que l'ha rebut (ja que si descomento això i la rebuda a commands.c, el select d'aquí rep el missatge que he enviat aquí i no s'envia a qui li vull enviar)
	/*if(mq_send(qfd, md5sum_verification, strlen(md5sum_verification) + 1,  0) == -1) {
		pthread_mutex_lock(&mutex_print);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The message could not be sent\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(&mutex_print);
		exit_program = 1; 
	}*/

	return (0);
}
