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
 * Revision 1.2  2008/04/07 13:11:44  ycaniou
 * Correct "deprecated conversion from string constant to 'char*'" warnings
 * First attempt to code functions to dynamicaly get batch information
 * 	(e.g.,  getNbMaxResources(), etc.)
 *
 * Revision 1.1  2008/01/04 12:49:46  ycaniou
 * Added those two files, but there is no glue for SGE at the moment
 *
 *
 ****************************************************************************/

#ifndef _SGE_BATCH_SYSTEM_HH_
#define _SGE_BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

/* Strategy :
- If mail is given, Diet submits a script which demands the client to
  alway be notified by mail. Maybe we can improve.
*/

class SGE_BatchSystem : public BatchSystem
{
  
public :

  SGE_BatchSystem(int batchID, const char * batchName) ;

  ~SGE_BatchSystem() ;

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
  static const char * const statusNames[] ;

} ;

#endif // LOADLEVELER_BATCH_SYSTEM
