/****************************************************************************/
/* DIET Fault Detector simple hashtable                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
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

#include "hashtable.h"


/* constructor */
hashtable hashtable_new (unsigned int (*hash)(void *),
	unsigned int (*matches)(void *, void*))
{
	int i;
	hashtable result = (hashtable) malloc (sizeof (hashtable));
	result->hash = hash;
	result->matches = matches;
	result->data = (list*) malloc (HASHTABLE_SIZE * sizeof (list));
	for (i = 0; i < HASHTABLE_SIZE; ++i)
		result->data[i] = EMPTY_LIST;
	return result;
}

/* destructor (does NOT free the contained cells) */
void hashtable_destroy (hashtable h)
{
	int i;
	for (i = 0; i < HASHTABLE_SIZE; ++i)
		list_destroy (h->data[i]);
	free (h);
}


/* insertion */
void hashtable_insert (hashtable h, void* c)
{
	int key = (h->hash (c)) % HASHTABLE_SIZE;
	h->data[key] = list_cons (h->data[key], c);
}

/* search */
void* hashtable_search (hashtable h, void* c)
{
	void * e;
	list l = h->data [(h->hash (c)) % HASHTABLE_SIZE];
	while (l != NULL) {
		e = list_head (l);
		if (h->matches (c, e)) {
			return e;
		} else {
			l = list_tail (l);
		}
	}
	return NULL;
}

/* removing */
void hashtable_remove (hashtable h, void* c)
{
	list previous, current;
	int key = (h->hash (c)) % HASHTABLE_SIZE;
	list l = h->data [key];
	
	/* special case 1 : empty list */
	if (l == NULL) return;

	/* special case 2 : searched element is at head */
	current = list_tail (l);
	if (h->matches (list_head (l), c)) {
		free (l);
		h->data [key] = current;
		return;
	}

	/* normal case */
	previous = l;
	while (current != NULL) {
		if (h->matches (list_head (current), c)) {
			previous->tail = list_tail (current);
			free (current);
			return;
		} else {
			previous = current;
			current = list_tail (current);
		}
	}
}

