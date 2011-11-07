/**
 * @file client_container.c
 *
 * @brief  Workflow example : Generic client for workflow submission
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
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

/* argv[1]: client config file path
   argv[2]: path of the worflow description file */

void
usage(char *s) {
  fprintf(stderr, "Usage: %s <file.cfg> <wf_file>\n", s);
  exit(1);
}
int
checkUsage(int argc, char **argv) {
  if ((argc != 3) && (argc != 4)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  diet_wf_desc_t *profile;
  char *fileName;
  char *dataID = NULL;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  fileName = argv[2];

  profile = diet_wf_profile_alloc(fileName, "test", DIET_WF_DAG);

  printf("Try to execute the workflow\n");
  if (!diet_wf_call(profile)) {
    printf("The workflow submission succeed\n");
    if (!diet_wf_container_get(profile, "BOTTOM#out", &dataID)) {
      printf("output container ID = %s\n", dataID);
    } else {
      printf("error while getting container\n");
    }
  } else {
    printf("The workflow submission failed\n");
  }

  diet_wf_free(profile);

  return 0;
} /* main */
