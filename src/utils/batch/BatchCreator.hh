/****************************************************************************/
/* Batch System Management with performance prediction                      */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2007/04/16 22:34:39  ycaniou
 * Added the class to create the correct batch system
 *
 *
 ****************************************************************************/

#ifndef _BATCH_CREATOR_HH_
#define _BATCH_CREATOR_HH_

class BatchSystem ;

class BatchCreator
{

public :

  typedef enum {
    // SHELL, 
    //CONDOR, DQS, LOADLEVELER, LSF, PBS, SGE, 
    OAR1_6, 
    NUMBER_OF_SUPPORTED_BATCH
  } batchType ;

  static char * batchNames[NUMBER_OF_SUPPORTED_BATCH] ;

  BatchCreator() ;

  /** Returns a new inherited instance of the class BatchSystem corresponding
      to the name @param batchName.
      Returns NULL if batch system is not managed.
  */
  static BatchSystem *
  getBatchSystem( const char * batchName ) ;

  /** Returns 1 if the batch system is managed by DIET, o otherwise */
  static int
  existBatchScheduler(const char * batchName, batchType * batchID) ;
      
} ;

#endif
