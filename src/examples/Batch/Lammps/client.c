/****************************************************************************/
/* DIET client for Batch submission                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2007/01/08 13:45:35  ycaniou
 * Lammps parallel request example. Still work to do
 *
 * Revision 1.3  2006/11/28 20:40:31  ycaniou
 * Only headers
 *
 * Revision 1.2  2006/11/27 08:13:59  ycaniou
 * Added missing fields Id and Log in headers
 ****************************************************************************/

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
  double nbreel=0 ;
  size_t file_size = 0 ;
  struct timeval tv ;
  struct timezone tz ;
  int server_found = 0 ;
      
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <double> <file2>\n",
            argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  path = "concatenation" ;
  profile = diet_profile_alloc(path, 2, 2, 3);
  if (diet_file_set(diet_parameter(profile,0), DIET_VOLATILE, argv[2])) {
    printf("file1: diet_file_set error\n");
    return 1;
  }
  nbreel = strtod(argv[3],NULL) ;
  diet_scalar_set(diet_parameter(profile,1), &nbreel, DIET_VOLATILE,
                  DIET_DOUBLE);
  if (diet_file_set(diet_parameter(profile,2), DIET_VOLATILE, argv[4])) {
    printf("file2: diet_file_set error\n");
    return 1;
  }
  if (diet_file_set(diet_parameter(profile,3), DIET_VOLATILE, NULL)) {
    printf("result_file: diet_file_set error\n");
    return 1;
  }


  /*********************
   * DIET Call
   *********************/

  gettimeofday(&tv, &tz);
  printf("L'heure de soumission est %ld:%ld\n\n",tv.tv_sec,tv.tv_usec) ;

  if( SUBMISSION_TYPE == 1 ) {
    /* To ask explicitely for a parallel submission */
    printf("Call explicitly a parallel service\n") ;
    if (!diet_parallel_call(profile)) {
      printf("Job correctly submitted!\n\n\n") ;
      server_found = 1 ;
      diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  } else if ( SUBMISSION_TYPE == 0 ) {
    printf("All services, seq and parallel, with the correct name are Ok.\n") ;
    if (!diet_call(profile)) {
      printf("Job correctly submitted!\n\n\n") ;
      server_found = 1 ;
      diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  } else { /* only sequential servers are considered */
    printf("Only proposed sequential services can be selected.\n") ;
    if (!diet_sequential_call(profile)) {
      printf("Job correctly submitted!\n\n\n") ;
      server_found = 1 ;
      diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %d.\n",
               path, (int) file_size);
      }
    }
  }

  gettimeofday(&tv, &tz);
  printf("L'heure de terminaison est %ld:%ld\n\n",tv.tv_sec,tv.tv_usec) ;
  
  if( server_found == 1 ) {
    /* If uncommented, the result file is removed */
    /*  diet_free_data(diet_parameter(profile,4)); */
    free(path) ;
  }
  
  diet_profile_free(profile);
  
  diet_finalize();

  return 0;
}

