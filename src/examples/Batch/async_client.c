/**
 * @file async_client.c
 *
 * @brief   DIET client for Batch submission
 *
 * @author  Yves Caniou (Yves.Caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
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
  diet_reqID_t rst;

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

  if (!diet_call_async(profile, &rst)) {
    printf("The job is beeing solved\n");
  } else {printf("Error during diet_call_async()\n");
  }

  gettimeofday(&tv, &tz);
  printf("Return of the call is %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);

  printf("request ID value = %d\n", rst);
  diet_wait(rst);

  diet_scalar_get(diet_parameter(profile, 0), &nbprocs, NULL);
  printf("The job has been solved on %d processor(s)\n", *nbprocs);

  gettimeofday(&tv, &tz);
  printf("Completion time is %ld:%ld\n\n", tv.tv_sec, tv.tv_usec);

  diet_profile_free(profile);
  diet_finalize();

  return 0;
} /* main */
