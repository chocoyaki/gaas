/****************************************************************************/
/* DIET performance estimation vector                                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2005/05/15 15:42:41  alsu
 * - minor changes to the interfaces
 * - hiding most of the useless estimation vector stuff from the server
 *   interface
 *
 */
#ifndef _ESTVECTOR_H_
#define _ESTVECTOR_H_

#include "DIET_data.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct estVector_s *estVector_t;

/****************************************************************************/
/* DIET estimation vector interface                                         */
/****************************************************************************/
estVector_t new_estVector();
int estVector_addEstimation(estVector_t ev, int tag, double val);
int estVector_setEstimation(estVector_t ev, int tag, double val);
int estVector_numEstimationsByTag(estVector_t ev, int tag);
int estVector_numEstimations(estVector_t ev);
int estVector_getEstimationTagByIdx(estVector_t ev, int idx);
double estVector_getEstimationValueByIdx(estVector_t ev,
                                         int idx,
                                         double errVal);
double estVector_getEstimationValue(estVector_t ev, int tag, double errVal);
double estVector_getEstimationValueNum(estVector_t ev,
                                       int tag,
                                       double errVal,
                                       int idx);
void free_estVector(estVector_t ev);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ESTVECTOR_H_ */
