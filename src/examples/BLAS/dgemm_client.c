/****************************************************************************/
/* $Id$ */
/* BLAS/dgemm server : a DIET client for dgemm                              */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2002/12/03 19:05:12  pcombes
 * Clean CVS logs in file.
 * Separate BLAS and SCALAPACK examples.
 *
 * Revision 1.3  2002/09/17 15:23:17  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
 *
 * Revision 1.2  2002/09/09 15:56:56  pcombes
 * Update for dgemm and bug fixes
 *
 * Revision 1.1.1.1  2002/05/24 19:40:09  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 *
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"


#define print_matrix(mat, m, n)            \
  {                                        \
    size_t i, j;                           \
    printf("%s = \n", #mat);               \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
	printf("%3f ", (mat)[i + j*(m)]);  \
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }

/* argv[1]: client config file path
 * argv[2]: dgemm, MatSUM, or MatPROD */

int
main(int argc, char **argv)
{
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;
  char *path = "dgemm";

  int    i, j, m, n, k;
  double alpha, beta;
  double *A, *B, *C;

  if (argc != 7) {
    fprintf(stderr, "Usage: client <file> m n k alpha beta\n");
    return 1;
  }

  // Parsing and preparation of m, n, j, A, B, C, alpha and beta
  m     = atoi(argv[2]);
  n     = atoi(argv[3]);
  k     = atoi(argv[4]);
  alpha = strtod(argv[5], NULL);
  beta  = strtod(argv[6], NULL);

  // Fill A, B and C randomly ...
  A = calloc(m*k, sizeof(double));
  B = calloc(k*n, sizeof(double));
  C = calloc(m*n, sizeof(double));
  for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
  for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
  for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

  if (diet_initialize(argc, argv, argv[1])) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  fhandle = diet_function_handle_default(path);
  profile = profile_alloc(3, 4, 4);

  scalar_set(&(profile->parameters[0]), &alpha, DIET_VOLATILE, DIET_DOUBLE);
  matrix_set(&(profile->parameters[1]), A, DIET_VOLATILE, DIET_DOUBLE, m, k, 0);
  matrix_set(&(profile->parameters[2]), B, DIET_VOLATILE, DIET_DOUBLE, k, n, 0);
  scalar_set(&(profile->parameters[3]), &beta, DIET_VOLATILE, DIET_DOUBLE);
  matrix_set(&(profile->parameters[4]), C, DIET_VOLATILE, DIET_DOUBLE, m, n, 0);
  
  print_matrix(A, m, k);
  print_matrix(B, k, n);
  print_matrix(C, m, n);
  
  if (!diet_call(fhandle, profile)) {
    print_matrix(C, m, n);
  }
  
  profile_free(profile);
  diet_function_handle_destruct(fhandle);
    
  diet_finalize();

  return 0;
}

