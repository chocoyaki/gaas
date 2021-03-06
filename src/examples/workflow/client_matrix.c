/**
 * @file client_matrix.c
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

#include "progs.h"

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
  size_t m, n;
  diet_matrix_order_t o;
  double *A;

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
    /**
     *  diet_matrix_get(diet_parameter(profile, 0), NULL, NULL, &m, &n, &oA);
       print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
     */
    /*
       diet_wf_matrix_get(id, value, nb_rows, nb_cols, order)
     */
    diet_wf_matrix_get(profile, "n3#out2", &A, &m, &n, &o);
    print_matrix(A, m, n, (o == DIET_ROW_MAJOR));
  } else {
    printf("The workflow submission failed\n");
  }

  diet_wf_free(profile);

  return 0;
} /* main */
