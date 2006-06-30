/****************************************************************************/
/* DIET server for Batch submission                                         */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Yves CANIOU (ycaniou@ens-lyon.fr)                                   */
/* $LICENSE$                                                                */
/****************************************************************************/
/* 
 * 
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"


/* argv[1]: client config file path */

int
main(int argc, char* argv[])
{
  char* path = NULL;
  diet_profile_t* profile = NULL;
  double nbreel=0 ;
  size_t file_size = 0 ;
    
  if (argc != 5) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <double> <file2>\n",
	    argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  path = "helloMPI" ;
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

  /* Ask explicitely for a batch submission */
  printf("Call explicitly a parallel service\n") ;
  diet_profile_set_batch(profile) ;

  /*********************
   * DIET Call
   *********************/

  if (!diet_call(profile)) {
    printf("Job correctly submitted!\n\n\n") ;
    diet_file_get(diet_parameter(profile,3), NULL, &file_size, &path);
    if (path && (*path != '\0')) {
      printf("Location of returned file is %s, its size is %d.\n",
	     path, (int) file_size);
    }
  }
  
  /* If uncommented, the result file is removed */
  /*  diet_free_data(diet_parameter(profile,4)); */

  free(path) ;
  diet_profile_free(profile);
  
  diet_finalize();

  return 0;
}

