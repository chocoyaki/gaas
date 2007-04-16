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

#include <cstring>
#include "OAR1_6BatchSystem.hh"
#include "BatchCreator.hh"

char * BatchCreator::batchNames[NUMBER_OF_SUPPORTED_BATCH] = {
  "oar1.6" 
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
  batchType batchID ;
  
  if( !(existBatchScheduler(batchName,&batchID) ) ) {
    return NULL ;
  }
  
  switch( (int)batchID ) {
      // case SHELL:
      //       batch = new SHELLBatch() ;
      //     case CONDOR:
      //     case DQS:
      //     case LOADLEVELER:
      //     case LSF:
      //     case PBS:
      //     case SGE:
    case OAR1_6:
      return new OAR1_6BatchSystem(OAR1_6,batchNames[OAR1_6]) ;
  default:
    return NULL ;
  }
}

int 
BatchCreator::existBatchScheduler(const char * batchName,
				  batchType * batchID)
{
  *batchID = (batchType)0 ;
  
  while( (*batchID < NUMBER_OF_SUPPORTED_BATCH)
	 && (strncmp(batchName,batchNames[(int)*batchID],4)!=0) )
    /* for the moment, only a check on the 4 first chars is needed */
    batchID++ ;

   if( (int)batchID == NUMBER_OF_SUPPORTED_BATCH ) return 0 ;
   return 1 ; 
}
