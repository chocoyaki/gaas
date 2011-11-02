/**
* @file  Vector.c
* 
* @brief  Plug-in Scheduler: Information vector management 
* 
* @author  - Alan Su (Alan.Su@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.8  2008/04/01 09:13:37  bdepardo
 * Removed cast of sort_fn which was added to remove warning with intel compiler, but caused an error when compiling on OS X with GCC 4.0.1
 *
 * Revision 1.7  2008/03/28 13:17:17  rbolze
 * update code to avoid warning with the intel compiler
 *
 * Revision 1.6  2006/11/01 21:38:04  ecaron
 * Correction of wrong comment
 *
 * Revision 1.5  2006/10/31 23:25:04  ecaron
 * Management of vector information used in plug-in scheduler context
 *
 ****************************************************************************/
#include "Vector.h"


/* forward declarations for private functions */
static int __Vector_idxIsValid(const VectorConst_t v, int idx);
static int __pointerCompare(const void* o1, const void* o2);


Vector_t
new_Vector()
{
  return ((Vector_t) calloc (1, sizeof (struct Vector_s)));
}

Vector_t
new_Vector1(int capacity)
{
  Vector_t v = new_Vector();
  if (v == NULL) {
    fprintf(stderr, "new_Vector1: unable to allocate vector\n");
    return (NULL);
  }
  if (! Vector_ensureCapacity(v, capacity)) {
    fprintf(stderr, "new_Vector1: unable to ensure capacity %d\n", capacity);
    free_Vector(v);
    return (NULL);
  }
  return (v);
}

void
free_Vector(Vector_t v)
{
  if (v == NULL) {
    fprintf(stderr, "free_Vector: NULL v\n");
    return;
  }

  if (v->v_size > 0) {
    fprintf(stderr,
            "free_Vector: (warning) vector not empty (%d elts)\n",
            v->v_size);
  }

  free(v->v_obj);
  free(v);
}

int
Vector_add(Vector_t v, const void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_add: NULL v\n");
    return (0);
  }

  if (! Vector_ensureCapacity(v, Vector_size(v) + 1)) {
    fprintf(stderr,
            "Vector_add: unable to ensure capacity %d\n",
            Vector_size(v) + 1);
    return (0);
  }

  (v->v_obj)[v->v_size] = o;
  v->v_size++;

  return (1);
}

int
Vector_addAtPosition(Vector_t v, const void *o, int idx)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_addAtPosition: NULL v\n");
    return (0);
  }
  if (idx < 0) {
    fprintf(stderr, "Vector_addAtPosition: bad idx: %d\n", idx);
    return (0);
  }

  if (idx <= Vector_size(v)) {
    if (! Vector_ensureCapacity(v, Vector_size(v) + 1)) {
      fprintf(stderr,
              "Vector_addAtPosition: unable to ensure capacity %d\n",
              Vector_size(v) + 1);
      return (0);
    }

    memmove(&((v->v_obj)[idx+1]),
            &((v->v_obj)[idx]),
            (v->v_size - idx) * sizeof (void *));
    v->v_size++;
  }
  else {
    if (! Vector_ensureCapacity(v, idx + 1)) {
      fprintf(stderr,
              "Vector_addAtPosition: unable to ensure capacity %d\n",
              idx + 1);
      return (0);
    }
    v->v_size = idx+1;
  }

  (v->v_obj)[idx] = o;
  return (1);
}

int
Vector_capacity(const VectorConst_t v)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_capacity: NULL v\n");
    return (0);
  }

  return (v->v_capacity);
}

int
Vector_clear(Vector_t v)
{
  int i, rv;

  if (v == NULL) {
    fprintf(stderr, "Vector_clear: NULL v\n");
    return (-1);
  }

  rv = v->v_size;

  for (i = 0; i < v->v_size; i++) {
    v->v_obj[i] = NULL;
  }
  v->v_size = 0;

  return (rv);
}

int
Vector_contains(const VectorConst_t v, const void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_contains: NULL v\n");
    return (0);
  }

  if (Vector_indexOf(v, o) == -1) {
    return (0);
  }
  return (1);
}

Vector_t
Vector_copy(const VectorConst_t v)
{
  Vector_t copy;

  if (v == NULL) {
    fprintf(stderr, "Vector_copy: NULL v\n");
    return (NULL);
  }
  if ((copy = new_Vector()) == NULL) {
    fprintf(stderr, "Vector_copy: failed to allocate copy\n");
    return (NULL);
  }

  copy->v_size = v->v_size;
  copy->v_capacity = v->v_capacity;
  copy->v_obj = (const void**) calloc(v->v_capacity, sizeof (const void*));
  memcpy(copy->v_obj, v->v_obj, v->v_capacity * sizeof (void*));

  return (copy);
}

const void *
Vector_elementAt(const VectorConst_t v, int idx)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_elementAt: NULL v\n");
    return (NULL);
  }
  if (! __Vector_idxIsValid(v, idx)) {
    fprintf(stderr,
            "Vector_elementAt: idx %d out of range (0-%d)\n",
            idx,
            Vector_size(v) - 1);
    return (NULL);
  }

  return ((v->v_obj)[idx]);
}

