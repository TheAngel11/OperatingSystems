/*********************************************************************
* @Purpose: Manages communication between processes in same machine
*           and defines the Internal Communication Protocol (ICP).
* @Authors: Claudia Lajara Silvosa
*           Angel Garcia Gascon
* @Date: 04/01/2023
* @Last change: 04/01/2023
*********************************************************************/
#include "icp.h"

/**********************************************************************
* @Purpose: Given a message SEND MSG from one IluvatarSon to another IluvatarSon
* 			 that are in the same machine, finds the origin user and the message.
* @Params: in/out: message = the message that the origin user sends
* 		    in/out: origin_user = the user who sends the message
* 		    in/out: msg = the message that the origin user sends
* @Note: The message is in the format: msg&origin_user&message
**********************************************************************/
void parseCreateNeighborMessageMsg(char *message, char **origin_user, char **msg) {
	int i = 0;	
	char *aux = NULL;

	//message is in the format:  msg + originUser + GPC_DATA_SEPARATOR + message
	// first we get rid of the "msg" part
	aux = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	free(aux);
	*origin_user = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);
	*msg = SHAREDFUNCTIONS_splitString(message, ICP_DATA_SEPARATOR, &i);

}

void ICP_sendMsg(char *message_receive, pthread_mutex_t *mutex) {
	char *msg = NULL;
	char *origin_user = NULL;
	char *buffer = NULL;

	parseCreateNeighborMessageMsg(message_receive, &origin_user, &msg);
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
