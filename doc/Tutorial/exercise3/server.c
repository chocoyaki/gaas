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

int
solve_dgemm(diet_profile_t *pb)
{
  char    tA, tB;
  int     i, m, n, k;
  double  alpha, beta;
  double *A, *B, *C;

  /* Get arguments */

  /* Launch computation */

  /* Set OUT arguments */

  return 0;
}

/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  size_t i, j;
  int res;

  diet_profile_desc_t *profile;

  /* Initialize table */

  /* Allocate dgemm profile */

  /* Set profile parameters */

  /* Add the dgemm to the service table */

  /* Free the dgemm profile, since it was deep copied */
    
  /* Launch the SeD: no return call 
     argv[1] is the path of the configuration file */

  /* Not reached */
  return res;
}


