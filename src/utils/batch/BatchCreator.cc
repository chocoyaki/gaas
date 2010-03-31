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
 * Revision 1.8  2010/03/31 21:15:40  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.7  2010/03/31 19:37:55  bdepardo
 * Changed "\n" into std::endl
 *
 * Revision 1.6  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.5  2008/08/19 00:46:37  bdepardo
 * PBS -> pbs for the name of the batch scheduler
 *
 * Revision 1.4  2008/08/17 08:10:19  bdepardo
 * Added PBS batch system
 * /!\ not tested yet
 *
 * Revision 1.3  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.2  2008/01/01 19:43:49  ycaniou
 * Modifications for batch management. Loadleveler is now ok.
 *
 * Revision 1.1  2007/04/16 22:34:39  ycaniou
 * Added the class to create the correct batch system
 *
 *
 ****************************************************************************/

//#include <cstring>
#include "OAR1_6BatchSystem.hh"
#include "OAR2_XBatchSystem.hh"
#include "Loadleveler_BatchSystem.hh"
#include "PBS_BatchSystem.hh"
#include "BatchCreator.hh"

#include <cstring>

#ifdef YC_DEBUG
#include "debug.hh"
#endif

const char * const BatchCreator::batchNames[NUMBER_OF_SUPPORTED_BATCH] = {
  "oar1.6",
  "loadleveler",
  "sge",
  "pbs",
  "oar2.x"
} ;

// "shellscript",
//"condor","dqs","loadleveler","lsf","pbs","sge", 
//      "oar1.6"
  
BatchCreator::BatchCreator()
{
}

BatchSystem *
BatchCreator::getBatchSystem( const char * batchName )
{
  batchID_t batchID ;
  
  if( !(existBatchScheduler(batchName,&batchID) ) ) {
    return NULL ;
  }

  switch( (int)batchID ) {
  case OAR1_6:
    return new OAR1_6BatchSystem(batchID,batchNames[batchID]) ;
  case LOADLEVELER:
    return new Loadleveler_BatchSystem(batchID,batchNames[batchID]) ;
  case PBS:
    return new PBS_BatchSystem(batchID,batchNames[batchID]) ;
  case OAR2_X:
    return new OAR2_XBatchSystem(batchID,batchNames[batchID]) ;
  default:
    return NULL ;
  }
}

int 
BatchCreator::existBatchScheduler(const char * batchName,
				  batchID_t * batchID)
{
  int id_tmp = 0 ;
    
  while( (id_tmp < NUMBER_OF_SUPPORTED_BATCH)
	 && (strncmp(batchName,batchNames[id_tmp],4)!=0) )
    /* for the moment, only a check on the 4 first chars is needed */
    id_tmp++ ;

#ifdef YC_DEBUG
    TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
	       << "NUMBER_OF_SUPPORTED_BATCH: " << NUMBER_OF_SUPPORTED_BATCH
	       << endl) ;
    TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
	       << " ; BatchID: " << id_tmp
	       << " ; batchName: " << batchName
	       << endl) ;
    TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
	       << " ; batchNames[batchID]: " << batchNames[id_tmp]
	       << " ; cmp(): " << strncmp(batchName,batchNames[id_tmp],4)
	       << endl << endl << endl) ;
#endif

    *batchID = (batchID_t)id_tmp ;
    
   if( id_tmp == NUMBER_OF_SUPPORTED_BATCH ) return 0 ;
   return 1 ; 
}
