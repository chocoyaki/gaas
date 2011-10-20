/****************************************************************************/
/* DIET client for BLAS functions                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2011/03/31 17:45:27  hguemar
 * more robust data input: add field width limits to scanf/fscanf
 *
 * Revision 1.10  2006/04/18 13:11:16  ycaniou
 * Still C++ -> C commentaries
 *
 * Revision 1.9  2006/04/12 16:12:28  ycaniou
 * Put back the right common_types.idl and made C commentaries from C++
 *   (discard compilation warnings)
 *
 * Revision 1.8  2005/04/25 09:03:36  hdail
 * Use hard-coded row and column orderings as input to BLAS service so we obtain
 * dependable performance.  Later we need to add more advanced example that allows
 * user to pass in file-based matrices with information on their ordering given on
 * command line.
 *
 * Revision 1.7  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.6  2003/04/10 13:33:21  pcombes
 * Apply new Coding Standards.
 *
 * Revision 1.5  2003/02/07 17:05:23  pcombes
 * Add SqMatSUM_opt with the new convertor API.
 * Use diet_free_data to properly free user's data.
 *
 * Revision 1.4  2003/01/23 19:13:44  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.3  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.2  2002/12/12 18:17:04  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

#define print_matrix(mat, m, n, rm)             \
  {                                             \
    int i, j;                                   \
    printf("%s (%s-major) = \n", #mat,          \
           (rm) ? "row" : "column");            \
    for (i = 0; i < (m); i++) {                 \
      for (j = 0; j < (n); j++) {               \
        if (rm)                                 \
          printf("%3f ", (mat)[j + i*(n)]);     \
        else                                    \
          printf("%3f ", (mat)[i + j*(m)]);     \
      }                                         \
      printf("\n");                             \
    }                                           \
    printf("\n");                               \
  }


int
empty_line()
{
  while (fgetc(stdin) != '\n');
  return 1;
}

#define NB_PB 5
static const char* PB[NB_PB] =
  {"dgemm", "SqMatSUM", "SqMatSUM_opt", "MatPROD", "MatScalMult"};


/* argv[1]: client config file path
 * argv[2]: one of the strings above */

int
main(int argc, char* argv[])
{
  char* path = NULL;
  diet_profile_t* profile = NULL;

  int i, j, m, n, k;
  double alpha, beta;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;

  srand(time(NULL));

  /*oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;*/
  oA = DIET_ROW_MAJOR;
  oB = DIET_COL_MAJOR;
  oC = DIET_ROW_MAJOR;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <file.cfg> [%s|%s|%s|%s|%s]\n",
            argv[0], PB[0], PB[1], PB[2], PB[3], PB[4]);
    return 1;
  }
  path = argv[2];

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }


  /*********************
   * dgemm
   *********************/

  if (!strcmp(path, PB[0])) {

    printf("%s: C = alpha.A*B + beta.C, with A[m, k], B[k, n] and C[m, n]\n", path);
    printf("A, B, and C are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%20d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a postive integer - Please enter m: ");
    printf("Please enter n: ");
    while (((fscanf(stdin, "%20d", &n) != 1) && empty_line()) || (n <= 0))
      printf("n is a postive integer - Please enter n: ");
    printf("Please enter k: ");
    while (((fscanf(stdin, "%20d", &k) != 1) && empty_line()) || (k <= 0))
      printf("k is a postive integer - Please enter k: ");
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line())
      printf("alpha is a double - Please enter alpha: ");
    printf("Please enter beta: ");
    while ((fscanf(stdin, "%lf", &beta) != 1) && empty_line())
      printf("beta is a double - Please enter beta: ");

    /* Fill A, B and C randomly ... */
    A = calloc(m*k, sizeof(double));
    B = calloc(k*n, sizeof(double));
    C = calloc(m*n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
    for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

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
     * SqMatSUM_opt
     *********************/

  } else if (!strcmp(path, PB[2])) {

    printf("%s: C = A + C, with A[m, m]\n", path);
    printf("A and C are generated randomly, from entered m.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%20d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    n = m;

    /* Fill A, B and C randomly ... */
    A = calloc(m*m, sizeof(double));
    C = calloc(m*m, sizeof(double));
    for (i = j = 0; i < m * m; i++) A[i] = 1.0 + j++;
    for (i = 0; i < m * m; i++)     C[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 0, 1, 1);
    diet_matrix_set(diet_parameter(profile, 0),
                    A, DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
    diet_matrix_set(diet_parameter(profile, 1),
                    C, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);

    print_matrix(A, m, m, (oA == DIET_ROW_MAJOR));
    print_matrix(C, m, m, (oC == DIET_ROW_MAJOR));



    /*********************
     * SqMatSUM and MatPROD
     *********************/

  } else if (!(strcmp(path, PB[1]) && strcmp(path, PB[3]))) {

    if (!strcmp(path, PB[1])) {
      printf("%s: C = A + B, with A[m, m] and B[m, m]\n", path);
      printf("A and B are generated randomly, from entered m.\n");
    } else {
      printf("%s: C = A * B, with A[m, k] and B[k, n]\n", path);
      printf("A and B are generated randomly, from entered m, n and k.\n");
    }
    printf("Please enter m: ");
    while (((fscanf(stdin, "%20d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    if (!strcmp(path, PB[1])) {
      k = m;
      n = m;
    } else {
      printf("Please enter n: ");
      while (((fscanf(stdin, "%20d", &n) != 1) && empty_line()) || (n <= 0))
        printf("n is a positive integer - Please enter n: ");
      printf("Please enter k: ");
      while (((fscanf(stdin, "%20d", &k) != 1) && empty_line()) || (k <= 0))
        printf("k is a positive integer - Please enter k: ");
    }

    /* Fill A, B and C randomly ... */
    A = calloc(m*k, sizeof(double));
    B = calloc(k*n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 1, 1, 2);
    diet_matrix_set(diet_parameter(profile, 0),
                    A, DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
    diet_matrix_set(diet_parameter(profile, 1),
                    B, DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
    diet_matrix_set(diet_parameter(profile, 2),
                    NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

    print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
    print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));



    /*********************
     * MatScalMult
     *********************/

  } else if (!strcmp(path, PB[4])) {

    printf("%s: C = alpha.C, with C[m, n]\n", path);
    printf("C is generated randomly, from entered m and n.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%20d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    printf("Please enter n: ");
    while (((fscanf(stdin, "%20d", &n) != 1) && empty_line()) || (n <= 0))
      printf("n is a positive integer - Please enter n: ");
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line())
      printf("alpha is a double - Please enter alpha: ");

    /* Fill A, B and C randomly ... */
    C = calloc(m*n, sizeof(double));
    for (i = j = 0; i < m * n; i++) C[i] = 1.0 + j++;

    profile = diet_profile_alloc(path, 0, 1, 1);
    diet_scalar_set(diet_parameter(profile, 0), &alpha, DIET_VOLATILE, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile, 1),
                    C, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);

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
    if (!(strcmp(path, PB[1]) && strcmp(path, PB[3]))) {
      /* C is OUT and thus must be set */
      diet_matrix_get(diet_parameter(profile, 2), &C, NULL, NULL, NULL, &oC);
      print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
      diet_free_data(diet_parameter(profile, 2));
      C = NULL;
    } else {
      print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
    }
  }

  if (A) free(A);
  if (B) free(B);
  if (C) free(C);

  diet_profile_free(profile);

  diet_finalize();

  return 0;
}

