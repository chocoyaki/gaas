/****************************************************************************/
/* DIET client for pdgemm                                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/23 19:20:00  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.2  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.1.1.1  2003/04/10 13:21:39  pcombes
 * SCALAPCK is included in the autotools chain under "ScaLAPACK".
 *
 * Revision 1.4  2003/01/23 19:13:45  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.3  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.2  2002/12/12 18:17:04  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 * Revision 1.3  2002/10/22 15:17:11  bvanheuk
 * New pdgemm version using dynamic number of processes,
 * dynamic shape of workergrid and dynamic blocksize.
 ****************************************************************************/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "DIET_client.h"

#define print_matrix(mat, m, n, rm)             \
  {                                             \
    size_t i, j;                                \
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

#undef  RAND_MAX
#define RAND_MAX 10000


/* argv[1]: client config file path
 * argv[2..4]: m, n, k
 * argv[5..6]: alpha, beta           */

int
main(int argc, char* argv[])
{
  char* path = "pdgemm";
  diet_profile_t* profile = NULL;

  size_t i, j, m, n, k, procs, rows, cols, bs;
  double alpha, beta;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;

  if (argc != 11) {
    fprintf(stderr,
            "Usage: %s <file.cfg> m n k alpha beta procs rows cols blocksize\n",
            argv[0]);
    return 1;
  }

  // Parsing and preparation of m, n, j, A, B, C, alpha and beta
  m     = (size_t) atoi(argv[2]);
  n     = (size_t) atoi(argv[3]);
  k     = (size_t) atoi(argv[4]);
  alpha = strtod(argv[5], NULL);
  beta  = strtod(argv[6], NULL);
  procs = (size_t) atoi(argv[7]);
  rows  = (size_t) atoi(argv[8]);
  cols  = (size_t) atoi(argv[9]);
  bs    = (size_t) atoi(argv[10]);

  srand(time(NULL));

  oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
  oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
  oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  // Fill A, B and C randomly ...
  A = calloc(m*k, sizeof(double));
  B = calloc(k*n, sizeof(double));
  C = calloc(m*n, sizeof(double));
  for (i = j = 0; i < m * k; i++) A[i] = (rand() / 100.0) * j++;
  for (i = 0; i < k * n; i++)     B[i] = (rand() / 100.0) * j++;
  for (i = 0; i < m * n; i++)     C[i] = (rand() / 100.0) * j++;

  profile = diet_profile_alloc(path, 7, 8, 8);

  diet_scalar_set(diet_parameter(profile, 0), &procs, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 1), &rows, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 2), &cols, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 3), &bs, DIET_VOLATILE, DIET_INT);
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
  
  if (!diet_call(profile)){
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
  }

  free(A);
  free(B);
  free(C);

  diet_profile_free(profile);

  diet_finalize();

  return 0;
}

