/****************************************************************************/
/* CoRI Collectors of ressource information for batch systems               */
/*                                                                          */
/* Author(s):                                                               */
/*    - Yves Caniou (yves.caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.3  2008/04/07 12:19:12  ycaniou
 * Except for the class Parsers (someone to re-code it? :)
 *   correct "deprecated conversion from string constant to 'char*'" warnings
 *
 * Revision 1.2  2007/04/30 13:55:18  ycaniou
 * Removed compilation warnings by adding Cori_Batch in the lib and modifs
 *   in files
 *
 * Revision 1.1  2007/04/16 22:37:20  ycaniou
 * Added the class to make perf prediction with batch systems.
 * First draw. Not operational for the moment.
 ****************************************************************************/

#include "Cori_batch.hh"
#include "BatchSystem.hh"

#include "debug.hh"
#include <iostream>
#include <fstream>	/*file reading*/
#include <cstdlib>
#include <string>
#include <math.h>

#define className Cori_batch

using namespace std ;

void
className::printMetric( estVector_t vector_v, int type_Info )
{
  switch( type_Info ){
  
  case EST_SERVER_TYPE: /* Value appears only once */
    cout << "In construction: EST_SERVER_TYPE\n\n" ;
    break;
  case EST_PARAL_NBTOT_RESOURCES:
    cout << "In construction: EST_PARAL_NBTOT_RESOURCES\n\n" ;
    break;
  case EST_PARAL_NBTOT_FREE_RESOURCES:
    cout << "In construction: EST_PARAL_NBTOT_FREE_RESOURCES\n\n" ;
    break;
  case EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE:
    cout << 
      "CoRI: EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE... " <<
      (int)diet_est_get_system(vector_v,
			       EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE, 0)
	 << "\n\n" ;
    break;
  default: {
  INTERNAL_WARNING( "CoRI: Tag " << type_Info <<" for printing info");
  }	
  }
}

Cori_batch::Cori_batch( diet_profile_t * profile )
{
  /* At this time, because diet_SeD() has not yet been called, we cannot do
     that */
  /* TODO: make a diet_initialize() or something before diet_SeD()! */

  /* this->SeD = (SeDImpl*)(profile->SeDPtr) ;
     this->batch = ((SeDImpl*)(profile->SeDPtr))->getBatch() ; */
}

int
Cori_batch::get_Information(int type_Info,       
			    estVector_t * estvect,
			    const void * data)
{
  switch( type_Info ) {
  /* The following one should be replaced, when SeDseq, SeDpar will be
     implemented, by est_PARAL_ID */
  case EST_SERVER_TYPE:
    /*    diet_est_set_internal( *estvect, type_Info, this->SeD->getServerStatus()) ; */
    diet_est_set_internal( *estvect, type_Info,
			   (( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))->getServerStatus()) ;
    break ;
  case EST_PARAL_NBTOT_RESOURCES:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getNbTotResources() ) ;
    /*			   this->batch->getNbTotResources() ) ; */
    break ;
  case EST_PARAL_NBTOT_FREE_RESOURCES:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getNbTotFreeResources() ) ;
    /*			   this->batch->getNbTotFreeResources() ) ; */
    break ;
  /* Information concerning the default queue now */
  case EST_PARAL_NB_RESOURCES_IN_DEFAULT_QUEUE:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getNbResources() ) ;
    /*			   this->batch->getNbResources() ) ; */
    break ;
  case EST_PARAL_NB_FREE_RESOURCES_IN_DEFAULT_QUEUE:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getNbFreeResources() ) ;
    /*			   this->batch->getNbFreeResources() ) ; */
    break ;
  case EST_PARAL_MAX_WALLTIME:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getMaxWalltime() ) ;
    /*			   this->batch->getMaxWalltime() ) ; */
    break ;
  case EST_PARAL_MAX_PROCS:
    diet_est_set_internal( *estvect, type_Info,
			   ((( SeDImpl*)(((diet_profile_t*)data)->SeDPtr))
			    ->getBatch())->getMaxProcs() ) ;
    /*			   this->batch->getMaxProcs() ) ; */
    break ;
  default:
    {
      
    WARNING("CoRI Batch: Tag " << type_Info << 
	    " unknown for collecting info") ;
    }
  }
  
  if (TRACE_LEVEL>=TRACE_ALL_STEPS)
    printMetric( *estvect, type_Info ) ; 
  return 0 ;
}

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

int  
Cori_batch::convertArray(vector <double> vect,
			     estVector_t * estvect,
			     int typeOfInfo )
{
   /*  vector<double>::iterator iter1;
  iter1 = vect.begin();
  int i=0;
  while( iter1 != vect.end()) {
    diet_est_array_set_internal(*estvect,typeOfInfo,i,*iter1);
    i++; 
    iter1++;
  }
  return 0;
   */
  return 0 ;
}

int  
Cori_batch::convertSimple(double value,
			      estVector_t * estvect,
			      int typeOfInfo ){
  diet_est_set_internal(*estvect,typeOfInfo,value);
  return 0;
}


