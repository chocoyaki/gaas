/****************************************************************************/
/* DIET tutorial dgemm example: client                                      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Eddy CARON                Eddy.Caron@ens-lyon.fr                    */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"


#define print_matrix(mat, m, n, rm)        \
  {                                        \
    size_t i, j;                           \
    printf("%s (%s-major) = \n", #mat,     \
           (rm) ? "row" : "column");       \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
        if (rm)                            \
	  printf("%3f ", (mat)[j + i*(n)]);\
        else                               \
	  printf("%3f ", (mat)[i + j*(m)]);\
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/* argv[1]: client config file path */

int
main(int argc, char **argv)
{
  diet_profile_t *profile;
  char *problem_name = "dgemm";

  size_t i, j, m, n, k;
  double alpha, beta;
  double *A, *B, *C;
  diet_matrix_order_t oA, oB;

  srand(time(NULL));
  
  oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
  oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /*
   * Initialize m, n, j, A, B, C, alpha and beta
   */
  m     = 3;
  n     = 2;
  k     = 4;
  alpha = 3.4;
  beta  = -2.7;
  // Fill A, B and C randomly ...
  A = calloc(m*k, sizeof(double));
  B = calloc(k*n, sizeof(double));
  C = calloc(m*n, sizeof(double));
  for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
  for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
  for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

  /* Initialize a DIET session */
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  /* Create the profile */
  profile = diet_profile_alloc(problem_name,3, 4, 4);

  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile,0), &alpha, DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,1), A, DIET_VOLATILE, DIET_DOUBLE, m, k, oA);
  diet_matrix_set(diet_parameter(profile,2), B, DIET_VOLATILE, DIET_DOUBLE, k, n, oB);
  diet_scalar_set(diet_parameter(profile,3), &beta, DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,4), C, DIET_VOLATILE, DIET_DOUBLE, m, n, DIET_COL_MAJOR);
  
  print_matrix(A, m, k, (oA == DIET_ROW_MAJOR));
  print_matrix(B, k, n, (oB == DIET_ROW_MAJOR));
  print_matrix(C, m, n, 0);
  
  /* Call DIET */
  if (!diet_call(profile)) {
    print_matrix(C, m, n, 0);
  }
  
  /* Free profile and function handle */
  diet_profile_free(profile);
    
  /* Finalize the DIET session */
  diet_finalize();

  return 0;
}

