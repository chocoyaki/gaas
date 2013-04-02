/**
 * @file  est_internal.h
 *
 * @brief  Plug-in scheduler: DIET-internal estimation vector access functions
 *
 * @author   Alan Su (Alan.Su@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef OLD_EV
#ifndef _EST_INTERNAL_HH_
#define _EST_INTERNAL_HH_

#include "DIET_data.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*
** Set of #define's for future "rules" for the scalar
** values of estimation values.  The first
** (EST_IDX_SCALAR) denotes a scalar value that does
** not have a rule associated with it.
**
** (All new rule values should be negative, to avoid
** colliding with the values for valid array values!)
*/
#define EST_IDX_SCALAR -1

int
diet_est_set_internal(estVector_t ev, int tag, double value);

double
diet_est_get_internal(estVectorConst_t ev, int tag, double errVal);

int
diet_est_defined_internal(estVectorConst_t ev, int tag);

int
diet_est_array_size_internal(estVectorConst_t ev, int tag);

int
diet_est_array_set_internal(estVector_t ev, int tag,
                            int idx, double value);

double
diet_est_array_get_internal(estVectorConst_t ev, int tag,
                            int idx, double errVal);

int
diet_est_array_defined_internal(estVectorConst_t ev, int tag, int idx);

int
diet_est_set_str_internal(estVector_t ev, int tag, const char *value);

const char*
diet_est_get_str_internal(estVectorConst_t ev, int tag);

int
diet_est_set_bin_internal(estVector_t ev, int tag, const unsigned char *buf, size_t size);

int
diet_est_get_bin_internal(estVectorConst_t ev, int tag, const unsigned char **buf, size_t *size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* _EST_INTERNAL_HH_ */
#endif  /* OLD_EV */
