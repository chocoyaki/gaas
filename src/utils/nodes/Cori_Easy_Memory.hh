/**
 * @file Cori_Easy_Memory.hh
 *
 * @brief  CoRI Colleting Memory Information for EASY
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */



#ifndef _CORI_EASY_MEMORY_HH_
#define _CORI_EASY_MEMORY_HH_

class Easy_Memory {
public:
  int
  get_Total_Memory(double *result);

  int
  get_Avail_Memory(double *result);

private:
  int
  get_Avail_Memory_byvmstat(double *result);

  int
  get_Avail_Memory_bysysinfo(double *result);

  int
  get_Total_Memory_bysysinfo(double *result);

  int
  get_Info_Memory_byProcMem(double *resultat, int freemem);
};
#endif  // _CORI_EASY_MEMORY_HH_
