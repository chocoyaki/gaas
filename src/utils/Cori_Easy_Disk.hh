/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Colleting Disk Information for EASY                                    */
/*                                                                          */
/* Author(s):                                                               */
/*    - Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)                     */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 ****************************************************************************/

#ifndef _CORI_EASY_DISK_HH_
#define _CORI_EASY_DISK_HH_

#include <iostream>
#include <fstream>	
#include <cstdlib>
using namespace std;
#define FILESIZE_MB 10 //MB
#define KILOBYTE 1024
#define MEGABYTE KILOBYTE*KILOBYTE
#define FILESIZE FILESIZE_MB*MEGABYTE
#define BUFFSIZE 1024
#define COUNTPERBUFFER 20

class Easy_Disk{
private:
  // int sigalarm;
  // void stop_count(int a);
  int 
  create_file(const char* path_file);
  int 
  gatherSizeDisks(int typeOfInfo,
		  double * result, 
		  const char* path);

  int 
  get_Write_Speed_by_gettimeofday(const char* path, 
				  double * result);

  int 
  get_Write_Speed_by_sig_alarm(const char* path, 
			       double * result);
  int 
  get_Read_Speed_by_gettimeofday(const char* path, 
				 double * result);

  int 
  get_Read_Speed_by_sig_alarm(const char* path, 
			      double * result);

public:
  int 
  get_Read_Speed(const char* path, 
		 double * result);
  int 
  get_Write_Speed(const char* path, 
		  double * result);
  int 
  get_Available_DiskSpace(const char* path, 
			  double * result);
  int 
  get_Total_DiskSpace(const char* path, 
		      double * result);
  double 
  search_for_percent(ifstream &file,
		     int typeOfInfo);
  int 
  is_Accessible(const char * path);
};

#endif //_CORI_EASY_DISK_HH_
