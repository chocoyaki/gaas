/**
 * @file  Vector.h
 *
 * @brief  Plug-in Scheduler: Information vector management
 *
 * @author  Alan Su (Alan.Su@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _VECTOR_H_
#define _VECTOR_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct Vector_s {
  const void **v_obj;
  int v_size;
  int v_capacity;
} *Vector_t;

typedef const struct Vector_s *VectorConst_t;

typedef int (*compfn_t)(const void *, const void *);

Vector_t
new_Vector();

Vector_t
new_Vector1(int capacity);

void
free_Vector(Vector_t v);

int
Vector_add(Vector_t v, const void *o);

int
Vector_addAtPosition(Vector_t v, const void *o, int idx);

int
Vector_capacity(const VectorConst_t v);

int
Vector_clear(Vector_t v);

int
Vector_contains(const VectorConst_t v, const void *o);

Vector_t
Vector_copy(const VectorConst_t v);

const void *
Vector_elementAt(const VectorConst_t v, int idx);

int
Vector_ensureCapacity(Vector_t v, int capacity);

int
Vector_indexOf(const VectorConst_t v, const void *o);

int
Vector_indexOf3(const VectorConst_t v, const void *o, compfn_t c);

int
Vector_isEmpty(const VectorConst_t v);

const void *
Vector_lastElement(const VectorConst_t v);

int
Vector_lastIndexOf(const VectorConst_t v, const void *o);

int
Vector_remove(Vector_t v, const void *o);

int
Vector_remove3(Vector_t v, const void *o, compfn_t c);

void *
Vector_removeAtPosition(Vector_t v, int idx);

const void *
Vector_set(Vector_t v, const void *o, int idx);

int
Vector_size(const VectorConst_t v);

void
Vector_sort(Vector_t v, int (*sort_fn)(const void *a, const void *b));

int
Vector_trimToSize(Vector_t v);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _VECTOR_H_ */
