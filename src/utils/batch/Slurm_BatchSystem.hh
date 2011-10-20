/****************************************************************************/
/* Batch System Management with performance prediction: Slurm               */
/*                                                                          */
/* Author(s):                                                               */
/*    - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 ****************************************************************************/

#ifndef _SLURM_BATCH_SYSTEM_HH_
#define _SLURM_BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

class Slurm_BatchSystem : public BatchSystem {
public:
  Slurm_BatchSystem(int batchID, const char * batchName);

  ~Slurm_BatchSystem();

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
  static const char * statusNames[];
};

#endif  // SLURM_BATCH_SYSTEM
