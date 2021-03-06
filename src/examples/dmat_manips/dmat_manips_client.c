/**
 * @file dmat_manips_client.c
 *
 * @brief  a DIET client for transpose, MatSUM and MatPROD problems
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
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

#include "DIET_client.h"

#define print_condition 1
#define print_matrix(mat, m, n, rm)             \
  if (print_condition) {                        \
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


#define NB_PB 3
static const char *PB[NB_PB] =
{"T", "MatPROD", "MatSUM"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char *cmd) {
  fprintf(stderr, "Usage: %s %s <file.cfg> [%s|%s|%s]\n", cmd,
          "[--repeat <n>] [--pause <n �s>]", PB[0], PB[1], PB[2]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 client.cfg MatSUM\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 --pause 1000 %s\n",
          cmd, "client.cfg MatSUM (1ms between two steps of the loop)");
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i;
  size_t m, n;
  int n_loops = 1;
  char *path = NULL;
  diet_profile_t *profile = NULL;
  /* double mat1[9] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0}; */
  /* double mat2[9] = {10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0, 18.0}; */
  double A[9];
  double B[9];
  double *C = NULL;
  diet_matrix_order_t oA, oB, oC;

  int pb[NB_PB] = {0, 0, 0};

  size_t nb_of_requests;
  int pause = 0;

  srand(time(NULL));

  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = atoi(argv[i + 1]);
      if (n_loops < 0) {
        fprintf(stderr, "Argument to option %s must be >= 0.\n", argv[i]);
        usage(argv[0]);
      }
      i++;
      memmove(argv + i - 2, argv + i, (argc - i) * sizeof(char *));
      i -= 2;
      argc -= 2;
    } else if (strcmp("--pause", argv[i]) == 0) {
      pause = atoi(argv[i + 1]);
      i++;
      memmove(argv + i - 2, argv + i, (argc - i) * sizeof(char *));
      i -= 2;
      argc -= 2;
    } else {
      fprintf(stderr, "Unrecognized option %s\n", argv[i]);
      usage(argv[0]);
    }
  }

  if (argc - i != 2) {
    usage(argv[0]);
  }
  path = argv[argc - 1];

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) {
      break;
    }
  }


  nb_of_requests = 0;

  oA = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  oB = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  oC = ((double) rand() / (double) RAND_MAX <= 0.5) ? DIET_ROW_MAJOR :
       DIET_COL_MAJOR;
  /* printf("oA: %d oB: %d oC: %d\n", oA, oB, oC); */

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

  C = NULL;

  if (pb[0]) {
    /* Each repetition transposes result of last */
    /* Set values only once */
    n = 2;
    m = 3;
  }

  for (i = 0; i < n_loops; i++) {
    if (pb[1] || pb[2]) {
      n = 2;
      m = 3;
    }

    if (pb[0]) {
      profile = diet_profile_alloc(path, 0, 0, 1);
      diet_matrix_set(diet_parameter(profile, 0),
                      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      diet_matrix_set(diet_parameter(profile, 1),
                      NULL, DIET_VOLATILE, DIET_DOUBLE, n, m, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
    } else if (pb[1] || pb[2]) {
      profile = diet_profile_alloc(path, 1, 1, 2);
      diet_matrix_set(diet_parameter(profile, 0),
                      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      if (pb[1]) {
        diet_matrix_set(diet_parameter(profile, 1),
                        B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
        print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
        diet_matrix_set(diet_parameter(profile, 2),
                        NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
      } else {
        diet_matrix_set(diet_parameter(profile, 1),
                        B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
        print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
        diet_matrix_set(diet_parameter(profile, 2),
                        NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
      }
    } else {
      fprintf(stderr, "Unknown problem: %s !\n", path);
      return 1;
    }

    nb_of_requests++;
    if (!diet_call(profile)) {
      if (pb[0]) {
        diet_matrix_get(diet_parameter(profile, 1), &C, NULL, &m, &n, &oA);
        print_matrix(C, m, n, (oA == DIET_ROW_MAJOR));
        diet_free_data(diet_parameter(profile, 1));
      } else {
        diet_matrix_get(diet_parameter(profile, 2), &C, NULL, &m, &n, &oC);
        print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
        diet_free_data(diet_parameter(profile, 2));
      }
    }
    diet_profile_free(profile);
  }

  diet_finalize();

  return 0;
} /* main */
