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
 * Revision 1.2  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
 * Revision 1.1  2007/04/16 22:35:55  ycaniou
 * Added the class for DIET to manage OAR v1.6
 *
 *
 ****************************************************************************/

#ifndef _OAR1_6BATCH_SYSTEM_HH_
#define _OAR1_6BATCH_SYSTEM_HH_

#include "BatchSystem.hh"

class OAR1_6BatchSystem : public BatchSystem
{
  
public :

  OAR1_6BatchSystem(int batchID, const char * batchName) ;

  ~OAR1_6BatchSystem() ;

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

#endif // OAR_BATCH_SYSTEM