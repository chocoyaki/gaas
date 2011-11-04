/**
 * @file PBS_BatchSystem.hh
 *
 * @brief  Batch System Management with performance prediction: PBS
 *
 * @author  Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _PBS_BATCH_SYSTEM_HH_
#define _PBS_BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

/* Strategy :
   - If mail is given, Diet submits a script which demands the client to
   alway be notified by mail. Maybe we can improve.
 */

class PBS_BatchSystem : public BatchSystem {
public:
  PBS_BatchSystem(int batchID, const char *batchName);

  ~PBS_BatchSystem();

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

  const char *
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
  /* Strings used to filter batch job status if possible */
  static const char *statusNames[];
};

#endif  // PBS_BATCH_SYSTEM
