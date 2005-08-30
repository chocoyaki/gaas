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
  double entier=4.2 ;
  
  path = "test_mpi" ;

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc(path, 0, 0, 0);
  /*  diet_string_set(diet_parameter(profile,0), "Arf! BATCH !!!",DIET_VOLATILE);*/
  diet_scalar_set(diet_parameter(profile,0), &entier,
		  DIET_VOLATILE, DIET_DOUBLE);

  // Ask explicitely for a batch submission
  diet_profile_set_batch(profile) ;

  /*********************
   * DIET Call
   *********************/

  if (!diet_call(profile)) {
    printf("Job correctly submitted!\n\n\n") ;
  }
  
  diet_profile_free(profile);
  
  diet_finalize();

  return 0;
}

