
	/***************************************\
	 * Linked List                         *
	 * Aurélien Bouteiller                 *
	\***************************************/


#include <stdlib.h>

#include "list.h"



/* destructor (does NOT free the cells' contents) */
void list_destroy (list l)
{
	list current, next;

	current = l;
	while (current != NULL) {
		next = list_tail (current);
		free (current);
		current = next;
	}
}


/* basic operations */
void* list_head (list l)
{
	return l->head;
}

list list_tail (list l)
{
	return l->tail;
}

/* returns the new list, the list that was passed as a parameter is not to
 * be used anymore, i.e. mylist = list_add (mylist, myelement);
 */
list list_cons (list l, void* c)
{
	list n = (list) malloc (sizeof(list));
	n->head = c;
	n->tail = l;
	return n;
}

