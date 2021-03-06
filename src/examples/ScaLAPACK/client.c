/**
 * @file client.c
 *
 * @brief   DIET client for SCALAPACK functions
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
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


int
empty_line() {
  while (fgetc(stdin) != '\n') {
  }
  return 1;
}


/* argv[1]: client config file path
 * argv[2]: pdgemm, dgemm, SqMatSUM, MatPROD or MatScalMult */

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;

  size_t i, j, m, n, k, nbProcs, nbRows, nbCols, blSize;
  double alpha, beta;
  double *A = NULL;
  double *B = NULL;
  double *C = NULL;
  diet_matrix_order_t oA, oB, oC;

  char *PB[5] = {"pdgemm", "dgemm", "SqMatSUM", "MatPROD", "MatScalMult"};

  srand(time(NULL));

  oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
  oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
  oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;

  if (argc != 3) {
    fprintf(
      stderr,
      "Usage: %s <file.cfg> [pdgemm|dgemm|SqMatSUM|MatPROD|MatScalMult]\n",
      argv[0]);
    return 1;
  }
  path = argv[2];

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }


  /*********************
  * pdgemm
  *********************/

  if (!strcmp(path, PB[0])) {
    printf("%s: C = alpha.A*B + beta.C, with A[m, k], B[k, n] and C[m, n]\n",
           path);
    printf("A, B, and C are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0)) {
      printf("m is a postive integer - Please enter m: ");
    }
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0)) {
      printf("n is a postive integer - Please enter n: ");
    }
    printf("Please enter k: ");
    while (((fscanf(stdin, "%d", &k) != 1) && empty_line()) || (k <= 0)) {
      printf("k is a postive integer - Please enter k: ");
    }
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line()) {
      printf("alpha is a double - Please enter alpha: ");
    }
    printf("Please enter beta: ");
    while ((fscanf(stdin, "%lf", &beta) != 1) && empty_line()) {
      printf("beta is a double - Please enter beta: ");
    }
    while (((fscanf(stdin, "%d",
                    &nbRows) != 1) && empty_line()) || (nbRows <= 0)) {
      printf("nbRows is a postive integer - Please enter nbRows: ");
    }
    while (((fscanf(stdin, "%d",
                    &nbCols) != 1) && empty_line()) || (nbCols <= 0)) {
      printf("nbCols is a postive integer - Please enter nbCols: ");
    }
    while (((fscanf(stdin, "%d",
                    &blSize) != 1) && empty_line()) || (blSize <= 0)) {
      printf("blSize is a postive integer - Please enter blSize: ");
    }

    // Fill A, B and C randomly ...
    A = calloc(m * k, sizeof(double));
    B = calloc(k * n, sizeof(double));
    C = calloc(m * n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++) B[i] = 1.0 + j++;
    for (i = 0; i < m * n; i++) C[i] = 1.0 + j++;

    nbProcs = nbRows * nbCols;

    profile = diet_profile_alloc(path, 3, 4, 4);
    diet_scalar_set(diet_parameter(profile, 0), &nbProcs,
                    DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 1), &nbRows,
                    DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 2), &nbCols,
                    DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 3), &blSize,
                    DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 4), &alpha,
                    DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 5), A,
                    DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
    diet_matrix_set(diet_parameter(profile, 6), B,
                    DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
    diet_scalar_set(diet_parameter(profile, 7), &beta,
                    DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 8), C,
                    DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
    print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));



    /*********************
    * dgemm
    *********************/
  } else if (!strcmp(path, PB[1])) {
    printf("%s: C = alpha.A*B + beta.C, with A[m, k], B[k, n] and C[m, n]\n",
           path);
    printf("A, B, and C are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0)) {
      printf("m is a postive integer - Please enter m: ");
    }
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0)) {
      printf("n is a postive integer - Please enter n: ");
    }
    printf("Please enter k: ");
    while (((fscanf(stdin, "%d", &k) != 1) && empty_line()) || (k <= 0)) {
      printf("k is a postive integer - Please enter k: ");
    }
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line()) {
      printf("alpha is a double - Please enter alpha: ");
    }
    printf("Please enter beta: ");
    while ((fscanf(stdin, "%lf", &beta) != 1) && empty_line()) {
      printf("beta is a double - Please enter beta: ");
    }

    // Fill A, B and C randomly ...
    A = calloc(m * k, sizeof(double));
    B = calloc(k * n, sizeof(double));
    C = calloc(m * n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++) B[i] = 1.0 + j++;
    for (i = 0; i < m * n; i++) C[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 3, 4, 4);
    diet_scalar_set(diet_parameter(profile, 0), &alpha,
                    DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 1), A,
                    DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
    diet_matrix_set(diet_parameter(profile, 2), B,
                    DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
    diet_scalar_set(diet_parameter(profile, 3), &beta,
                    DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 4), C,
                    DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
    print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));



    /*********************
    * MatSUM
    *********************/
  } else if (!strcmp(path, PB[2])) {
    printf("%s: C = A + C, with A[m, m]\n", path);
    printf("A and C are generated randomly, from entered m.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0)) {
      printf("m is a positive integer - Please enter m: ");
    }
    n = m;

    // Fill A, B and C randomly ...
    A = calloc(m * m, sizeof(double));
    B = calloc(m * m, sizeof(double));
    C = calloc(m * m, sizeof(double));
    for (i = j = 0; i < m * m; i++) A[i] = 1.0 + j++;
    for (i = 0; i < m * m; i++) C[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 0, 1, 1);
    diet_matrix_set(diet_parameter(profile, 0), A,
                    DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
    diet_matrix_set(diet_parameter(profile, 1), C,
                    DIET_VOLATILE, DIET_DOUBLE, m, m, oB);

    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));



    /*********************
    * MatPROD
    *********************/
  } else if (!strcmp(path, PB[3])) {
    printf("%s: C = A * B, with A[m, k] and B[k, n]\n", path);
    printf("A and B are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0)) {
      printf("m is a positive integer - Please enter m: ");
    }
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0)) {
      printf("n is a positive integer - Please enter n: ");
    }
    printf("Please enter k: ");
    while (((fscanf(stdin, "%d", &k) != 1) && empty_line()) || (k <= 0)) {
      printf("k is a positive integer - Please enter k: ");
    }

    // Fill A, B and C randomly ...
    A = calloc(m * k, sizeof(double));
    B = calloc(k * n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++) B[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 1, 1, 2);
    diet_matrix_set(diet_parameter(profile, 0), A,
                    DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
    diet_matrix_set(diet_parameter(profile, 1), B,
                    DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
    diet_matrix_set(diet_parameter(profile, 2), NULL,
                    DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
    print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));



    /*********************
    * MatScalMult
    *********************/
  } else if (!strcmp(path, PB[4])) {
    printf("%s: C = alpha.C, with C[m, n]\n", path);
    printf("C is generated randomly, from entered m and n.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0)) {
      printf("m is a positive integer - Please enter m: ");
    }
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0)) {
      printf("n is a positive integer - Please enter n: ");
    }
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line()) {
      printf("alpha is a double - Please enter alpha: ");
    }

    // Fill A, B and C randomly ...
    C = calloc(m * n, sizeof(double));
    for (i = j = 0; i < m * n; i++) C[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 0, 1, 1);
    diet_scalar_set(diet_parameter(profile, 0), &alpha,
                    DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 1), C,
                    DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));



    /*********************
    * Dummy pb (no server offers the correspunding service)
    *********************/
  } else {
    alpha = 0.0;
    m = n = 0;
    profile = diet_profile_alloc(path, -1, 0, 0);
    diet_scalar_set(diet_parameter(profile, 0), &alpha,
                    DIET_VOLATILE, DIET_DOUBLE);
  }


  /*********************
  * DIET Call
  *********************/

  if (!diet_call(profile)) {
    if (!strcmp(path, PB[3])) { // C is OUT and thus must be set
      diet_matrix_get(diet_parameter(profile, 2), &C, NULL, NULL, NULL, &oC);
      print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
      diet_free_data(diet_parameter(profile, 2));
      C = NULL;
    } else {
      print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
    }
  }

  if (A) {
    free(A);
  }
  if (B) {
    free(B);
  }
  if (C) {
    free(C);
  }

  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */
