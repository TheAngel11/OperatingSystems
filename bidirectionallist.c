/*********************************************************************
* @Purpose: Contains all the necessary functions to manage a
*           bidirectional list of a defined element.
* @Last change: 18/11/2022
*********************************************************************/
#include "bidirectionallist.h"

BidirectionalList BIDIRECTIONALLIST_create() {
	BidirectionalList list;

	list.head = NULL;
	// First phantom node creation
	list.head = (Node*) malloc (sizeof (Node));
	if (list.head != NULL) {
		list.tail = NULL;

		// Second (tail) phantom node creation
		list.tail = (Node*) malloc (sizeof (Node));
		if (list.tail == NULL) {
			// The second malloc failed, we can't create the list
			// Destroy anything already created (aka first phantom).
			free(list.head);

			list.head = NULL;
			list.error = LIST_ERROR_MALLOC;
		}
		else {
			list.error = LIST_NO_ERROR;

			list.head->next = list.tail;
			list.head->previous = NULL;
			list.tail->next = NULL;
			list.tail->previous = list.head;
			list.poi = list.head;
		}
	}
	else {
		list.head = NULL;
		list.tail = NULL;
		list.poi  = NULL;
		list.error = LIST_ERROR_MALLOC;
	}

	return list;
}

void BIDIRECTIONALLIST_addBefore(BidirectionalList * list, Element element) {
	Node * new_node = NULL;
	
	// We cannot add elements before the head phantom node. 
	if (list->poi == list->head) {
		list->error = LIST_ERROR_INVALID;
	}
	else {
		new_node = (Node*) malloc (sizeof (Node));
		if (NULL == new_node) {
			list->error = LIST_ERROR_MALLOC;
		}
		else {
			list->error = LIST_NO_ERROR;

			// Link the new node to the structure.
			new_node->element = element;
			new_node->next = list->poi;
			new_node->previous = list->poi->previous;
			
			list->poi->previous->next = new_node;
			list->poi->previous = new_node;		
		}
	}
}

void BIDIRECTIONALLIST_addAfter (BidirectionalList * list, Element element) {
	Node * new_node = NULL;
	// We cannot add elements after the tail phantom node. 
	if (list->poi == list->tail) {
		list->error = LIST_ERROR_INVALID;
	}
	else {
		new_node = (Node*) malloc (sizeof (Node));
		if (NULL == new_node) {
			list->error = LIST_ERROR_MALLOC;
		}
		else {
			list->error = LIST_NO_ERROR;

			// Link the new node to the structure.
			new_node->element = element;
			new_node->next = list->poi->next;
			new_node->previous = list->poi;
			
			list->poi->next->previous = new_node;
			list->poi->next = new_node;		
		}
	}
}

Element	BIDIRECTIONALLIST_get(BidirectionalList * list) {
	Element element;

	if (BIDIRECTIONALLIST_isEmpty(*list)) {
		list->error = LIST_ERROR_EMPTY;
	} else {
		if (!BIDIRECTIONALLIST_isValid(*list))	{
			list->error = LIST_ERROR_INVALID;
		}
		else {
			list->error = LIST_NO_ERROR;
			element = list->poi->element;
		}
	}

	return element;
}

void BIDIRECTIONALLIST_remove(BidirectionalList * list) {
	Node * aux = NULL;

	if (BIDIRECTIONALLIST_isEmpty(*list)) {
		list->error = LIST_ERROR_EMPTY;
	} else {
		if (!BIDIRECTIONALLIST_isValid(*list))	{
			list->error = LIST_ERROR_INVALID;
		}
		else {
			aux = list->poi;

			list->poi->previous->next = list->poi->next;
			list->poi->next->previous = list->poi->previous;

			list->poi = list->poi->next;
			free(aux);
		}
	}
}

int	BIDIRECTIONALLIST_isEmpty(BidirectionalList list) {
	return list.head->next == list.tail;
}

int BIDIRECTIONALLIST_isValid(BidirectionalList list) {
	return list.poi != list.head && list.poi != list.tail;
}

void BIDIRECTIONALLIST_goToHead(BidirectionalList * list) {
	list->poi = list->head->next;
}

void BIDIRECTIONALLIST_next(BidirectionalList * list) {
	if (list->poi == list->tail) {
		list->error = LIST_ERROR_END;
	}
	else {
		list->poi = list->poi->next;
	}
}

void BIDIRECTIONALLIST_goToTail(BidirectionalList * list) {
	list->poi = list->tail->previous;
}

void BIDIRECTIONALLIST_previous(BidirectionalList * list) {
	if (list->poi == list->head) {
		list->error = LIST_ERROR_START;
	}
	else {
		list->poi = list->poi->previous;
	}
}


void BIDIRECTIONALLIST_destroy(BidirectionalList * list) {
	while (list->head != NULL) {
		list->poi = list->head;
		list->head = list->head->next;
		free(list->poi);
	}
	list->tail = NULL;
	list->poi = NULL;
}
