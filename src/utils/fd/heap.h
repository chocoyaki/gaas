
	/***************************************\
	 * Heap (sorted in decreasing order)   *
	 * Aurélien Bouteiller                 *
	\***************************************/


#ifndef _HEAP_H_
#define _HEAP_H_


/* type definition */
typedef struct _heap {
	int (*compare) (void *, void *);
	void** data;
	int nb_elements;
	int size;
} _heap;
typedef struct _heap * heap;


/* constructor */
heap heap_new (int (*compare)(void*, void*));

/* destructor */
void heap_destroy (heap);

/* emptiness test */
int heap_is_empty (heap);

/* insertion */
void heap_insert (heap, void*);

/* peek */
void* heap_peek (heap);

/* extraction */
void* heap_extract (heap);

#endif /* _HEAP_H_ */

