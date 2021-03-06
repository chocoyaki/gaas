/**
 * @file Cori_Data_Easy.cc
 *
 * @brief  CoRI Collectors of ressource information (Data part)
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include "Cori_Data_Easy.hh"

#include "debug.hh"
#include <iostream>
#include <fstream>      /*file reading*/
#include <cstdlib>
#include <string>
#include <cmath>


using namespace std;

void
print_Metric(estVector_t vector_v, int type_Info) {
  double errorCode = 0;
  switch (type_Info) {
  case EST_ALLINFOS:
    break;
  case EST_CPUSPEED:
    for (int i = 0; i < diet_est_array_size_internal(vector_v, EST_CPUSPEED);
         i++)
      cout << "CPU " << i << " frequence : " << diet_est_array_get_internal(
        vector_v, EST_CPUSPEED, i, errorCode) << " Mhz" << endl;
    break;
  case EST_CACHECPU:
    for (int i = 0; i < diet_est_array_size_internal(vector_v, EST_CACHECPU);
         i++)
      cout << "CPU " << i << " cache : " << diet_est_array_get_internal(
        vector_v, EST_CACHECPU, i, errorCode) << " Kb" << endl;
    break;
  case EST_BOGOMIPS:
    for (int i = 0; i < diet_est_array_size_internal(vector_v, EST_BOGOMIPS);
         i++)
      cout << "CPU " << i << " Bogomips : " << diet_est_array_get_internal(
        vector_v, EST_BOGOMIPS, i, errorCode) << endl;
    break;
  case EST_AVGFREECPU:
    cout << "cpu average load : " <<
    diet_est_get_internal(vector_v, EST_AVGFREECPU,
                          errorCode) << endl;
    break;
  case EST_NBCPU:
    cout << "number of processors : " <<
    diet_est_get_internal(vector_v, EST_NBCPU,
                          errorCode) << endl;
    break;
  case EST_DISKACCESREAD:
    cout << "diskspeed in reading : " << diet_est_get_internal(
      vector_v, EST_DISKACCESREAD, errorCode) << " Mbyte/s" << endl;
    break;
  case EST_DISKACCESWRITE:
    cout << "diskspeed in writing : " << diet_est_get_internal(
      vector_v, EST_DISKACCESWRITE, errorCode) << " Mbyte/s" << endl;
    break;
  case EST_TOTALSIZEDISK:
    cout << "total disk size : " <<
    diet_est_get_internal(vector_v, EST_TOTALSIZEDISK,
                          errorCode) << " Mb" << endl;
    break;
  case EST_FREESIZEDISK:
    cout << "available disk size  :" <<
    diet_est_get_internal(vector_v, EST_FREESIZEDISK,
                          errorCode) << " Mb" << endl;
    break;
  case EST_TOTALMEM:
    cout << "total memory : " << diet_est_get_internal(vector_v, EST_TOTALMEM,
                                                       errorCode) << " Mb" <<
    endl;
    break;
  case EST_FREEMEM:
    cout << "available memory : " <<
    diet_est_get_internal(vector_v, EST_FREEMEM,
                          errorCode) << " Mb" << endl;
    break;
  case EST_FREECPU:
    cout << "free cpu: " << diet_est_get_internal(vector_v, EST_FREECPU,
                                                  errorCode) << endl;
    break;
  default: {
    INTERNAL_WARNING("CoRI: Tag " << type_Info << " for printing info");
  }
  } // switch
} // print_Metric
Cori_Data_Easy::Cori_Data_Easy() {
  // cpu = new Cori_Easy_CPU();
  // memory = new Cori_Easy_Memory();
  // disk = new Cori_Easy_Disk();
}

