/**
 * @file Cori_Easy_Disk.hh
 *
 * @brief  CoRI: Collectors for disk ressource
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#ifndef _CORI_EASY_DISK_HH_
#define _CORI_EASY_DISK_HH_

#include <cstdlib>
#include <iostream>
#include <fstream>

#define FILESIZE_MB 10  // MB
#define KILOBYTE 1024
#define MEGABYTE KILOBYTE * KILOBYTE
#define FILESIZE FILESIZE_MB * MEGABYTE
#define BUFFSIZE 1024
#define COUNTPERBUFFER 20

class Easy_Disk {
public:
  int
  get_Read_Speed(const char *path, double *result);

  int
  get_Write_Speed(const char *path, double *result);

  int
  get_Available_DiskSpace(const char *path, double *result);

  int
  get_Total_DiskSpace(const char *path, double *result);

private:
  int
  create_file(char **path_file);

  int
  gatherSizeDisks(int typeOfInfo, double *result, const char *path);

  int
  get_Write_Speed_by_gettimeofday(const char *path, double *result);

  int
  get_Write_Speed_by_sig_alarm(const char *path, double *result);

  int
  get_Read_Speed_by_gettimeofday(const char *path, double *result);

  int
  get_Read_Speed_by_sig_alarm(const char *path, double *result);

  int
  openfile(char **path, std::ofstream *outfile);

  double
  search_for_percent(FILE *file, int typeOfInfo);
};

#endif  // _CORI_EASY_DISK_HH_
