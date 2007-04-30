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

//include here other Info type 

class Cori_batch {

private :
//   Easy_CPU*  cpu;
//   Easy_Disk* disk;
//   Easy_Memory* memory;

  int 
  convertArray(vector <double> vect,
	       estVector_t * estvect,
	       int typeOfInfo );

  int 
  convertSimple(double value,
		estVector_t * estvect,
		int typeOfInfo );
public :
  Cori_batch();
  int 
  get_Information(int type_Info,       
		  estVector_t* info,
		  const void * data);
 
};
#endif //CORI_DATA_EASY_HH
