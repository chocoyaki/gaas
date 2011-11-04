/**
*  @file  server.c
*  @brief   DIET  tutorial dgemm example: server  
*  @author  - Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           - Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
*  @section Licence 
*    |LICENCE|
*/

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "DIET_server.h"

extern void dgemm_(char   *tA,
		   char   *tB,
		   int    *m,
		   int    *n,
		   int    *k,
		   double *alpha,
		   double *A,
		   int    *lda,
		   double *B,
		   int    *ldb,
		   double *beta,
		   double *C,
		   int    *ldc);

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

/*
 * SOLVE FUNCTIONS
 */

/*
 * Let us assume that C is always column-major (see src/examples/BLAS for the
 * complete solution, with matrix storage order and convertors */
int
solve_dgemm(diet_profile_t *pb) {
  char tA, tB;
  size_t m, n, k, k_, m_, n_;
  double alpha, beta;
  double *A, *B, *C;
  diet_matrix_order_t oA, oB, oC;

  /* Get arguments */
  // This line is equivalent to:
  //   alpha = *diet_value(double, diet_parameter(pb, 0))
  diet_scalar_get(diet_parameter(pb, 0), &alpha, NULL);
  diet_matrix_get(diet_parameter(pb, 1), &A, NULL, &m, &k, &oA);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_matrix_get(diet_parameter(pb, 2), &B, NULL, &k_, &n, &oB);
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (k_ != k) {
    fprintf(stderr, "dgemm Error: invalid matrix dimensions !\n");
    return 1;
  }
  diet_scalar_get(diet_parameter(pb, 3), &beta,  NULL);

  diet_matrix_get(diet_parameter(pb, 4), &C, NULL, &m_, &n_, &oC);

//  diet_generic_desc_set(diet_param_desc(pb, 4),
//			DIET_MATRIX, DIET_DOUBLE); // C

  // DEBUG
  printf("dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	 m, n, k, alpha, beta, tA, tB);
  
  /* Launch computation */
  printf("Solving dgemm_ ...");
  dgemm_(&tA, &tB, &m, &n, &k, &alpha,
	 A, (tA == 'T') ? &k : &m, B, (tB == 'T') ? &n : &k, &beta, C, &m);
  printf(" done.\n");

  /* Set OUT arguments */
  // No OUT arguments, and INOUT are already set,
  // since it has been written in the already allocated zone
 return 0;
}

/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  int res;
  diet_profile_desc_t *profile;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /* Initialize table */
  diet_service_table_init(3);

  /* Allocate dgemm profile */
  profile = diet_profile_desc_alloc("dgemm", 3, 4, 4);

  /* Set profile parameters */
  // alpha, A, B, beta, C
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 4), DIET_MATRIX, DIET_DOUBLE);

  /* Add the dgemm to the service table */
  diet_service_table_add(profile, NULL, solve_dgemm);
 
  /* Free the dgemm profile, since it was deep copied */
  diet_profile_desc_free(profile);

  // Print the table to check
  diet_print_service_table();
    
  /* Launch the SeD: no return call 
     argv[1] is the path of the configuration file */
  res = diet_SeD(argv[1], argc, argv);

  // Not reached
  return res;
}


