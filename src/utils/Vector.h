#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct s_Vector {
  void** v_obj;
  int v_size;
  int v_capacity;
} s_Vector, *Vector_t;

typedef int(*compfn_t)(const void*, const void*);

Vector_t new_Vector();
Vector_t new_Vector1(int capacity);
void free_Vector(Vector_t v);
int Vector_add(Vector_t v, void* o);
int Vector_addAtPosition(Vector_t v, void *o, int idx);
int Vector_capacity(Vector_t v);
int Vector_clear(Vector_t v);
int Vector_contains(Vector_t v, void* o);
Vector_t Vector_copy(Vector_t v);
void* Vector_elementAt(Vector_t v, int idx);
int Vector_ensureCapacity(Vector_t v, int capacity);
int Vector_indexOf(Vector_t v, const void* o);
int Vector_indexOf3(Vector_t v, const void* o, compfn_t c);
int Vector_isEmpty(Vector_t v);
void* Vector_lastElement(Vector_t v);
int Vector_lastIndexOf(Vector_t v, const void* o);
int Vector_remove(Vector_t v, void* o);
int Vector_remove3(Vector_t v, void* o, compfn_t c);
void* Vector_removeAtPosition(Vector_t v, int idx);
void* Vector_set(Vector_t v, void* o, int idx);
int Vector_size(Vector_t v);
void Vector_sort(Vector_t v, int (*sort_fn)(const void* a, const void* b));
int Vector_trimToSize(Vector_t v);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _VECTOR_H_ */
