/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Collecting Ressource Information with Cori                               */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 *****************************************************************************/
#ifndef _CORI_DATA_EASY_HH_
#define _CORI_DATA_EASY_HH_

#include "DIET_data.h" 
#include "est_internal.hh"

//include here other Info type 
#include "Cori_Easy_CPU.hh"
#include "Cori_Easy_Disk.hh"
#include "Cori_Easy_Memory.hh"


class Cori_Data_Easy{

private :
  Easy_CPU*  cpu;
  Easy_Disk* disk;
  Easy_Memory* memory;

  int 
  convertArray(vector <double> vect,
	       estVector_t * estvect,
	       int typeOfInfo );

  int 
  convertSimple(double value,
		estVector_t * estvect,
		int typeOfInfo );
public :
  Cori_Data_Easy();
  int 
  get_Information(int type_Info,       
		  estVector_t* info,
		  const void * data);
 
};
#endif //CORI_DATA_EASY_HH
