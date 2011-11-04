/**
 * @file BatchCreator.cc
 *
 * @brief  Batch System Management with performance prediction
 *
 * @author  Yves Caniou (yves.caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


// #include <cstring>
#include "OAR1_6BatchSystem.hh"
#include "OAR2_XBatchSystem.hh"
#include "Loadleveler_BatchSystem.hh"
#include "PBS_BatchSystem.hh"
#include "Slurm_BatchSystem.hh"

#ifdef HAVE_CLOUD
#include "Eucalyptus_BatchSystem.hh"
#endif

#include "BatchCreator.hh"

#include <cstring>

#ifdef YC_DEBUG
#include "debug.hh"
#endif

const char *const BatchCreator::batchNames[NUMBER_OF_SUPPORTED_BATCH] = {
  "oar1.6",
  "loadleveler",
  "sge",
  "pbs",
  "oar2.x",
  "slurm",

#ifdef HAVE_CLOUD
  "eucalyptus"
#endif
};

// "shellscript",
// "condor", "dqs", "loadleveler", "lsf", "pbs", "sge",
// "oar1.6"

BatchCreator::BatchCreator() {
}

BatchSystem *
BatchCreator::getBatchSystem(const char *batchName) {
  batchID_t batchID;

  if (!(existBatchScheduler(batchName, &batchID))) {
    return NULL;
  }

  switch ((int) batchID) {
  case OAR1_6:
    return new OAR1_6BatchSystem(batchID, batchNames[batchID]);

  case LOADLEVELER:
    return new Loadleveler_BatchSystem(batchID, batchNames[batchID]);

  case PBS:
    return new PBS_BatchSystem(batchID, batchNames[batchID]);

  case OAR2_X:
    return new OAR2_XBatchSystem(batchID, batchNames[batchID]);

  case SLURM:
    return new Slurm_BatchSystem(batchID, batchNames[batchID]);

#ifdef HAVE_CLOUD
  case EUCALYPTUS:
    return new Eucalyptus_BatchSystem(batchID, batchNames[batchID]);

#endif

  default:
    return NULL;
  } // switch
} // getBatchSystem

int
BatchCreator::existBatchScheduler(const char *batchName,
                                  batchID_t *batchID) {
  int id_tmp = 0;

  while ((id_tmp < NUMBER_OF_SUPPORTED_BATCH)
         && (strncmp(batchName, batchNames[id_tmp], 4) != 0)) {
    /* for the moment, only a check on the 4 first chars is needed */
    id_tmp++;
  }

#ifdef YC_DEBUG
  TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
             << "NUMBER_OF_SUPPORTED_BATCH: " << NUMBER_OF_SUPPORTED_BATCH
             << endl);
  TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
             << "; BatchID: " << id_tmp
             << "; batchName: " << batchName
             << endl);
  TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
             << "; batchNames[batchID]: " << batchNames[id_tmp]
             << "; cmp(): " << strncmp(batchName, batchNames[id_tmp], 4)
             << endl << endl << endl);
#endif // ifdef YC_DEBUG

  *batchID = (batchID_t) id_tmp;

  if (id_tmp == NUMBER_OF_SUPPORTED_BATCH) {
    return 0;
  }
  return 1;
} // existBatchScheduler
