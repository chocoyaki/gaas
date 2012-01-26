/**
 * @file batch_server.c
 *
 * @brief   Batch server: choose a random number of procs to run a minimal batch
 *
 * @author  Yves Caniou (Yves.Caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "DIET_server.h"

#define NBPROCS_MAX 5

/****************************************************************************
* PERFORMANCE FUNCTION
****************************************************************************/

void
make_perf(diet_profile_t *pb) {
  /* Give arbitrary values */
  pb->walltime = 120; /* In seconds */
  pb->nbprocs = (int) ((((float) rand()) / RAND_MAX) * NBPROCS_MAX + 1);
  pb->nbprocess = pb->nbprocs;
}


/****************************************************************************
* SOLVE FUNCTION
****************************************************************************/

int
solve_random(diet_profile_t *pb) {
  char *script = NULL;
  int result;
  int *nbprocs = (int *) malloc(sizeof(int));

  printf("Resolving batch service random!\n\n");

  *nbprocs = pb->nbprocs;
  /* Set mandatory information, like walltime */
  make_perf(pb);
  diet_scalar_desc_set(diet_parameter(pb, 0), nbprocs);

  /*****************************************************/
  /* Put the command to submit into a script           */
  /* Some unecessary things, only for the example      */
  /*****************************************************/
  script = (char *) malloc(600 * sizeof(char));
  if (script == NULL) {
    fprintf(stderr, "Memory allocation problem.. not solving the service\n\n");
    return 2;
  }
  sprintf(script,
          "echo \"File containing nodes list: $DIET_BATCH_NODESFILE\"\n"
          "echo \"List of nodes:  $DIET_BATCH_NODESLIST\"\n"
          "echo \"Name of the frontale station: $DIET_NAME_FRONTALE\"\n"
          "echo \"Number of nodes:  $DIET_BATCH_NBNODES\"\n"
          "\n");

  /* Submission */
  result = diet_submit_parallel(pb, NULL, script);

  /* Free memory */
  /* free(script); */

  if (result == -1) {
    printf("Error when submitting the script\n");
  }
  return result;
} /* solve_random */

/****************************************************************************
* MAIN
****************************************************************************/

int
main(int argc, char *argv[]) {
  int res = 0;
  int nb_max_services = 1;
  diet_profile_desc_t *profile = NULL;


  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /* Initialize state of SeD: batch or not */
  diet_set_server_status(BATCH);

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /* Allocate profile (IN, INOUT, OUT) */
  profile = diet_profile_desc_alloc("random", -1, -1, 0);

  /* Set profile parameters:
     this job is submitted by a parallel/batch system */
  diet_profile_desc_set_parallel(profile);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, solve_random)) {
    return 1;
  }

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
} /* main */
