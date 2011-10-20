/****************************************************************************/
/* Workflow client for admin commands                                       */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/09/25 12:54:23  bisnard
 * new executable to cancel dag submission
 *
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>

#include "DIET_client.h"

/* argv[1]: client config file path
 */

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> -stop <dagId>\n", s);
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc != 4)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
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
  if (! diet_wf_cancel_dag(dagId)) {
    printf("Cancel OK\n");
  } else {
    printf("Cancel failed\n");
  }

  diet_finalize();
  fflush(stdout);
  return 0;
}
