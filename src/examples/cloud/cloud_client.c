/**
 * @file cloud_client.c
 *
 * @brief  Example client for the Cloud Batch System
 *
 * @author  Adrian Muresan (adrian.muresan@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include "DIET_client.h"

#define print_matrix(mat, m, n, rm)             \
  {                                             \
    size_t i, j;                                \
    printf("%s (%s-major) = \n", # mat,          \
           (rm) ? "row" : "column");            \
    for (i = 0; i < (m); i++) {                 \
      for (j = 0; j < (n); j++) {               \
        if (rm) {                                 \
          printf("%3f ", (mat)[j + i * (n)]); }     \
        else {                                    \
          printf("%3f ", (mat)[i + j * (m)]); }     \
      }                                         \
      printf("\n");                             \
    }                                           \
    printf("\n");                               \
  }

char *path_to_A;
char *path_to_B;

void
usage(char *me) {
  fprintf(
    stderr,
    "Usage: %s <file.cfg> [--A-path <path-to-A-matrix> --B-path <path-to-B-matrix>]\n"
    "\t ex1: %s client_cloud.cfg\n"
    "\t ex2: %s client_cloud.cfg --A-path A.in --B-path B.in\n",
    me, me, me);
}

char
parse_args(int argc, char **argv) {
  int i;
  path_to_A = NULL;
  path_to_B = NULL;
  if (argc <= 1) {
    usage(argv[0]);
    return 0;
  }
  for (i = 2; i < argc; i++) {
    if (strcmp("--A-path", argv[i]) == 0 && i + 1 < argc) {
      path_to_A = argv[i + 1];
    }
    if (strcmp("--B-path", argv[i]) == 0 && i + 1 < argc) {
      path_to_B = argv[i + 1];
    }
  }
  if ((path_to_A == NULL &&
       path_to_B == NULL) || (path_to_A != NULL && path_to_B != NULL)) {
    return 1;
  } else {
    usage(argv[0]);
    return 0;
  }
} /* parse_args */

char
matrix_from_file(char *file_path, double **mat, int *n, int *m) {
  FILE *fi;
  int i;
  fi = fopen(file_path, "r");
  if (fi == NULL) {
    return 0;
  }
  fscanf(fi, "%20d", n);
  fscanf(fi, "%20d", m);
  *mat = (double *) malloc(*n * *m * sizeof(double));

  for (i = 0; i < *n * *m; i++)
    fscanf(fi, "%lf", &(*mat)[i]);

  fclose(fi);

  return 1;
} /* matrix_from_file */

int
main(int argc, char **argv) {
  if (!parse_args(argc, argv)) {
    return 1;
  }
  diet_profile_t *profile;
  char *result;
  double *C = NULL;
  char *rC = NULL;
  /* Initialize matrixes */
  double *A;
  double *B;
  int n;
  int m;
  if (path_to_A == NULL) {
    n = 2;
    m = 3;
    A = (double *) malloc(9 * sizeof(double));
    B = (double *) malloc(9 * sizeof(double));
    A[0] = 1.0;
    A[1] = 2.0;
    A[2] = 3.0;
    A[3] = 4.0;
    A[4] = 5.0;
    A[5] = 6.0;
    A[6] = 7.0;
    A[7] = 8.0;
    A[8] = 9.0;

    B[0] = 10.0;
    B[1] = 11.0;
    B[2] = 12.0;
    B[3] = 13.0;
    B[4] = 14.0;
    B[5] = 15.0;
    B[6] = 16.0;
    B[7] = 17.0;
    B[8] = 18.0;
  } else {
    if (!matrix_from_file(path_to_A, &A, &n, &m) ||
        !matrix_from_file(path_to_B, &B, &m, &n)) {
      fprintf(stderr, "Cannot read input files.\n");
      return 1;
    }
  }

  /* Initialize a DIET session */
  diet_initialize(argv[1], argc, argv);
  /* Create the profile as explained in Chapter 3 */
  profile = diet_profile_alloc("cloud-demo", 1, 1, 3); /* last_in, last_inout, last_out */
  /* Set profile arguments */
  diet_string_set(diet_parameter(profile, 3), NULL, DIET_VOLATILE);

  /* init matrix orders */
  diet_matrix_order_t oA, oB, oC;
  oA = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  oB = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  oC = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  C = NULL;

  diet_matrix_set(diet_parameter(profile, 0),
                  A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
  print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile, 1),
                  B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
  print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
  diet_string_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE);

  if (!diet_parallel_call(profile)) { /* If the call has succeeded ... */
    /* Get and print time */
    diet_string_get(diet_parameter(profile, 3), &result, NULL);
    if (result == NULL) {
      printf("Error: result is null !\n");
    } else {
      printf("Host info: %s\n", result);
    }
    diet_string_get(diet_parameter(profile, 2), &rC, NULL);
    printf("C = %s\n", rC);
  } else {
    printf("calling error...\n");
  }

  /* Free profile */
  diet_profile_free(profile);
  diet_finalize();

  return 0;
} /* main */
