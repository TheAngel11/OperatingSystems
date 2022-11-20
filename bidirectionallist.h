/**************************************************************************** 
 * 
 * @Objective: A bidirectional list (or Doubly Linked List) is a linear data
 *              structure, which does not store the elements at contiguous
 *              memory locations. The bidirectional list is similar to the
 *              linked list in terms of storing the elements but adds another
 *              pointer to each node to let them know both which is the next
 *              element (stored in a Node) and which is the previous element.
 * @Author/s:  Eduard de Torres (eduard.detorres@salle.url.edu)
 *			   David Vernet (david.vernet@salle.url.edu)	
 *			   Alberto Soto (alberto.soto@salle.url.edu)
 *			   Daniel Amo (daniel.amo@salle.url.edu)
 *			   Eduard Fernandez (eduard.fernandez@salle.url.edu)
 * @Creation date: 20/07/2018
 * @Last modification date: 07/05/2019
 * 
 ****************************************************************************/

// Define guard to prevent compilation problems if we add the module more
//  than once in the project.
#ifndef _BIDIRECTIONALLIST_H_
#define _BIDIRECTIONALLIST_H_

// Libraries
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#include "definitions.h"

// Constants to manage the list's error codes.
#define LIST_NO_ERROR 0
#define LIST_ERROR_EMPTY 2			// Error, the list is empty.
#define LIST_ERROR_MALLOC 3			// Error, a malloc failed.
#define LIST_ERROR_END 4			// Error, the POI is at the tail.
#define LIST_ERROR_START 5			// Error, the POI is at the head.
#define LIST_ERROR_INVALID 6		// Error, the POI is on a phantom node.


// Data Types
typedef struct {
	char *username;
	char *ip_network;
	int port;
	pid_t pid;
	int clientFD;
} Element;

/*
 * Node is a recursive structure that will contain each one of the elements.
 * A node has two kinds of fields, the element to store and the pointers to
 *  the next/previous node in the structure.
 * The structure is recursively defined (a Node has a pointer to another
 *  node), so we need to define a new type (typedef) from a structure 
 *  (struct _Node).
 */
typedef struct _Node {
	Element element;
	struct _Node * next;
	struct _Node * previous;
} Node;


typedef struct {
	int error;
	Node * head;
	Node * tail;
	Node * poi;
} BidirectionalList;


// Global module variables


// Headers
BidirectionalList BIDIRECTIONALLIST_create();

void	BIDIRECTIONALLIST_addBefore(BidirectionalList * list, Element element);
void	BIDIRECTIONALLIST_addAfter (BidirectionalList * list, Element element);
Element	BIDIRECTIONALLIST_get(BidirectionalList * list);
void	BIDIRECTIONALLIST_remove(BidirectionalList * list);
int		BIDIRECTIONALLIST_isEmpty(BidirectionalList list);

int 	BIDIRECTIONALLIST_isValid(BidirectionalList list);

void	BIDIRECTIONALLIST_goToHead(BidirectionalList * list);
void	BIDIRECTIONALLIST_next(BidirectionalList * list);

void	BIDIRECTIONALLIST_goToTail(BidirectionalList * list);
void	BIDIRECTIONALLIST_previous(BidirectionalList * list);

void 	BIDIRECTIONALLIST_destroy(BidirectionalList * list);

int		BIDIRECTIONALLIST_getErrorCode(BidirectionalList list);

#endif