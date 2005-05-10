/****************************************************************************/
/* DIET performance estimation vector                                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2005/05/10 11:55:07  alsu
 * optimized and retested estimation vector
 *
 * Revision 1.2  2004/12/08 15:02:52  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.1.2.3  2004/11/06 16:22:55  alsu
 * estimation vector access functions now have parameter-based default
 * return values
 *
 * Revision 1.1.2.2  2004/11/02 00:43:42  alsu
 * adding a setEstimation interface for the estimation vector (to be used
 * in the case where a value for a particular tag should replace all
 * instances of that tag)
 *
 * Revision 1.1.2.1  2004/10/31 22:14:21  alsu
 * implementation of an "estimation vector": a collection of performance
 * values either user- or system-defined.
 *
 ****************************************************************************/

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_data.h"
#include "DIET_server.h"

typedef struct __tagvalpair_s {
  diet_est_tag_t __tag;
  double         __val;
} __tagvalpair_s, *__tagvalpair_t;
struct estVector_s {
  size_t ev_num;
  size_t ev_cap;
  __tagvalpair_t ev_vect;
};

static int
__estVector_ensureCapacity(estVector_t ev, size_t cap)
{
  if (ev->ev_cap >= cap) {
    return (1);
  }
  ev->ev_vect = realloc(ev->ev_vect, cap * sizeof (struct __tagvalpair_s));
  if (ev->ev_vect == NULL) {
    return (0);
  }

  {
    int numElts = cap - ev->ev_cap;
    memset(&(ev->ev_vect[ev->ev_cap]),
           0,
           numElts * sizeof (struct __tagvalpair_s));
  }

  ev->ev_cap = cap;
  return (1);
}
static int
__estVector_grow(estVector_t ev)
{
  return (__estVector_ensureCapacity(ev, ev->ev_cap * 2));
}


estVector_t
new_estVector()
{
  estVector_t ev = calloc(1, sizeof (struct estVector_s));
  assert(ev != NULL);
  return (ev);
}

int
estVector_addEstimation(estVector_t ev, diet_est_tag_t tag, double val)
{
  assert(ev != NULL);
  if (ev->ev_num == ev->ev_cap) {
    __estVector_grow(ev);
  }
  return (ev->ev_num);
}

int
estVector_setEstimation(estVector_t ev, diet_est_tag_t tag, double val)
{
  assert(ev != NULL);

  { /* eliminate all instances of this tag */
    int tvpIter = 0;
    while (tvpIter < ev->ev_num) {
      if ((ev->ev_vect[tvpIter]).__tag == tag) {
        ev->ev_vect[tvpIter] = ev->ev_vect[ev->ev_num-1];
        memset(&(ev->ev_vect[ev->ev_num-1]),
               0,
               sizeof (struct __tagvalpair_s));
        ev->ev_num--;
      }
      else {
        tvpIter++;
      }
    }
  }
  estVector_addEstimation(ev, tag, val);

  return (ev->ev_num);
}

int
estVector_numEstimations(estVector_t ev)
{
  assert(ev != NULL);
  return (ev->ev_num);
}

int
estVector_numEstimationsByType(estVector_t ev, diet_est_tag_t tag)
{
  int tvpIter, count;

  assert(ev != NULL);

  for (tvpIter = 0, count = 0 ; tvpIter < ev->ev_num ; tvpIter++) {
    if (ev->ev_vect[tvpIter].__tag == tag) {
      count++;
    }
  }
  return (count);
}

diet_est_tag_t
estVector_getEstimationTagByIdx(estVector_t ev, int idx)
{
  assert(ev != NULL);
  if (idx >= ev->ev_num) {
    return (EST_INVALID);
  }
  return (ev->ev_vect[idx].__tag);
}

double
estVector_getEstimationValueByIdx(estVector_t ev, int idx, double errVal)
{
  assert(ev != NULL);
  assert(idx >= 0);

  if (idx >= ev->ev_num) {
    return (EST_INVALID);
  }
  return (ev->ev_vect[idx].__val);
}

double
estVector_getEstimationValue(estVector_t ev,
                             diet_est_tag_t tag,
                             double errVal)
{
  return (estVector_getEstimationValueNum(ev, tag, errVal, 0));
}

double
estVector_getEstimationValueNum(estVector_t ev,
                                diet_est_tag_t tag,
                                double errVal,
                                int idx)
{
  int tvpIter;
  int found;

  assert(ev != NULL);
  assert(idx >= 0);

  for (found = 0, tvpIter = 0 ; tvpIter < ev->ev_num ; tvpIter++) {
    if (ev->ev_vect[tvpIter].__tag != tag) {
      continue;
    }
    if (found == idx) {
      return (ev->ev_vect[tvpIter].__val);
    }
    found++;
  }
  return (errVal);
}

void
free_estVector(estVector_t ev)
{
  assert(ev != NULL);

  if (ev->ev_cap > 0) {
    assert(ev->ev_vect != NULL);
    free(ev->ev_vect);
  }

  free(ev);
}
