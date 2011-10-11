/****************************************************************************/
/* CoRI Collectors of ressource information                                 */
/* Colleting Disk Information for EASY                                    */
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
 * Revision 1.4  2006/10/31 23:18:33  ecaron
 * Correct wrong header
 *
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
  
private:
  // int sigalarm;
  // void stop_count(int a);
  int 
  create_file(char** path_file);
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

  int 
  openfile(char ** path,
           ofstream* outfile);
 
  double 
  search_for_percent(FILE * file,
                     int typeOfInfo);

};

#endif  //_CORI_EASY_DISK_HH_
