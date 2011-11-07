/**
 * @file Cori_batch.hh
 *
 * @brief  CoRI Collectors of ressource information for batch systems
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _CORI_BATCH_HH_
#define _CORI_BATCH_HH_

#include <vector>

#include "DIET_data.h"
#include "est_internal.hh"
#include "SeDImpl.hh"

class SeDImpl;
class BatchSystem;

class Cori_batch {
public:
  explicit
  Cori_batch(diet_profile_t *profile);

  /** Print the values defined for the metric \c type_Info stored in the
      estimation vector \c vector_v */
  void
  printMetric(estVector_t vector_v, int type_Info);

  /* TODO: Change the name of this prototype! We don't get anything!
     This place in the vector the information! */
  int
  get_Information(int type_Info, estVector_t *info, const void *data);
private:
  SeDImpl *SeD;
  BatchSystem *batch;

  int
  convertArray(std::vector<double> vect, estVector_t *estvect,
               int typeOfInfo);

  int
  convertSimple(double value, estVector_t *estvect, int typeOfInfo);
};
#endif  // CORI_BATCH_HH
