/**
* @file client.c
* 
* @brief   DIET client for Batch submission   
* 
* @author   Yves Caniou (Yves.Caniou@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

#include <sys/time.h>

/* argv[1]: client config file path */

#define SUBMISSION_TYPE 0 /* 0: seq or //, 1: // only, 2: seq only */

int
main(int argc, char* argv[])
{
  char* path = NULL;
  diet_profile_t* profile = NULL;
  double nbreel = 0;
  size_t file_size = 0;
  struct timeval tv;
  struct timezone tz;
  int server_found = 0;
      
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <double> <file2>\n",
            argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  path = "concatenation";
  profile = diet_profile_alloc(path, 2, 2, 3);
  if (diet_file_set(diet_parameter(profile, 0), argv[2], DIET_VOLATILE)) {
    printf("file1: diet_file_set error\n");
    return 1;
  }
  nbreel = strtod(argv[3], NULL);
  diet_scalar_set(diet_parameter(profile, 1), &nbreel, DIET_VOLATILE,
                  DIET_DOUBLE);
  if (diet_file_set(diet_parameter(profile, 2), argv[4], DIET_VOLATILE)) {
    printf("file2: diet_file_set error\n");
    return 1;
  }
  if (diet_file_set(diet_parameter(profile, 3), NULL, DIET_VOLATILE)) {
    printf("result_file: diet_file_set error\n");
    return 1;
  }


  /*********************
   * DIET Call
   *********************/

  gettimeofday(&tv, &tz);
  printf("L'heure de soumission est %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);

  if (SUBMISSION_TYPE == 1) {
    /* To ask explicitely for a parallel submission */
    printf("Call explicitly a parallel service\n");
    if (!diet_parallel_call(profile)) {
      printf("Job correctly submitted!\n\n\n");
      server_found = 1;
      diet_file_get(diet_parameter(profile, 3), &path, NULL, &file_size);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  } else if (SUBMISSION_TYPE == 0) {
    printf("All services, seq and parallel, with the correct name are Ok.\n");
    if (!diet_call(profile)) {
      printf("Job correctly submitted!\n\n\n");
      server_found = 1;
      diet_file_get(diet_parameter(profile, 3), &path, NULL, &file_size);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  } else { /* only sequential servers are considered */
    printf("Only proposed sequential services can be selected.\n");
    if (!diet_sequential_call(profile)) {
      printf("Job correctly submitted!\n\n\n");
      server_found = 1;
      diet_file_get(diet_parameter(profile, 3), &path, NULL, &file_size);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  }

  gettimeofday(&tv, &tz);
  printf("L'heure de terminaison est %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);
  
  if (server_found == 1) {
    /* If uncommented, the result file is removed */
    /*  diet_free_data(diet_parameter(profile, 4)); */
    free(path);
  }
  
  diet_profile_free(profile);
  
  diet_finalize();

  return 0;
}

