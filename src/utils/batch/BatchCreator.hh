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
 * Revision 1.5  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.4  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.3  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.2  2007/04/17 13:34:54  ycaniou
 * Error in debug.tex header
 * Removes some warnings during doc generation
 *
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
    LOADLEVELER,
    SGE,
    NUMBER_OF_SUPPORTED_BATCH
  } batchID_t ;

  static const char * const batchNames[NUMBER_OF_SUPPORTED_BATCH] ;

  BatchCreator() ;

  /** Returns a new inherited instance of the class BatchSystem corresponding
      to the name @param batchName .
      Returns NULL if batch system is not managed.
  */
  static BatchSystem *
  getBatchSystem( const char * batchName ) ;

  /** Returns 1 if the batch system is managed by DIET, 0 otherwise */
  static int
  existBatchScheduler(const char * batchName, batchID_t * batchID) ;
      
} ;

#endif
