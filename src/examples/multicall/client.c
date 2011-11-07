/**
 * @file client.c
 *
 * @brief  Multicall client
 *
 * @author  Ghislain Charrier (ghislain.charrier@inria.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

void
usage(char *cmd) {
  fprintf(stdout, "Usage: %S <file.cfg> string\n", cmd);
  fprintf(stdout, "The string must contain simulations names separated by #\n");
  fprintf(stdout, "exemple : simu1#simu2#simu3#simu4\n");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int NR = 1800;
  int NS = 0;
  char c = '#';
  diet_profile_t *profile = NULL;

  if (argc != 3) {
    usage(argv[0]);
  }


  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  char *s = argv[2];
  char *temp = (char *) malloc((strlen(s) + 1) * sizeof(char));
  char *tempBak = temp;
  temp = strcpy(temp, s);


  while ((temp = strtok(temp, &c)) != NULL) {
    temp = NULL;
    NS++;
  }
  free(tempBak);

  profile = diet_profile_alloc("CERFACS", 2, 2, 2);
  diet_scalar_set(diet_parameter(profile, 0), &NS, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 1), &NR, DIET_VOLATILE, DIET_INT);
  diet_paramstring_set(diet_parameter(profile, 2), s, DIET_VOLATILE);

  diet_reqID_t rst;

  if (!diet_call_async(profile, &rst)) {
    printf("call done\n");
  } else {
    fprintf(stderr, "diet_call has returned with an error code !!!\n");
    return 1;
  }
  diet_wait(rst);
  diet_profile_free(profile);
  diet_finalize();
  return 0;
} /* main */
