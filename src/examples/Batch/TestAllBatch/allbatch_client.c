/****************************************************************************/
/* DIET client for Batch submission: loadleveler on a parallel machine      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/08/06 14:25:27  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.3  2010/03/05 15:52:08  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.2  2009/11/27 03:24:30  ycaniou
 * Add user_command possibility before the end of Batch prologue (only
 * to be used for batch dependent code!)
 * Memory leak/segfault--
 * New easy Batch basic example
 * Management of OAR2_X Batch scheduler
 *
 * Revision 1.1  2007/11/02 16:12:57  ycaniou
 * Ajout des fichiers exemple. Tests en cours.
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

#include <sys/time.h>

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
        printf("Location of returned file is %s, its size is %zd.\n",
               path, file_size);
      }
    } else printf("Error in diet_parallel_call()\n") ;
  } else if ( SUBMISSION_TYPE == 0 ) {
    printf("All services, seq and parallel, with the correct name are Ok.\n") ;
    if (!diet_call(profile)) {
      printf("Job correctly submitted!\n\n\n") ;
      server_found = 1 ;
      diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %zd.\n",
               path, file_size);
      }
    } else printf("Error in diet_call()\n") ;
  } else { /* only sequential servers are considered */
    printf("Only proposed sequential services can be selected.\n") ;
    if (!diet_sequential_call(profile)) {
      printf("Job correctly submitted!\n\n\n") ;
      server_found = 1 ;
      diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
      if (path && (*path != '\0')) {
        printf("Location of returned file is %s, its size is %zd.\n",
               path, file_size);
      }
    } else printf("Error in diet_sequential_call()\n") ;
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

