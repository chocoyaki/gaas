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
 * Revision 1.3  2008/05/11 16:19:51  ycaniou
 * Check that pathToTmp and pathToNFS exist
 * Check and eventually correct if pathToTmp or pathToNFS finish or not by '/'
 * Rewrite of the propagation of the request concerning job parallel_flag
 * Implementation of Cori_batch system
 * Numerous information can be dynamically retrieved through batch systems
 *
 * Revision 1.2  2007/04/30 13:55:18  ycaniou
 * Removed compilation warnings by adding Cori_Batch in the lib and modifs
 *   in files
 *
 * Revision 1.1  2007/04/16 22:37:20  ycaniou
 * Added the class to make perf prediction with batch systems.
 * First draw. Not operational for the moment.
 *
 *
 *****************************************************************************/
#ifndef _CORI_BATCH_HH_
#define _CORI_BATCH_HH_

#include <vector>
using namespace std;

#include "DIET_data.h" 
#include "est_internal.hh"
#include "SeDImpl.hh"

class SeDImpl ;
class BatchSystem ;

class Cori_batch 
{

private :

  SeDImpl     * SeD ;
  BatchSystem * batch ;

  int 
  convertArray(vector <double> vect,
               estVector_t * estvect,
               int typeOfInfo );

  int 
  convertSimple(double value,
                estVector_t * estvect,
                int typeOfInfo );
public :
  Cori_batch( diet_profile_t* );

  /** Print the values defined for the metric \c type_Info stored in the
      estimation vector \c vector_v */
  void
  printMetric( estVector_t vector_v, int type_Info ) ;
  
  /* TODO: Change the name of this prototype! We don't get anything!
     This place in the vector the information! */
  int
  get_Information(int type_Info,       
                  estVector_t * info,
                  const void * data);
 
};
#endif //CORI_BATCH_HH
