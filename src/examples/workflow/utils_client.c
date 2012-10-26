/**
 * @file utils_client.c
 *
 * @brief  Workflow client for admin commands
 *
 * @author  Benjamin Isnard (benjamin.isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <string.h>
#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#ifdef __WIN32__
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif


#include "DIET_client.h"

/* argv[1]: client config file path
 */

void
usage(char *s) {
  fprintf(stderr, "Usage: %s <file.cfg> -stop <dagId>\n", s);
  exit(1);
}
int
checkUsage(int argc, char **argv) {
  if ((argc != 4)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  char *dagId;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  if (strcmp(argv[2], "-stop")) {
    usage(argv[0]);
  }

  dagId = argv[3];

  printf("Try to cancel dag '%s'\n", dagId);
  if (!diet_wf_cancel_dag(dagId)) {
    printf("Cancel OK\n");
  } else {
    printf("Cancel failed\n");
  }

  diet_finalize();
  fflush(stdout);
  return 0;
} /* main */