int
Cori_Data_Easy::get_Information(int type_Info,
                                estVector_t *info,
                                const void *data) {
  const char *path;
  vector<double> vect;
  int res = 0;
  double temp = 0;
  int minut;
  const char *tmp = "./";
  switch (type_Info) {
  case EST_ALLINFOS: {
    minut = 15;
    res = get_Information(EST_CPUSPEED, info, NULL) || res;
    res = get_Information(EST_AVGFREECPU, info, &minut) || res;
    res = get_Information(EST_CACHECPU, info, NULL) || res;
    res = get_Information(EST_NBCPU, info, NULL) || res;
    res = get_Information(EST_BOGOMIPS, info, NULL) || res;
    res = get_Information(EST_DISKACCESREAD, info, tmp) || res;
    res = get_Information(EST_DISKACCESWRITE, info, tmp) || res;
    res = get_Information(EST_TOTALSIZEDISK, info, tmp) || res;
    res = get_Information(EST_FREESIZEDISK, info, tmp) || res;
    res = get_Information(EST_TOTALMEM, info, NULL) || res;
    res = get_Information(EST_FREEMEM, info, NULL) || res;
    // set all other TAGS to bad values
    diet_est_set_internal(*info, EST_TCOMP, HUGE_VAL);
    break;
  }
  case EST_CPUSPEED:
    res = cpu->get_CPU_Frequence(&vect);
    convertArray(vect, info, type_Info);
    break;
  case EST_AVGFREECPU:
    if (data == NULL) {
      minut = 15;
    } else {
      minut = *((int *) data);
    }
    res = cpu->get_CPU_Avg(minut, &temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_CACHECPU:
    res = cpu->get_CPU_Cache(&vect);
    convertArray(vect, info, type_Info);
    break;
  case EST_NBCPU:
    res = cpu->get_CPU_Number(&temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_BOGOMIPS:
    res = cpu->get_CPU_Bogomips(&vect);
    convertArray(vect, info, type_Info);
    break;
  case EST_DISKACCESREAD:
    if (data == NULL) {
      path = "./";
    } else {
      path = (char *) data;
    }
    res = disk->get_Read_Speed(path, &temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_DISKACCESWRITE:
    if (data == NULL) {
      path = "./";
    } else {
      path = (char *) data;
    }
    res = disk->get_Write_Speed(path, &temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_TOTALSIZEDISK:
    if (data == NULL) {
      path = "./";
    } else {
      path = (char *) data;
    }
    res = disk->get_Total_DiskSpace(path, &temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_FREESIZEDISK:
    if (data == NULL) {
      path = "./";
    } else {
      path = (char *) data;
    }
    res = disk->get_Available_DiskSpace(path, &temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_TOTALMEM:
    res = memory->get_Total_Memory(&temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_FREEMEM:
    res = memory->get_Avail_Memory(&temp);
    convertSimple(temp, info, type_Info);
    break;
  case EST_FREECPU:
    double nb;
    res = cpu->get_CPU_Number(&nb);
    res = cpu->get_CPU_ActualLoad(&temp);
    temp = 1 - (temp / nb);
    convertSimple(temp, info, type_Info);
    break;
  default: {
    INTERNAL_WARNING(
      "CoRI: Tag " << type_Info << " unknown for collecting info");
    res = 1;
  }
  } // switch
  if (TRACE_LEVEL >= TRACE_ALL_STEPS) {
    print_Metric(*info, type_Info);
  }
  return res;
} // get_Information

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/


int
Cori_Data_Easy::convertArray(vector <double> vect,
                             estVector_t *estvect,
                             int typeOfInfo) {
  vector<double>::iterator iter1;
  iter1 = vect.begin();
  int i = 0;
  while (iter1 != vect.end()) {
    diet_est_array_set_internal(*estvect, typeOfInfo, i, *iter1);
    i++;
    ++iter1;
  }
  return 0;
} // convertArray

int
Cori_Data_Easy::convertSimple(double value,
                              estVector_t *estvect,
                              int typeOfInfo) {
  diet_est_set_internal(*estvect, typeOfInfo, value);
  return 0;
}
