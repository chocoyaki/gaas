/****************************************************************************/
/* DIET Fault Detector simple heap (decr. ordering)                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Aurélien Bouteiller (aurelien.bouteiller@inria.fr)                  */
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
#include <stdlib.h>
#include "heap.h"

/* minimum size */
#define HEAP_MIN_SIZE 1024


/* constructor */
heap heap_new (int (*compare)(void*, void*))
{
	heap result = (heap) malloc (sizeof (heap));
	result->compare = compare;
	result->data = (void **) malloc (HEAP_MIN_SIZE * sizeof (void *));
	result->nb_elements = 0;
	result->size = HEAP_MIN_SIZE;
	return result;
}

/* destructor */
void heap_destroy (heap h)
{
	free (h->data);
	free (h);
}

/* emptiness test */
int heap_is_empty (heap h)
{
	return (h->nb_elements == 0);
}

/* utility functions */

/* memory management */
void heap_check_more (heap h)
{
	if (h->nb_elements > h->size) {
		h->size = 2*h->size;
		h->data = (void **) realloc (h->data, h->size * sizeof (void*));
	}
}

void heap_check_less (heap h)
{
	if ((h->nb_elements < h->size / 3) && (h->size > HEAP_MIN_SIZE)) {
		h->size = h->size / 2;
		h->data = (void **) realloc (h->data, h->size * sizeof (void*));
	}
}

/* heap navigation */
int heap_parent (int i)
{
	return (i-1)/2;
}

int heap_left (int i)
{
	return (i*2)+1;
}

int heap_right (int i)
{
	return (i*2)+2;
}

/* insertion */
void heap_insert (heap h, void* c)
{
	void* tmp;
	int p;
	int i;
	
	i = h->nb_elements;
	++h->nb_elements;
	heap_check_more (h);
	h->data[i] = c;
	p = heap_parent (i);
	/* reorder elements in heap to keep it partially sorted */
	while ((i > 0) && (h->compare (h->data[i], h->data[p]) < 0)) {
		tmp = h->data[i];
		h->data[i] = h->data[p];
		h->data[p] = tmp;
		i = p;
		p = heap_parent (i);
	}
}

/* peek */
void* heap_peek (heap h)
{
	if (h->nb_elements == 0)
		return NULL;
	else
		return h->data[0];
}

/* extraction */
void* heap_extract (heap h)
{
	int i, r, l, m;
	void* tmp;
	void* result;
	
	if (h->nb_elements == 0) return NULL;

	result = h->data[0];

	h->data[0] = h->data[h->nb_elements-1];
	
	--h->nb_elements;
	heap_check_less (h);

	/* reordering */
	i = 0;
	while (1) {
		l = heap_left (i);
		r = heap_right (i);

		if ((l < h->nb_elements) &&
			(h->compare (h->data[l], h->data[i]) < 0)) {
			m = l;
		} else {
			m = i;
		}
		if ((r < h->nb_elements) &&
			(h->compare (h->data[r], h->data[m]) < 0)) {
			m = r;
		}
		if (m == i) {
			break;
		} else {
			tmp = h->data[i];
			h->data[i] = h->data[m];
			h->data[m] = tmp;
			i = m;
		}
	};
	return result;
}

