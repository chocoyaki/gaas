/****************************************************************************/
/* DIET Fault Detector simple lists                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.6  2010/10/04 08:17:23  bdepardo
 * Changed memory management from C to C++ (malloc/free -> new/delete)
 * This corrects a bug at initialization
 *
 * Revision 1.5  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.4  2006/11/01 14:02:25  ecaron
 * Correct wrong end line character (Ctrl+M)
 *
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/
#include <cstdlib>

#include "list.h"



/* destructor (does NOT free the cells' contents) */
void list_destroy (list l) {
	list current, next;

	current = l;
	while (current != NULL) {
		next = list_tail (current);
		delete current;
		current = next;
	}
}


/* basic operations */
void* list_head (list l) {
	return l->head;
}

list list_tail (list l) {
	return l->tail;
}

/* returns the new list, the list that was passed as a parameter is not to
 * be used anymore, i.e. mylist = list_add (mylist, myelement);
 */
list list_cons (list l, void* c) {
	list n = new _list;
	n->head = c;
	n->tail = l;
	return n;
}

