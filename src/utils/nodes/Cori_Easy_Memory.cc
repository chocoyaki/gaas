/**
 * @file Cori_Easy_Memory.cc
 *
 * @brief  CoRI Collectors of ressource information (Memory part)
 *
 * @author  Frauenkron Peter (Peter.Frauenkron@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifdef CORI_HAVE_SYS_SYSINFO
#include <sys/sysinfo.h> // get_phys_pages (), get_avphys_pages ()
#endif
#include <cstdio>
#include "Cori_Easy_Memory.hh"
#include <cstdlib> // system()
#include <iostream>
#include <fstream>
#include <cstring>
// #include <string.h>//srtcmp
#include <string>
#include "debug.hh"
#include "OSIndependance.hh" // int getpagesize (void)

using namespace std;

int
Easy_Memory::get_Total_Memory(double *result) {
  double temp1, temp2, temp3;
  if (!get_Info_Memory_byProcMem(&temp1, 0)) {
    *result = temp1;
    return 0;
  } else if (!get_Total_Memory_bysysinfo(&temp1)
             && (!get_Avail_Memory_byvmstat(&temp2))
             && (!get_Avail_Memory_bysysinfo(&temp3))) {
    double pagesize = temp2 / temp3;
    *result = temp1 * pagesize;
    return 0;
  } else {
    *result = 0;
    return 1;
  }
} // get_Total_Memory
int
Easy_Memory::get_Avail_Memory(double *result) {
  double temp;
  if (!get_Info_Memory_byProcMem(&temp, 1)) {
    *result = temp;
    return 0;
  } else if (!get_Avail_Memory_byvmstat(&temp)) {
    *result = temp;
    return 0;
  } else {
    *result = 0;
    return 1;
  }
} // get_Avail_Memory

/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/

int
Easy_Memory::get_Total_Memory_bysysinfo(double *result) {
  // using  get_phys_pages ()
  // returns the total number of physical pages the system has
  // and
  // int getpagesize (void)
  // Inquire about the virtual memory page size of the machine.
#if defined CORI_HAVE_get_avphys_pages && defined (CORI_HAVE_getpagesize)
  *result = (get_phys_pages() / 1024) * (getpagesize() / 1024);
  return 0;

#else
  // chercher autre solutions
  return 1;

#endif
} // get_Total_Memory_bysysinfo

int
Easy_Memory::get_Avail_Memory_bysysinfo(double *result) {
  // using get_avphys_pages ()
  // returns the number of physical available pages the system has
  // and
  // int getpagesize (void)
  // Inquire about the virtual memory page size of the machine.
#if defined CORI_HAVE_get_avphys_pages && defined (CORI_HAVE_getpagesize)
  *result = get_avphys_pages() * getpagesize() / (1024 * 1024);
  return 0;

#else
  // chercher autres sol
  return 1;

#endif
} // get_Avail_Memory_bysysinfo

int
Easy_Memory::get_Avail_Memory_byvmstat(double *result) {
  int returnval = 1;
#ifdef WIN32
  FILE *myfile = _popen("vmstat", "r");
#else
  FILE *myfile = popen("vmstat", "r");
#endif
  char word[256];
  if ((myfile != NULL)) {
    if (!feof(myfile)) {
      fgets(word, 256, myfile);
      if (!feof(myfile)) {
        fgets(word, 256, myfile);
        int i = 0;
        while ((!feof(myfile)) && (i < 4)) {
          fscanf(myfile, "%255s", word);
          i++;
        }
        if (i == 4) {
          *result = atof(word) / 1024;
          returnval = 0;
        } else {return 1;
        }
      }
    }
#ifdef WIN32
    _pclose(myfile);
#else
    pclose(myfile);
#endif
  }
  return returnval;
} // get_Avail_Memory_byvmstat


/*
   cat /proc/meminfo*/
int
Easy_Memory::get_Info_Memory_byProcMem(double *resultat,
                                       int freemem) {
#ifdef CORI_HAVE_PROCMEM

  char word[256];
  char demanded[256];
  /* looking in the /proc/meminfo data file*/
  ifstream file("/proc/meminfo");

  if (freemem) {
    strcpy(demanded, "MemFree:");
  } else {strcpy(demanded, "MemTotal:");
  }


  if (file.is_open()) {
    while (!file.eof()) { // look at the whole file
      if (strcmp(demanded, word) == 0) {
        file >> word;
        *resultat = atof(word) / 1024;
        return 0;
      }
      file >> word;
    }
    file.close();
  } else {
    TRACE_TEXT(TRACE_MAX_VALUE, "Error on reading file");
    return 1;
  }
#endif  // CORI_HAVE_PROCMEM
  return 1;
} // get_Info_Memory_byProcMem


// int main (){
// Easy_Memory mem;
// double maxmem;
// double availmem;
// if (!mem.get_Total_Memory(&maxmem))
// printf("%d\n", maxmem);
// else printf("no info\n");

// if (!mem.get_Avail_Memory(&availmem))
// printf("%d\n", availmem);
// else printf("no info\n");



// }