int
Vector_ensureCapacity(Vector_t v, int capacity)
{
  int newCapacity;

  if (v == NULL) {
    fprintf(stderr, "Vector_ensureCapacity: NULL v\n");
    return (0);
  }

  if (capacity < 1) {
    fprintf(stderr, "Vector_ensureCapacity: invalid capacity %d\n", capacity);
    return (0);
  }

  if (capacity <= v->v_capacity) {
    return (1);
  }

  if (v->v_capacity * 2 < capacity) {
    newCapacity = capacity;
  }
  else {
    newCapacity = v->v_capacity * 2;
  }

  v->v_obj = (const void**) realloc(v->v_obj,
                                    newCapacity * sizeof (const void*));
  if (v->v_obj == NULL) {
    fprintf(stderr, "Vector_ensureCapacity: failed realloc\n");
    return (0);
  }
  else {
    size_t newAreaSize = (newCapacity - v->v_capacity) * sizeof (void *);
    const void **newAreaStart = &((v->v_obj)[v->v_capacity]);
    memset(newAreaStart, '\0', newAreaSize);
  }
  v->v_capacity = newCapacity;

  return (1);
}


int
Vector_indexOf(const VectorConst_t v, const void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_indexOf: NULL v\n");
    return (0);
  }

  return (Vector_indexOf3(v, o, &__pointerCompare));
}

int
Vector_indexOf3(const VectorConst_t v, const void* o, compfn_t c)
{
  int i;

  if (v == NULL) {
    fprintf(stderr, "Vector_indexOf3: NULL v\n");
    return (0);
  }

  for (i = 0; i < v->v_size; i++) {
    if ((*c)(o, v->v_obj[i])) {
      return (i);
    }
  }

  return (-1);
}

int
Vector_isEmpty(const VectorConst_t v)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_isEmpty: NULL v\n");
    return (0);
  }

  if (v->v_size == 0) {
    return (1);
  }

  return (0);
}

const void *
Vector_lastElement(const VectorConst_t v)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_lastElement: NULL v\n");
    return (NULL);
  }
  if (v->v_size == 0) {
    fprintf(stderr, "Vector_lastElement: empty vector\n");
    return (NULL);
  }

  return ((v->v_obj)[v->v_size - 1]);
}

int
Vector_lastIndexOf(const VectorConst_t v, const void* o)
{
  int i;

  if (v == NULL) {
    fprintf(stderr, "Vector_lastIndexOf: NULL v\n");
    return (0);
  }

  for (i = v->v_size - 1; i >= 0; i--) {
    if (o == (v->v_obj)[i]) {
      return (i);
    }
  }

  return (-1);
}

int
Vector_remove(Vector_t v, const void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_remove3: NULL v\n");
    return (-1);
  }

  return (Vector_remove3(v, o, &__pointerCompare));
}

int
Vector_remove3(Vector_t v, const void* o, compfn_t c)
{
  int oidx;

  if (v == NULL) {
    fprintf(stderr, "Vector_remove3: NULL v\n");
    return (-1);
  }

  if ((oidx = Vector_indexOf3(v, o, c)) != -1) {
    Vector_removeAtPosition(v, oidx);
  }
  else {
    fprintf(stderr, "Vector_remove3: element not found\n");
  }

  return (oidx);
}

void*
Vector_removeAtPosition(Vector_t v, int idx)
{
  void* rv;

  if (v == NULL) {
    fprintf(stderr, "Vector_removeAtPosition: NULL v\n");
    return (0);
  }
  if (! __Vector_idxIsValid(v, idx)) {
    fprintf(stderr,
            "Vector_removeAtPosition: idx %d out of range (0-%d)\n",
            idx,
            Vector_size(v) - 1);
    return (NULL);
  }

  rv = (void *)(v->v_obj)[idx];
  memmove(&((v->v_obj)[idx]),
          &((v->v_obj)[idx+1]),
          (v->v_size - 1 - idx) * sizeof (void *));
  (v->v_obj)[v->v_size - 1] = NULL;
  v->v_size--;

  return (rv);
}

const void*
Vector_set(Vector_t v, const void* o, int idx)
{
  const void* rv = NULL;

  if (v == NULL) {
    fprintf(stderr, "Vector_set: NULL v\n");
    return (0);
  }

  if (__Vector_idxIsValid(v, idx)) {
    rv = (v->v_obj)[idx];
    (v->v_obj)[idx] = o;
  }
  else {
    if (! Vector_addAtPosition(v, o, idx)) {
      fprintf(stderr, "Vector_set: unable to add object at index %d\n", idx);
      return (0);
    }
  }

  return (rv);
}

int
Vector_size(const VectorConst_t v)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_size: NULL v\n");
    return (0);
  }

  return (v->v_size);
}

void
Vector_sort(Vector_t v, int (*sort_fn)(const void* a, const void* b))
{
  qsort(v->v_obj, v->v_size, sizeof (void*), sort_fn);
}

int
Vector_trimToSize(Vector_t v)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_trimToSize: NULL v\n");
    return (0);
  }

  if (v->v_capacity > v->v_size) {
    v->v_obj = (const void**) realloc(v->v_obj,
                                      v->v_size * sizeof (const void*));
    v->v_capacity = v->v_size;
  }

  return (1);
}


/* private functions */

static int
__Vector_idxIsValid(const VectorConst_t v, int idx)
{
  return (idx >= 0 && idx < v->v_size);
}
static int
__pointerCompare(const void* o1, const void* o2)
{
  return (o1 == o2);
}
