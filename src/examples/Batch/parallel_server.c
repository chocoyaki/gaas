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
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "DIET_server.h"
//#include "DIET_mutex.h"


/****************************************************************************
 * SOLVE FUNCTION
 ****************************************************************************/

int solve_test_mpi(diet_profile_t *pb)
{
  char* chaine ;
  char *cmd ;
   
  diet_string_get(diet_parameter(pb,0), &chaine, NULL);

  /* Make the command to submit in a script */
  sprintf(cmd,"/JobMPI/a.out %s",chaine) ;
  
  diet_submit_batch(pb,cmd) ;
  
  /* Must keep an eye until the end of the job
   * to send back the corresponding results */

  return 0 ;
}
/*
int make_perf(diet_profile_t *pb)
{
// Must look how to integrate in elagi a convenient way 
//    to ask the batch scheduler for prediction performances 
  int l ;
  
  l=5 ;
  }
*/
/****************************************************************************
 * MAIN
 ****************************************************************************/

int
main(int argc, char* argv[])
{
  int res = 0;
  int nb_max_services=1 ;
  diet_profile_desc_t* profile = NULL ;
  
  
  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /* Allocate batch profile */
  profile = diet_profile_desc_alloc("test_mpi",1,1,1);

  /* Set profile parameters */
  /* string to print */
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_STRING, DIET_CHAR);

  /* This job is a batch one */
  diet_profile_desc_set_parallel(profile) ;
  
  /* Add the smprod to the service table */
  diet_service_table_add(profile, NULL, solve_test_mpi);

  /* Free the smprod profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Allocate Perf. profile */
  //  profile = diet_profile_desc_alloc("perf",0, 0, 0);

  /* Set profile parameters */
  /*  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_FLOAT);
  */
  /* Add the smprod to the service table */
  //diet_service_table_add(profile, NULL, make_perf);

  // Print the table to check
  diet_print_service_table();

  // Launch the server
  res = diet_SeD(argv[1], argc, argv);

  return res;
}
