#ifndef _GPC_H_
#define _GPC_H_

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sharedFunctions.h"
#include "bidirectionallist.h"

#define GPC_DATA_SEPARATOR				'&'
#define GPC_USERS_SEPARATOR				'#'
#define GPC_CONNECT_SON_HEADER			"NEW_SON\0"
#define GPC_UPDATE_USERS_HEADER_IN		"LIST_REQUEST\0"
#define GPC_UPDATE_USERS_HEADER_OUT		"LIST_RESPONSE\0"
#define GPC_HEADER_CONOK            	"CONOK\0"
#define GPC_HEADER_CONKO            	"CONKO\0"
#define GPC_EXIT_HEADER					"EXIT\0"
#define GPC_UNKNOWN_CMD_HEADER      	"UNKNOWN\0"

/**********************************************************************
* @Purpose: Reads a frame sent through the network.
* @Params: in: fd = file descriptor to read from.
*          in: type = type of frame received.
*          in/out: header = header to get from frame.
*          in/out: data = data to get from frame.
* @Return: Returns 1.
***********************************************************************/
char GPC_readFrame(int fd, char *type, char **header, char **data);

/**********************************************************************
* @Purpose: Write a frame to the given file descriptor.
* @Params: in: fd = file descriptor to write.
* 		   in: type = type of frame to send.
* 		   in: header = header of frame to send.
* 		   in: data = data to send.
* @Return: Returns 1.
**********************************************************************/
char GPC_writeFrame(int fd, char type, char *header, char *data);

/**********************************************************************
* @Purpose: Given the data of a frame containing the attributes of a
*           user, parses the user and stores it in list element.
* @Params: in: data = data containing the name, IP, port and PID of the
*              user.
* 		   in/out: e = instance of Element to store user.
* @Return: Returns 1.
**********************************************************************/
void GPC_parseUserFromFrame(char *data, Element *e);

/**********************************************************************
* @Purpose: Given the data of a frame containing the users, updates
*           list of users with the given ones.
* @Params: in/out: list = list of users.
* 		   in: users = data of the frame with the users' attributes.
* @Return: Returns 1.
**********************************************************************/
char GPC_updateUsersList(BidirectionalList *list, char *users);


/**********************************************************************
* @Purpose: Turns a list of users into a string following the GPC
*           format so that it can be sent in a frame.
* @Params: in: blist = list of the clients connected to the server.
* @Return: Returns a string containing the data in GPC format.
**********************************************************************/
char * GPC_getUsersFromList(BidirectionalList blist);

#endif
