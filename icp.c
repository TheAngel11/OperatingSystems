/*********************************************************************
* @Purpose: Manages communication between processes in same machine
*           and defines the Internal Communication Protocol (ICP).
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 04/01/2023
* @Last change: 08/01/2023
*********************************************************************/
#include "icp.h"

/*********************************************************************
* @Purpose: Sends a message to a user using message queues.
* @Params: in: pid = PID of the user that will receive the message
* 		   in: message = string containing the message to send
* 		   in: origin_username = string with the name of the sender
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if message was sent successfully, otherwise 1.
*********************************************************************/
char ICP_sendMsg(int pid, char *message, char *origin_username, pthread_mutex_t *mutex) {
	mqd_t qfd;
	char *buffer = NULL;

	// open queue
	asprintf(&buffer, "/%d", pid);
	qfd = mq_open(buffer, O_RDWR);
	free(buffer);
	buffer = NULL;
			
	// check message not empty
	if (strlen(message) == 2) {
	    // invalid message
		mq_close(qfd);
		return (1);
	}
			
	// prepare data for frame
	asprintf(&buffer, "msg%c%s%c%s", ICP_DATA_SEPARATOR, origin_username, ICP_DATA_SEPARATOR, message);

	// send message
	if (mq_send(qfd, buffer, strlen(buffer) + 1, 0) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_MSG_MQ_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		if (NULL != buffer) {
	        free(buffer);
			buffer = NULL;
		}

		mq_close(qfd);
		return (1);
	} else {
		pthread_mutex_lock(mutex);
		printMsg(SEND_MSG_OK_MSG); 
		pthread_mutex_unlock(mutex);
	}
					
	// free memory
	if (NULL != buffer) {
	    free(buffer);
		buffer = NULL;
	}

	mq_close(qfd);
	return (0);
}

/*********************************************************************
* @Purpose: Sends a file to a user using message queues.
* @Params: in/out: qfd = file descriptor of the message queue
* 		   in/out: path = path of the file to send
* 		   in: filename = name of the file to send
* 		   in/out: fd_file = file descriptor of the file to send
* 		   in: username = string containing the name of the sender
* 		   in: file_size = size of the file to send
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if the file was sent successfully, otherwise 1.
*********************************************************************/
char sendFileFrames(mqd_t *qfd, char **path, char *filename, int *fd_file, char *username, int file_size, pthread_mutex_t *mutex) {
	char *md5sum = NULL;
	char *buffer = NULL;

	// Get the MD5SUM
	md5sum = SHAREDFUNCTIONS_getMD5Sum(*path);
	// Prepare the message to send
	asprintf(&buffer, "file%c%s%c%s%c%d%c%s", ICP_DATA_SEPARATOR, username, ICP_DATA_SEPARATOR, filename, ICP_DATA_SEPARATOR, file_size, ICP_DATA_SEPARATOR, md5sum);
	free(*path);
	*path = NULL;
	free(md5sum);
	md5sum = NULL;
	
	// Send the file info message
	if (mq_send(*qfd, buffer, strlen(buffer) + 1, 0) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_FILE_MQ_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(buffer);
		buffer = NULL;
		// close queue
		mq_close(*qfd);
		close(*fd_file);
		return (1);
	}

	free(buffer);
	buffer = NULL;

	// Send the file in fragments if bigger than ICP_FILE_MAX_BYTES
	while (file_size > ICP_FILE_MAX_BYTES) {
		buffer = (char *) malloc(sizeof(char) * ICP_FILE_MAX_BYTES);
		read(*fd_file, buffer, ICP_FILE_MAX_BYTES);
		
		if (mq_send(*qfd, buffer, ICP_FILE_MAX_BYTES, 0) == -1) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg(SEND_FILE_MQ_ERROR);
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			free(buffer);
			buffer = NULL;
			// close queue
			mq_close(*qfd);
			close(*fd_file);
			return (1);
		}
		
		free(buffer);
		buffer = NULL;
		file_size -= ICP_FILE_MAX_BYTES;
	}
	
	// Send the last part of the file
	buffer = (char *) malloc(sizeof(char) * file_size);
	read(*fd_file, buffer, file_size);

	if (mq_send(*qfd, buffer, file_size, 0) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_FILE_MQ_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		free(buffer);
		buffer = NULL;
		// close queue
		mq_close(*qfd);
		close(*fd_file);
		return (1);
	}

	free(buffer);
	buffer = NULL;
	close(*fd_file);
	return (0);
}

