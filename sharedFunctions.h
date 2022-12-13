#ifndef _SHAREDFUNCTIONS_H_
#define _SHAREDFUNCTIONS_H_

#define _GNU_SOURCE 1
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "bidirectionallist.h" 

/* Constants */
#define READ_FILE_OK             0
#define READ_FILE_KO             -1

#define printMsg(x) write(1, x, strlen(x)) 

/*********************************************************************
* @Purpose: Read a string from a file descriptor, stopping at a given
*           char.
* @Params: in: fd = file descriptor
*          in: delimiter = char marking where to stop reading
* @Return: Returns a string.
*********************************************************************/
char * SHAREDFUNCTIONS_readUntil(int fd, char delimiter);

/*********************************************************************
* @Purpose: Splits a string given a char.
* @Params: in: string = original string to split
*          in: delimiter = char marking split point
*		   in/out: pos = int that indicates the position at which to
*		                 start parsing the string. After split is done
*						 it is updated so that consecutive splits of
*						 the same string can be done.
* @Return: Returns a string containin a part of the original string.
*********************************************************************/
char * SHAREDFUNCTIONS_splitString(char *string, char delimiter, int *pos);

/**********************************************************************
* @Purpose: Remove a char repeatedly from a string.
* @Params: in/out: string = string to modify
*          in: unwanted = char we want to remove
* @Return: ----
***********************************************************************/
char * SHAREDFUNCTIONS_removeChar(char *string, char unwanted);

/**********************************************************************
* @Purpose: Frees dynamic memory allocated for an IluvatarSon.
* @Params: in/out: iluvatarSon = IluvatarSon pointer referencing an 
*                                iluvatarSon
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_freeIluvatarSon(IluvatarSon *iluvatarSon);

/**********************************************************************
* @Purpose: Frees dynamic memory allocated for the server Arda.
* @Params: in/out: arda = Arda pointer referencing the server Arda      
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_freeArda(Arda *arda);

/**********************************************************************
* @Purpose: Reads the network frame that the client have sent.
* @Params: in: fd 	= file descriptor of the client
*          in/out: type = type of the frame passed by reference
*          in/out: header = header of the frame passed by reference
*          in/out: data = data of the frame passed by reference     
* @Return: ----
***********************************************************************/
void SHAREDFUNCTIONS_readFrame(int fd, int *type, char *header, char *data);

/**********************************************************************
 * @Purpose: Reads from a file descriptor until a given char is found.
 * @Params: in: fd = the file descriptor we want to read from 
 * 			in: type = the type of the frame
 * 			in/out: header = header of the frame passed by reference
 * 			in/out: data = data of the frame passed by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_writeFrame(int fd, int type, char *header, char *data);

/**********************************************************************
 * @Purpose: Parses the data field of a frame
 * @Params: in/out: data = the data field of the frame
 * 			in/out: username = the username of the client passed by reference
 * 			in/out: ip = the ip address of the client passed by reference
 * 			in/out: port = the port of the client passed by reference
 *  		in/out: pid = the pid of the client passed by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_parseDataFieldConnection(char *data, char *username, char *ip, int *port, pid_t *pid);

/**********************************************************************
 * @Purpose: Writes the data field of a frame when a client connects or 
 * 			 updates the clients data
 * @Params: in: blist = the list of the clients connected to the server
 * @Return: data = the data field of the frame with all the clients 
 * 				   connected
 * ********************************************************************/
char * SHAREDFUNCTIONS_writeDataFieldUpdate(BidirectionalList blist);

/**********************************************************************
 * @Purpose: Reads the data field of a frame when a client connects
 * 			 or updates the clients data
 * @Params: in/out: data = the data field of the frame
 * 			in/out: username = the matrix of usernames of the clients 
 * 							   passed by reference
 * 			in/out: ip = the matrix of ip addresses of the clients 
 * 						 passed by reference
 * 			in/out: port = the matrix of ports of the clients passed 
 * 						   by reference
 *  		in/out: pid = the matrix of pids of the clients passed 
 * 						  by reference
 * @Return: ----
 * ********************************************************************/
void SHAREDFUNCTIONS_readDataFieldUpdate(char *data, char **username, char **ip, int **port, int **pid);

#endif
