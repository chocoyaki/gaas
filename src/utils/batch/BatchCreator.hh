/**
 * @file BatchCreator.hh
 *
 * @brief  Batch System Management with performance prediction
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _BATCH_CREATOR_HH_
#define _BATCH_CREATOR_HH_

class BatchSystem;

class BatchCreator {
public:
  typedef enum {
    OAR1_6,
    LOADLEVELER,
    SGE,
    PBS,
    OAR2_X,
    SLURM,
#ifdef HAVE_CLOUD
    EUCALYPTUS,
#endif
    NUMBER_OF_SUPPORTED_BATCH
  } batchID_t;

  static const char *const batchNames[NUMBER_OF_SUPPORTED_BATCH];

  BatchCreator();

  /** Returns a new inherited instance of the class BatchSystem corresponding
      to the name @param batchName .
      Returns NULL if batch system is not managed.
   */
  static BatchSystem *
  getBatchSystem(const char *batchName);

  /** Returns 1 if the batch system is managed by DIET, 0 otherwise */
  static int
  existBatchScheduler(const char *batchName, batchID_t *batchID);
};

#endif /* ifndef _BATCH_CREATOR_HH_ */
