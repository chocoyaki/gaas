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
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.4  2007/04/30 13:53:22  ycaniou
 * Cosmetic changes (indentation) and small changes for Cori_Batch
 *
 * Revision 1.3  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 *****************************************************************************/
#ifndef _CORI_DATA_EASY_HH_
#define _CORI_DATA_EASY_HH_

#include <vector>

#include "DIET_data.h"
#include "est_internal.hh"

// include here other Info type
#include "Cori_Easy_CPU.hh"
#include "Cori_Easy_Disk.hh"
#include "Cori_Easy_Memory.hh"


class Cori_Data_Easy {
public:
  Cori_Data_Easy();

  int
  get_Information(int type_Info, estVector_t* info, const void * data);

private:
  Easy_CPU*  cpu;
  Easy_Disk* disk;
  Easy_Memory* memory;

  int
  convertArray(std::vector<double> vect, estVector_t * estvect,
               int typeOfInfo);

  int
  convertSimple(double value, estVector_t * estvect, int typeOfInfo);
};

#endif  // CORI_DATA_EASY_HH
