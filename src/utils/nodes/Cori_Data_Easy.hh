/**
 * @file Cori_Data_Easy.hh
 *
 * @brief  CoRI Collectors of ressource information
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _CORI_DATA_EASY_HH_
#define _CORI_DATA_EASY_HH_

#include <vector>

#include "DIET_data.h"
#include "est_internal.h"

// include here other Info type
#include "Cori_Easy_CPU.hh"
#include "Cori_Easy_Disk.hh"
#include "Cori_Easy_Memory.hh"


class Cori_Data_Easy {
public:
  Cori_Data_Easy();

  int
  get_Information(int type_Info, estVector_t *info, const void *data);

private:
  Easy_CPU *cpu;
  Easy_Disk *disk;
  Easy_Memory *memory;

  int
  convertArray(std::vector<double> vect, estVector_t *estvect,
               int typeOfInfo);

  int
  convertSimple(double value, estVector_t *estvect, int typeOfInfo);
};

#endif  // CORI_DATA_EASY_HH
