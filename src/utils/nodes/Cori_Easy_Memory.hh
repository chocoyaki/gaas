/**
* @file Cori_Easy_Memory.hh
* 
* @brief  CoRI Colleting Memory Information for EASY   
* 
* @author  - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.1  2010/03/03 14:26:35  bdepardo
 * BEWARE!!!
 * Huge modifications to take into account CYGWIN.
 * Lots of files' directory have been changed.
 *
 * Revision 1.2  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 ****************************************************************************/


#ifndef _CORI_EASY_MEMORY_HH_
#define _CORI_EASY_MEMORY_HH_

class Easy_Memory {
public:
  int
  get_Total_Memory(double * result);

  int
  get_Avail_Memory(double * result);

private:
  int
  get_Avail_Memory_byvmstat(double * result);

  int
  get_Avail_Memory_bysysinfo(double * result);

  int
  get_Total_Memory_bysysinfo(double * result);

  int
  get_Info_Memory_byProcMem(double* resultat, int freemem);
};
#endif  // _CORI_EASY_MEMORY_HH_