/*********************************************************************
* @Purpose: Sends a file to a user using message queues.
* @Params: in: pid = PID of the user that will receive the file
* 		   in: filename = name of the file to send
* 		   in: directory = string with the directory of the file
* 		   in: username = string containing the name of the sender
*          in/out: mutex = screen mutex to prevent writing to screen
*                  simultaneously
* @Return: Returns 0 if the file was sent successfully, otherwise 1.
*********************************************************************/
char ICP_sendFile(int pid, char *filename, char *directory, char *username, pthread_mutex_t *mutex) {
	char *filename_path = NULL;
	int file_size = 0;
	int fd_file = FD_NOT_FOUND;
	mqd_t qfd;
	semaphore sem_queue;
	struct mq_attr attr;
	char *buffer = NULL;

	// open queue
	asprintf(&buffer, "/%d", pid);
	qfd = mq_open(buffer, O_RDWR);
	free(buffer);
	buffer = NULL;
	// create semaphore
	SEM_constructor_with_name(&sem_queue, pid);
	// open the file to send
	asprintf(&filename_path, ".%s/%s", directory, filename);
	fd_file = open(filename_path, O_RDONLY);

	// check file FD
	if (fd_file == FD_NOT_FOUND) {
		asprintf(&buffer, SEND_FILE_OPEN_FILE_ERROR, filename);
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(buffer);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(buffer);
		buffer = NULL;
		free(filename_path);
		filename_path = NULL;
		// close queue
		mq_close(qfd);
		return (1);
	}

	// get file size
	file_size = (int) lseek(fd_file, 0, SEEK_END);
	lseek(fd_file, 0, SEEK_SET);
	
	if (file_size == 0) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(SEND_FILE_EMPTY_FILE_ERROR);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		// free memory
		free(filename_path);
		filename_path = NULL;
		// close queue
		mq_close(qfd);
		return (1);
	}

	if (0 != sendFileFrames(&qfd, &filename_path, filename, &fd_file, username, file_size, mutex)) {
		// close queue
		mq_close(qfd);
	    return (1);
	}
	
	// get the attributes of the queue
	if (mq_getattr(qfd, &attr) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg(MQ_ATTR_ERROR_MSG);
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		perror("mq_getattr");
		// close queue
		mq_close(qfd);
		return (1);
	}
	
	// wait for queue to be ready
	SEM_wait(&sem_queue);
	// Receive the answer
	buffer = (char *) malloc((attr.mq_msgsize + 1) * sizeof(char));
	
	if (mq_receive(qfd, buffer, attr.mq_msgsize, NULL) == -1) {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The message could not be received\n");
		printMsg(COLOR_DEFAULT_TXT);
		perror("woefdhaspfdc");
		pthread_mutex_unlock(mutex);
		free(buffer);
		buffer = NULL;
		// close queue
		mq_close(qfd);
		// unblock receiver process
		SEM_signal(&sem_queue);
		return (1);
	}

	if (0 == strcmp(buffer, FILE_OK_REPLY)) {
		pthread_mutex_lock(mutex);
		printMsg("File correctly sent\n");
		pthread_mutex_unlock(mutex);
	} else {
		pthread_mutex_lock(mutex);
		printMsg(COLOR_RED_TXT);
		printMsg("ERROR: The file sent has lost its integrity\n");
		printMsg(COLOR_DEFAULT_TXT);
		pthread_mutex_unlock(mutex);
		free(buffer);
		buffer = NULL;
		// close queue
		mq_close(qfd);
		// unblock receiver process
		SEM_signal(&sem_queue);
		return (1);
	}

	free(buffer);
	buffer = NULL;

	// close queue
	mq_close(qfd);
	// unblock receiver process
	SEM_signal(&sem_queue);
	return (0);
}

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
	asprintf(&buffer, ICP_MSG_RECEIVED_MSG, origin_user, msg);
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
* @Purpose: Parses the frame containing the initial data of a received
*           file by a user in the same machine following the ICP.
* @Params: in: frame = ICP frame with the initial data
* 		   in/out: origin_user = string to store the name of the sender
* 		   in/out: filename = string to store the name of the file
* 		   in/out: file_size = total size of the file in bytes
* 		   in/out: md5sum = string to store the checksum of the file
**********************************************************************/
void parseInitialSendFileFrame(char *frame, char **origin_user, char **filename, int *file_size, char **md5sum) {
	int i = 0;	
	char *aux = NULL;

	// first we get rid of the "file" part
	aux = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	free(aux);
	aux = NULL;
	// get information about user and file
	*origin_user = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	*filename = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	aux = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
	*file_size = atoi(aux);
	free(aux);
	aux = NULL;
	*md5sum = SHAREDFUNCTIONS_splitString(frame, ICP_DATA_SEPARATOR, &i);
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
	*frame = (char *) malloc((msg_size + 1) * sizeof(char));

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
		// copy file
		if (*frame != NULL) {
			write(file_fd, *frame, file_size);
		}
	}

	free(*frame);
	*frame = NULL;
	return (ICP_READ_FRAME_NO_ERROR);
}

