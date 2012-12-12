/**
 * @file echo_client.c
 *
 * @brief  DIET echo example: a client that ask for an echo.
 *
 * @author  Guillaume Verger (guillaume.verger@inria.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __WIN32__
#include <unistd.h>
#endif
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"


#define print_condition 1


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char *cmd) {
  fprintf(stderr, "Usage: %s <file.cfg> <name>\n",
          cmd);
  exit(1);
}

int
main(int argc, char *argv[]) {
  diet_profile_t *profile = NULL;
  char * name = NULL; // Name of the client
  char * echo = NULL; // Answer of the server

int *pb = NULL;

  if (argc != 2) {
    usage(argv[0]);
  }


  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc("echo", 0, 0, 1);

  name = argv[2];

  diet_string_set(diet_parameter(profile, 0), name, DIET_VOLATILE);
  diet_string_set(diet_parameter(profile, 1), NULL, DIET_VOLATILE);

if (!diet_call(profile)) {
	  diet_string_get(diet_parameter(profile, 1), echo, NULL);
	  printf("Answer : %s\n", echo);
  }

  diet_profile_free(profile);
  diet_finalize();

  return 0;
} /* main */
