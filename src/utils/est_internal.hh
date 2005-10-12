#ifndef OLD_EV
#ifndef _EST_INTERNAL_HH_
#define _EST_INTERNAL_HH_

#include "DIET_data.h"

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

int diet_est_set_internal(estVector_t ev, int tag, double value);
double diet_est_get_internal(estVectorConst_t ev, int tag, double errVal);
int diet_est_defined_internal(estVectorConst_t ev, int tag);
int diet_est_array_size_internal(estVectorConst_t ev, int tag);
int diet_est_array_set_internal(estVector_t ev,
                                int tag,
                                int idx,
                                double value);
double diet_est_array_get_internal(estVectorConst_t ev,
                                   int tag,
                                   int idx,
                                   double errVal);
int diet_est_array_defined_internal(estVectorConst_t ev, int tag, int idx);

#endif /* _EST_INTERNAL_HH_ */
#endif /* OLD_EV */