/**********************************************************************
* @Purpose: Checks that the MD5SUM of the copied file and the received
*           one match.
* @Params: in/out: path = string containing the path of the copied file
*          in/out: filename = string containing the name of the
*		           received file
*		   in/out: md5sum = string with the MD5SUM of the received file
*		   in/out: user = string containing the name of the sender
*		   in/out mutex = screen mutex to prevent writing on screen
*		          simultaneously
* @Return: Returns FILE_MD5SUM_OK if the checksums match, otherwise
*          FILE_MD5SUM_KO.
**********************************************************************/
char checkMD5Sum(char **path, char **filename, char **md5sum, char **user, pthread_mutex_t *mutex) {
	char *buffer = NULL;

	// get current file MD5SUM
	buffer = SHAREDFUNCTIONS_getMD5Sum(*path);
	free(*path);
	*path = NULL;

	// compare with original MD5SUM
	if (strcmp(buffer, *md5sum) == 0) {
		free(buffer);
		buffer = NULL;
		asprintf(&buffer, ICP_FILE_RECEIVED_MSG, *user, *filename);
		pthread_mutex_lock(mutex);
		printMsg(buffer);
		pthread_mutex_unlock(mutex);
		// free memory
		free(buffer);
		buffer = NULL;
		free(*filename);
		*filename = NULL;
		free(*user);
		*user = NULL;
		free(*md5sum);
		*md5sum = NULL;
		return (FILE_MD5SUM_OK);
	}

	// mismatch in MD5SUM
	// free memory
	free(buffer);
	buffer = NULL;
	free(*filename);
	*filename = NULL;
	free(*user);
	*user = NULL;
	free(*md5sum);
	*md5sum = NULL;
	return (FILE_MD5SUM_KO);
}

/**********************************************************************
* @Purpose: Gets the file sent by another user in the same machine and
*           checks it. If there are no errors, copies the file into the
*           given directory.
* @Params: in/out: frame = string containing the initial data to get a
*                  file. This data is formed by the user sending the
*				   file, the name of the file, the total size of the
*				   file and the checksum of the file
*          in: directory = string containing the name of the directory
*		       in which to copy the file
*		   in/out: attr = attributes of the message queue
*		   in: qfd = file descriptor of the message queue
*		   in/out mutex = screen mutex to prevent writing on screen
*		          simultaneously
* @Return: Returns ICP_READ_FRAME_NO_ERROR if the file was received
*          correctly, otherwise ICP_READ_FRAME_ERROR.
**********************************************************************/
char ICP_receiveFile(char **frame, char *directory, struct mq_attr *attr, int qfd, int pid, pthread_mutex_t *mutex) {
	char *origin_user = NULL;
	char *filename = NULL;
	char *filename_path = NULL;
	char *md5sum = NULL;
	semaphore sem_queue;
	int file_size = 0;
	int file_fd = -1;

	// create semaphore
	SEM_constructor_with_name(&sem_queue, pid);
	// get file frames
	parseInitialSendFileFrame(*frame, &origin_user, &filename, &file_size, &md5sum);
	free(*frame);
	*frame = NULL;
	// open file
	asprintf(&filename_path, ".%s/%s", directory, filename);
	file_fd = open(filename_path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	
	while (file_size > ICP_FILE_MAX_BYTES) {
		// Read frame
		if (ICP_READ_FRAME_ERROR == readFileFrame(file_fd, qfd, frame, attr->mq_msgsize, ICP_FILE_MAX_BYTES, mutex)) {
			// signal that queue is ready
			SEM_signal(&sem_queue);
		    return (ICP_READ_FRAME_ERROR);
		}

		file_size -= ICP_FILE_MAX_BYTES;
	}

	// Read the last frame
	if (ICP_READ_FRAME_ERROR == readFileFrame(file_fd, qfd, frame, attr->mq_msgsize, file_size, mutex)) {
		// signal that queue is ready
		SEM_signal(&sem_queue);
	    return (ICP_READ_FRAME_ERROR);
	}
	
	close(file_fd);
	
	// check md5sum
	if (FILE_MD5SUM_OK == checkMD5Sum(&filename_path, &filename, &md5sum, &origin_user, mutex)) {
		// send reply of success
		if (mq_send(qfd, FILE_OK_REPLY, strlen(FILE_OK_REPLY) + 1,  0) == -1) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg("ERROR: The message could not be sent\n");
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			// signal that queue is ready
			SEM_signal(&sem_queue);
			// wait for sender to receive reply
			SEM_wait(&sem_queue);
			return (ICP_READ_FRAME_ERROR);
		}
	    
		// signal that queue is ready
		SEM_signal(&sem_queue);
	} else {
		// send reply of failure
		if (mq_send(qfd, FILE_KO_REPLY, strlen(FILE_KO_REPLY) + 1,  0) == -1) {
		    pthread_mutex_lock(mutex);
			printMsg(COLOR_RED_TXT);
			printMsg("ERROR: The message could not be sent\n");
			printMsg(COLOR_DEFAULT_TXT);
			pthread_mutex_unlock(mutex);
			// signal that queue is ready
			SEM_signal(&sem_queue);
			// wait for sender to receive reply
			SEM_wait(&sem_queue);
			return (ICP_READ_FRAME_ERROR);
		}

	    // signal that queue is ready
		SEM_signal(&sem_queue);
	}

	// wait for sender to receive reply
	SEM_wait(&sem_queue);
	return (ICP_READ_FRAME_NO_ERROR);
}
