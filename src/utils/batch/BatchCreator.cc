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
#include "Loadleveler_BatchSystem.hh"
#include "BatchCreator.hh"

#ifdef YC_DEBUG
#include "debug.hh"
#endif

char * BatchCreator::batchNames[NUMBER_OF_SUPPORTED_BATCH] = {
  "oar1.6",
  "loadleveler",
  "sge"
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
    // case SHELL:
    //       batch = new SHELLBatch() ;
    //     case CONDOR:
    //     case DQS:
    //     case LSF:
    //     case PBS:
    //     case SGE:
  case OAR1_6:
    return new OAR1_6BatchSystem(batchID,batchNames[batchID]) ;
  case LOADLEVELER:
    return new Loadleveler_BatchSystem(batchID,batchNames[batchID]) ;
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
	       << "\n") ;
    TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
	       << " ; BatchID: " << id_tmp
	       << " ; batchName: " << batchName
	       << "\n") ;
    TRACE_TEXT(TRACE_MAIN_STEPS, "YC_DEBUG: "
	       << " ; batchNames[batchID]: " << batchNames[id_tmp]
	       << " ; cmp(): " << strncmp(batchName,batchNames[id_tmp],4)
	       << "\n\n\n") ;
#endif

    *batchID = (batchID_t)id_tmp ;
    
   if( id_tmp == NUMBER_OF_SUPPORTED_BATCH ) return 0 ;
   return 1 ; 
}
