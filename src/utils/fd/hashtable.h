/****************************************************************************/
/* DIET Fault Detector simple hashtable headers                             */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aur�lien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.3  2006/06/20 13:29:16  abouteil
 *
 *
 * Chandra&Toueg&Aguilera fault detector implementation.
 *
 * A
 *
 ****************************************************************************/
#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_


#include "list.h"


/* some prime number */
#define HASHTABLE_SIZE 211


/* hash table = array of lists with user-supplied functions :
 * - hash : takes an element and returns an integer in [0..MAXINT]
 * - matches : tests equality between searched and candidate
 */
typedef struct _hashtable {
	unsigned int (*hash) (void *);
	unsigned int (*matches) (void*, void*);	/* true means match */
	list* data;
} _hashtable;
typedef struct _hashtable * hashtable;

/* constructor and destructor */
hashtable hashtable_new (unsigned int (*hash)(void *),
	unsigned int (*compare)(void *, void*));
void hashtable_destroy (hashtable);


/* insertion, search, removing */
void hashtable_insert (hashtable, void*);
void* hashtable_search (hashtable, void*);
void hashtable_remove (hashtable, void*);


#endif /* HASHTABLE_H_ */

