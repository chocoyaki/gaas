/****************************************************************************/
/* Batch System Management with performance prediction: OAR                 */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 *
 ****************************************************************************/

#ifndef _LOADLEVELER_BATCH_SYSTEM_HH_
#define _LOADLEVELER_BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

/* Strategy :
- If mail is given, Diet submits a script which demands the client to
  alway be notified by mail. Maybe we can improve.
*/

class Loadleveler_BatchSystem : public BatchSystem
{
  
public :

  Loadleveler_BatchSystem(int batchID, const char * batchName) ;

  ~Loadleveler_BatchSystem() ;

  /** If job not terminated, ask the batch system for the status of job
      whose ID is @param batchJobID .
      Updates the internal structure.
      Returns NB_STATUS on error, the status otherwise.
  */
  batchJobState
  askBatchJobStatus(int batchJobID) ;
  
  /** If job whose id is @param batchJobID is:
      - not finished, returns 0
      - terminated, returns 1
      - not found, -1
  */
  int
  isBatchJobCompleted(int batchJobID) ;

  /** TODO
   */  
  int
  getNumberOfAvailableComputingResources() ;

private :

  /* Strings used to filter batch job status if possible */
  static const char * statusNames[] ;

} ;

#endif // LOADLEVELER_BATCH_SYSTEM
