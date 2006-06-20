/****************************************************************************/
/* DIET Fault Detector simple lists headers                                 */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$ 
 * $Log$
 * Revision 1.2  2006/06/20 13:18:27  abouteil
 *
 * Testing if comments in headers are correct
 *
 ****************************************************************************/
#ifndef _LIST_H_
#define _LIST_H_


/* type definition */
typedef struct _list {
	void* head;
	struct _list* tail;
} _list;
typedef struct _list* list;


/* constructor (kind of) */
#define EMPTY_LIST NULL


/* destructor (does NOT free the cells' contents) */
void list_destroy (list);


/* basic operations */
void* list_head (list);
list list_tail (list);
list list_cons (list, void*);


#endif /* _LIST_H_ */

