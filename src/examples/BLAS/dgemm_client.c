/****************************************************************************/
/* BLAS/dgemm server : a DIET client for dgemm                              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.10  2011/01/24 23:14:19  bdepardo
 * Fixed memory leaks
 *
 * Revision 1.9  2006/06/30 15:26:02  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.8  2005/04/25 09:06:01  hdail
 * Use hard-coded row and column orderings as input to BLAS service so we obtain
 * dependable performance.  Later we need to add more advanced example that
 * allows user to pass in file-based matrices with information on their ordering
 * given on command line.
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
 *
 * Revision 1.3  2002/09/17 15:23:17  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
 *
 * Revision 1.2  2002/09/09 15:56:56  pcombes
 * Update for dgemm and bug fixes
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

/* argv[1]: client config file path
 * argv[2]: dgemm, MatSUM, or MatPROD */

int
main(int argc, char* argv[])
{
  diet_profile_t* profile;
  char* path = "dgemm";

  size_t i, j, m, n, k;
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

  if (argc != 7) {
    fprintf(stderr, "Usage: %s <file.cfg> m n k alpha beta\n", argv[0]);
    return 1;
  }

  /* Parsing and preparation of m, n, j, A, B, C, alpha and beta */
  m     = (size_t) atoi(argv[2]);
  n     = (size_t) atoi(argv[3]);
  k     = (size_t) atoi(argv[4]);
  alpha = strtod(argv[5], NULL);
  beta  = strtod(argv[6], NULL);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  /* Fill A, B and C randomly ... */
  A = calloc(m*k, sizeof(double));
  B = calloc(k*n, sizeof(double));
  C = calloc(m*n, sizeof(double));
  for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
  for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
  for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

  profile = diet_profile_alloc(path, 3, 4, 4);

  diet_scalar_set(diet_parameter(profile,0), &alpha,
                  DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,1), A,
                  DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
  diet_matrix_set(diet_parameter(profile,2), B,
                  DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
  diet_scalar_set(diet_parameter(profile,3), &beta,
                  DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,4), C,
                  DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
  
  print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
  print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
  print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
  
  if (!diet_call(profile)) {
    print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
  }
  
  free(A);
  free(B);
  free(C);

  diet_profile_free(profile);
    
  diet_finalize();

  return 0;
}

