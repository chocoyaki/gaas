#include "Vector.h"


/* forward declarations for private functions */
static int __Vector_idxIsValid(Vector_t v, int idx);
static int __pointerCompare(const void* o1, const void* o2);


Vector_t
new_Vector()
{
  return ((Vector_t) calloc (1, sizeof (struct s_Vector)));
}

Vector_t
new_Vector1(int capacity)
{
  Vector_t v = new_Vector();
  Vector_ensureCapacity(v, capacity);
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
Vector_add(Vector_t v, void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_add: NULL v\n");
    return (0);
  }

  Vector_ensureCapacity(v, Vector_size(v) + 1);

  (v->v_obj)[v->v_size] = o;
  v->v_size++;

  return (1);
}

int
Vector_addAtPosition(Vector_t v, void *o, int idx)
{
  int i;

  if (v == NULL) {
    fprintf(stderr, "Vector_addAtPosition: NULL v\n");
    return (0);
  }
  if (idx < 0) {
    fprintf(stderr, "Vector_addAtPosition: bad idx: %d\n", idx);
    return (0);
  }

  if (idx <= Vector_size(v)) {
    Vector_ensureCapacity(v, Vector_size(v) + 1);
    for (i = v->v_size ; i > idx ; i--) {
      (v->v_obj)[i] = (v->v_obj)[i-1];
    }
    v->v_size++;
  }
  else {
    Vector_ensureCapacity(v, idx + 1);
    for (i = v->v_size ; i < idx ; i++) {
      (v->v_obj)[i] = NULL;
    }
    v->v_size = idx+1;
  }

  (v->v_obj)[idx] = o;
  return (1);
}

int
Vector_capacity(Vector_t v)
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

  for (i = 0 ; i < v->v_size ; i++) {
    v->v_obj[i] = NULL;
  }
  v->v_size = 0;

  return (rv);
}

int
Vector_contains(Vector_t v, void* o)
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
Vector_copy(Vector_t v)
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
  copy->v_obj = (void**) calloc(v->v_capacity, sizeof (void*));
  memcpy(copy->v_obj, v->v_obj, v->v_capacity * sizeof (void*));

  return (copy);
}

void*
Vector_elementAt(Vector_t v, int idx)
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
  int i;
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

  v->v_obj = (void**) realloc(v->v_obj, newCapacity * sizeof (void*));
  if (v->v_obj == NULL) {
    fprintf(stderr, "Vector_ensureCapacity: failed realloc\n");
    return (0);
  }

  for (i = v->v_capacity ; i < newCapacity ; i++) {
    (v->v_obj)[i] = NULL;
  }
  v->v_capacity = newCapacity;

  return (1);
}


int
Vector_indexOf(Vector_t v, const void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_indexOf: NULL v\n");
    return (0);
  }

  return (Vector_indexOf3(v, o, &__pointerCompare));
}

int
Vector_indexOf3(Vector_t v, const void* o, compfn_t c)
{
  int i;

  if (v == NULL) {
    fprintf(stderr, "Vector_indexOf3: NULL v\n");
    return (0);
  }

  for (i = 0 ; i < v->v_size ; i++) {
    if ((*c)(o, v->v_obj[i])) {
      return (i);
    }
  }

  return (-1);
}

int
Vector_isEmpty(Vector_t v)
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

void*
Vector_lastElement(Vector_t v)
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
Vector_lastIndexOf(Vector_t v, const void* o)
{
  int i;

  if (v == NULL) {
    fprintf(stderr, "Vector_lastIndexOf: NULL v\n");
    return (0);
  }

  for (i = v->v_size - 1 ; i >= 0 ; i--) {
    if (o == (v->v_obj)[i]) {
      return (i);
    }
  }

  return (-1);
}

int
Vector_remove(Vector_t v, void* o)
{
  if (v == NULL) {
    fprintf(stderr, "Vector_remove3: NULL v\n");
    return (-1);
  }

  return (Vector_remove3(v, o, &__pointerCompare));
}

int
Vector_remove3(Vector_t v, void* o, compfn_t c)
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
  int i;
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

  rv = (v->v_obj)[idx];
  for (i = idx ; i < v->v_size - 1 ; i++) {
    (v->v_obj)[i] = (v->v_obj)[i+1];
  }
  (v->v_obj)[v->v_size - 1] = NULL;
  v->v_size--;

  return (rv);
}

void*
Vector_set(Vector_t v, void* o, int idx)
{
  void* rv = NULL;

  if (v == NULL) {
    fprintf(stderr, "Vector_set: NULL v\n");
    return (0);
  }

  if (__Vector_idxIsValid(v, idx)) {
    rv = (v->v_obj)[idx];
    (v->v_obj)[idx] = o;
  }
  else {
    Vector_addAtPosition(v, o, idx);
  }

  return (rv);
}

int
Vector_size(Vector_t v)
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
    v->v_obj = (void**) realloc(v->v_obj, v->v_size * sizeof (void*));
    v->v_capacity = v->v_size;
  }

  return (1);
}


/* private functions */

static int
__Vector_idxIsValid(Vector_t v, int idx)
{
  return (idx >= 0 && idx < v->v_size);
}
static int
__pointerCompare(const void* o1, const void* o2)
{
  return (o1 == o2);
}
