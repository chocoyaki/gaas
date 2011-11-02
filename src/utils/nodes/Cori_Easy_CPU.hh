/**
* @file Cori_Easy_CPU.hh
* 
* @brief  CoRI Colleting CPU Information for EASY  
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

#ifndef _CORI_EASY_CPU_HH_
#define _CORI_EASY_CPU_HH_

#include <vector>

class Easy_CPU {
public:
  int
  get_CPU_Frequence(std::vector<double> * vlist);

  int
  get_CPU_Cache(std::vector<double> * vlist);

  int
  get_CPU_Bogomips(std::vector<double> * vlist);

  int
  get_CPU_Number(double * number);

  int
  get_CPU_ActualLoad(double * actualload);

  int
  get_CPU_Avg(int interval, double * resultat);

private:
  int
  get_CPU_Avg_byGetloadavg(int interval, double * resultat);

  int
  get_CPU_Number_byget_nprocs(double * result);

  int
  get_CPU_Number_byNum_Proc(double * result);

  int
  get_CPU_Freq_From_Proc(std::vector<double> * vlist);

  int
  get_Bogomips_From_Proc(std::vector<double> * vlist);

  int
  get_CPU_Cache_From_Proc(std::vector<double> * vlist);

  int
  get_CPU_Freq_for_FreeBSD(std::vector<double> * vlist);

  int
  get_CPU_Freq_for_Darwin(std::vector<double> * vlist);

  int
  get_CPU_Freq_for_NetBSD(std::vector<double> * vlist);

  int
  get_CPU_ActualLoad_Byps(double * actualload);
};

#endif  //_CORI_EASY_CPU_HH_
