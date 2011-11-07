/**
 * @file OAR1_6BatchSystem.hh
 *
 * @brief  Batch System Management with performance prediction: OAR
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _OAR1_6BATCH_SYSTEM_HH_
#define _OAR1_6BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

class OAR1_6BatchSystem : public BatchSystem {
public:
  OAR1_6BatchSystem(int batchID, const char *batchName);

  ~OAR1_6BatchSystem();

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

  /****************** Performance Prediction Functions ***************/

private:
  /* Dirty Trick for OAR1.6 to get information on default queue */
  const char *internQueueName;

  /* Strings used to filter batch job status if possible */
  static const char *const statusNames[];
};

#endif  // OAR_BATCH_SYSTEM
