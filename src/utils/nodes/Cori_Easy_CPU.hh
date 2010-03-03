/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Colleting CPU Information for EASY                                    */
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
 * Revision 1.2  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
 ****************************************************************************/

#ifndef _CORI_EASY_CPU_HH_
#define _CORI_EASY_CPU_HH_ 

#include <vector>
using namespace std;

class Easy_CPU
{
public:
  int 
  get_CPU_Frequence(vector <double> * vlist);
  int 
  get_CPU_Cache(vector <double> * vlist);
  int 
  get_CPU_Bogomips(vector <double> * vlist);
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
  get_CPU_Freq_From_Proc(vector <double> * vlist);
  int 
  get_Bogomips_From_Proc(vector <double> * vlist);
  int 
  get_CPU_Cache_From_Proc(vector <double> * vlist);
  int 
  get_CPU_Freq_for_FreeBSD(vector <double> * vlist);
  int 
  get_CPU_Freq_for_Darwin(vector <double> * vlist);
  int 
  get_CPU_Freq_for_NetBSD(vector <double> * vlist);
  int 
  get_CPU_ActualLoad_Byps(double * actualload);
};
 
#endif //_CORI_EASY_CPU_HH_ 
