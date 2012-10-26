/**
 * @file client_file.c
 *
 * @brief  Workflow example : Client for file workflow submission
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
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
  char *path1 = NULL, *path2 = NULL;
  size_t out_size1 = 0, out_size2 = 0;

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
    diet_wf_file_get(profile, "n3#out1", &out_size1, &path1);
    if (path1 && (*path1 != '\0')) {
      printf("Location of returned file is %s, its size is %ld.\n",
             path1, out_size1);
      /* diet_free_data(diet_parameter(profile, 4)); */
    }
    diet_wf_file_get(profile, "n3#out2", &out_size2, &path2);
    if (path2 && (*path2 != '\0')) {
      printf("Location of returned file is %s, its size is %ld.\n",
             path2, out_size2);
      /* diet_free_data(diet_parameter(profile, 4)); */
    }
  } else {
    printf("The workflow submission failed\n");
  }

  diet_wf_free(profile);

  return 0;
} /* main */
