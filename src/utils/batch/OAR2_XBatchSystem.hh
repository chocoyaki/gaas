/**
 * @file OAR2_XBatchSystem.hh
 *
 * @brief  Batch System Management with performance prediction: OAR v2.x
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _OAR2_XBATCH_SYSTEM_HH_
#define _OAR2_XBATCH_SYSTEM_HH_

#include "BatchSystem.hh"

class OAR2_XBatchSystem : public BatchSystem {
public:
  OAR2_XBatchSystem(int batchID, const char *batchName);

  ~OAR2_XBatchSystem();

  /** If job not terminated, ask the batch system for the status of job
      whose ID is @param batchJobID .
      Updates the internal structure.
      Returns NB_STATUS on error, the status otherwise.
   */
  batchJobState
  askBatchJobStatus(int batchJobID);

  /** If job whose id is @param batchJobID is:
      - not finished, returns 0
      - terminated, returns 1
      - not found, -1
   */
  int
  isBatchJobCompleted(int batchJobID);

  /********** Batch static information accessing Functions **********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  int
  getNbTotResources();

  int
  getNbResources();

  char *
  getResourcesName();

  int
  getMaxWalltime();

  int
  getMaxProcs();

  /********** Batch dynamic information accessing Functions *********/
  /* These should soon change for they assume a default queue and we
     want to be able to manage all queues of a system! */

  int
  getNbTotFreeResources();

  int
  getNbFreeResources();

private:
  /* Strings used to filter batch job status if possible */
  static const char *const statusNames[];
};

#endif  // OAR_BATCH_SYSTEM
