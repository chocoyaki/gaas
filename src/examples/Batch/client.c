/**
 * @file client.c
 *
 * @brief   DIET client for Batch submission
 *
 * @author  Yves Caniou (Yves.Caniou@ens-lyon.fr)
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

int
main(int argc, char *argv[]) {
  char *path = "random";
  diet_profile_t *profile = NULL;
  int *nbprocs;
  struct timeval tv;
  struct timezone tz;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n",
            argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  path = "random";
  profile = diet_profile_alloc(path, -1, -1, 0);
  diet_scalar_set(diet_parameter(profile, 0), NULL, DIET_VOLATILE, DIET_INT);

  /*********************
  * DIET Call
  *********************/

  gettimeofday(&tv, &tz);
  printf("L'heure de soumission est %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);

  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 0), &nbprocs, NULL);
    printf("The job has been solved on %d processor(s)\n", *nbprocs);
  } else {
    fprintf(stderr, "diet_call() has returned with an error code !\n");
    return 1;
  }

  gettimeofday(&tv, &tz);
  printf("L'heure de terminaison est %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);

  diet_profile_free(profile);
  diet_finalize();

  return 0;
} /* main */
