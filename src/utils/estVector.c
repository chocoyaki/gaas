/****************************************************************************/
/* DIET performance estimation vector                                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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

#include "DIET_data.h"
#include "DIET_server.h"
#include "Vector.h"

estVector_t
new_estVector()
{
  estVector_t ev;
  if ((ev = (estVector_t) new_Vector()) == NULL) {
    fprintf(stderr, "%s: unable to allocate vector\n", __FUNCTION__);
    return (NULL);
  }

  return (ev);
}

typedef struct __s_tagvalpair {
  diet_est_tag_t __tag;
  double         __val;
} __s_tagvalpair, *__tagvalpair_t;

int
estVector_addEstimation(estVector_t ev, diet_est_tag_t tag, double val)
{
  __tagvalpair_t tvp;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (-1);
  }

  if ((tvp = (__tagvalpair_t) calloc(1, sizeof (__s_tagvalpair))) == NULL) {
    fprintf(stderr, "%s: unable to allocate pair\n", __FUNCTION__);
    return (-1);
  }
  tvp->__tag = tag;
  tvp->__val = val;

  Vector_add((Vector_t) ev, tvp);
  return (Vector_size((Vector_t) ev));
}

int
estVector_setEstimation(estVector_t ev, diet_est_tag_t tag, double val)
{
  __tagvalpair_t tvp;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (-1);
  }

  if ((tvp = (__tagvalpair_t) calloc(1, sizeof (__s_tagvalpair))) == NULL) {
    fprintf(stderr, "%s: unable to allocate pair\n", __FUNCTION__);
    return (-1);
  }
  tvp->__tag = tag;
  tvp->__val = val;

  { /* eliminate all instances of this tag */
    int evIter;
    for (evIter = Vector_size((Vector_t) ev) - 1 ; evIter >= 0 ; evIter--) {
      __tagvalpair_t tvpIter;
      tvpIter = (__tagvalpair_t) Vector_elementAt((Vector_t) ev, evIter);
      if (tvpIter->__tag == tag) {
        Vector_removeAtPosition((Vector_t) ev, evIter);
        free(tvpIter);
      }
    }
  }

  Vector_add((Vector_t) ev, tvp);
  return (Vector_size((Vector_t) ev));
}

int
estVector_numEstimations(estVector_t ev)
{
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (-1);
  }
  return (Vector_size((Vector_t) ev));
}

int
estVector_numEstimationsByType(estVector_t ev, diet_est_tag_t tag)
{
  int vIter;
  int found;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (-1);
  }
  for (found = 0, vIter = 0 ; vIter < Vector_size((Vector_t) ev) ; vIter++) {
    __tagvalpair_t tvp =
      (__tagvalpair_t) Vector_elementAt((Vector_t) ev, vIter);
    if (tvp == NULL ||
        tvp->__tag != tag) {
      continue;
    }
    found++;
  }
  return (found);
}

diet_est_tag_t
estVector_getEstimationTagByIdx(estVector_t ev, int idx)
{
  __tagvalpair_t tvp;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (EST_INVALID);
  }
  if ((tvp = (__tagvalpair_t) Vector_elementAt((Vector_t) ev, idx)) == NULL) {
    fprintf(stderr, "%s: null element\n", __FUNCTION__);
    return (EST_INVALID);
  }
  return (tvp->__tag);
}

double
estVector_getEstimationValueByIdx(estVector_t ev, int idx, double errVal)
{
  __tagvalpair_t tvp;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (errVal);
  }
  if ((tvp = (__tagvalpair_t) Vector_elementAt((Vector_t) ev, idx)) == NULL) {
    fprintf(stderr, "%s: null element\n", __FUNCTION__);
    return (errVal);
  }
  return (tvp->__val);
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
  int vIter;
  int found;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return (errVal);
  }
  if (idx < 0) {
    fprintf(stderr, "%s: invalid index\n", __FUNCTION__);
    return (errVal);
  }
  for (found = 0, vIter = 0 ; vIter < Vector_size((Vector_t) ev) ; vIter++) {
    __tagvalpair_t tvp =
      (__tagvalpair_t) Vector_elementAt((Vector_t) ev, vIter);
    if (tvp == NULL ||
        tvp->__tag != tag) {
      continue;
    }

    if (found == idx) {
      return (tvp->__val);
    }
    found++;
  }

/*   fprintf(stderr, "%s: index not found\n", __FUNCTION__); */
  return (errVal);
}

void
free_estVector(estVector_t ev)
{
  __tagvalpair_t tvp;
  if (ev == NULL) {
    fprintf(stderr, "%s: null vector\n", __FUNCTION__);
    return;
  }
  while (! Vector_isEmpty((Vector_t) ev)) {
    tvp = (__tagvalpair_t) Vector_removeAtPosition((Vector_t) ev, 0);
    if (tvp != NULL) {
      free(tvp);
    }
    /* ELSE ERROR? */
  }
  free_Vector((Vector_t) ev);
}
