/****************************************************************************/
/* $Id$ */
/* DIET client for BLAS functions                                           */
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

int empty_line()
{
  while (fgetc(stdin) != '\n');
  return 1;
}


/* argv[1]: client config file path
 * argv[2]: dgemm, SqMatSUM, MatPROD or MatScalMult */

int
main(int argc, char **argv)
{
  char *path;
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;

  int    i, j, m, n, k;
  double alpha, beta;
  double *A, *B, *C = NULL;

  char *PB[4] = {"dgemm", "SqMatSUM", "MatPROD", "MatScalMult"};

  if (argc != 3) {
    fprintf(stderr,
	    "Usage: client <file> [dgemm|SqMatSUM|MatPROD|MatScalMult]\n");
    return 1;
  }
  path = argv[2];

  if (diet_initialize(argc, argv, argv[1])) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }


  /*********************
   * dgemm
   *********************/

  if (!strcmp(path, PB[0])) {
  
    printf("%s: C = alpha.A*B + beta.C, with A[m,k], B[k,n] and C[m,n]\n", path);
    printf("A, B, and C are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a postive integer - Please enter m: ");
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0))
      printf("n is a postive integer - Please enter n: ");
    printf("Please enter k: ");
    while (((fscanf(stdin, "%d", &k) != 1) && empty_line()) || (k <= 0))
      printf("k is a postive integer - Please enter k: ");
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line())
      printf("alpha is a double - Please enter alpha: ");
    printf("Please enter beta: ");
    while ((fscanf(stdin, "%lf", &beta) != 1) && empty_line())
      printf("beta is a double - Please enter beta: ");
  
    // Fill A, B and C randomly ...
    A = calloc(m*k, sizeof(double));
    B = calloc(k*n, sizeof(double));
    C = calloc(m*n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;
    for (i = 0; i < m * n; i++)     C[i] = 1.0 + j++;

    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(3, 4, 4);
    scalar_set(&(profile->parameters[0]), &alpha, DIET_VOLATILE, DIET_DOUBLE);
    matrix_set(&(profile->parameters[1]), A, DIET_VOLATILE, DIET_DOUBLE, m, k, 0);
    matrix_set(&(profile->parameters[2]), B, DIET_VOLATILE, DIET_DOUBLE, k, n, 0);
    scalar_set(&(profile->parameters[3]), &beta,  DIET_VOLATILE, DIET_DOUBLE);
    matrix_set(&(profile->parameters[4]), C, DIET_VOLATILE, DIET_DOUBLE, m, n, 0);
  
    print_matrix(A, m, k);
    print_matrix(B, k, n);
    print_matrix(C, m, n);



  /*********************
   * MatSUM
   *********************/

  } else if (!strcmp(path, PB[1])) {
    
    printf("%s: C = A + C, with A[m,m]\n", path);
    printf("A and C are generated randomly, from entered m.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    n = m;
    
    // Fill A, B and C randomly ...
    A = calloc(m*m, sizeof(double));
    B = calloc(m*m, sizeof(double));
    C = calloc(m*m, sizeof(double));
    for (i = j = 0; i < m * m; i++) A[i] = 1.0 + j++;
    for (i = 0; i < m * m; i++)     C[i] = 1.0 + j++;

    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(0, 1, 1);
    matrix_set(&(profile->parameters[0]),
	       A, DIET_VOLATILE, DIET_DOUBLE, m, m, 0);
    matrix_set(&(profile->parameters[1]),
	       C, DIET_VOLATILE, DIET_DOUBLE, m, m, 0);
    
    print_matrix(A, m, m);
    print_matrix(C, m, m);


    
  /*********************
   * MatPROD
   *********************/

  } else if (!strcmp(path, PB[2])) {

    printf("%s: C = A * B, with A[m,k] and B[k,n]\n", path);
    printf("A and B are generated randomly, from entered m, n and k.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0))
      printf("n is a positive integer - Please enter n: ");
    printf("Please enter k: ");
    while (((fscanf(stdin, "%d", &k) != 1) && empty_line()) || (k <= 0))
      printf("k is a positive integer - Please enter k: ");

    // Fill A, B and C randomly ...
    A = calloc(m*k, sizeof(double));
    B = calloc(k*n, sizeof(double));
    for (i = j = 0; i < m * k; i++) A[i] = 1.0 + j++;
    for (i = 0; i < k * n; i++)     B[i] = 1.0 + j++;

    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(1, 1, 2);
    matrix_set(&(profile->parameters[0]),
	       A, DIET_VOLATILE, DIET_DOUBLE, m, k, 0);
    matrix_set(&(profile->parameters[1]),
	       B, DIET_VOLATILE, DIET_DOUBLE, k, n, 0);
    matrix_set(&(profile->parameters[2]),
	       NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, 0);

    print_matrix(A, m, k);
    print_matrix(B, k, n);


    
  /*********************
   * MatScalMult
   *********************/

  } else if (!strcmp(path, PB[3])) {
  
    printf("%s: C = alpha.C, with C[m,n]\n", path);
    printf("C is generated randomly, from entered m and n.\n");
    printf("Please enter m: ");
    while (((fscanf(stdin, "%d", &m) != 1) && empty_line()) || (m <= 0))
      printf("m is a positive integer - Please enter m: ");
    printf("Please enter n: ");
    while (((fscanf(stdin, "%d", &n) != 1) && empty_line()) || (n <= 0))
      printf("n is a positive integer - Please enter n: ");
    printf("Please enter alpha: ");
    while ((fscanf(stdin, "%lf", &alpha) != 1) && empty_line())
      printf("alpha is a double - Please enter alpha: ");
    printf("m = %d\n", m);

    // Fill A, B and C randomly ...
    C = calloc(m*n, sizeof(double));
    for (i = j = 0; i < m * n; i++) C[i] = 1.0 + j++;

    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(0, 1, 1);
    scalar_set(&(profile->parameters[0]), &alpha, DIET_VOLATILE, DIET_DOUBLE);
    matrix_set(&(profile->parameters[1]),
	       C, DIET_VOLATILE, DIET_DOUBLE, m, n, 0);

    print_matrix(C, m, n);


    
  /*********************
   * Dummy pb (no server offers the correspunding service)
   *********************/

  } else {

    alpha = 0.0;
    m = n = 0;
    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(-1, 0, 0);
    scalar_set(&(profile->parameters[0]), &alpha, DIET_VOLATILE, DIET_DOUBLE);

  }
  

  /*********************
   * DIET Call
   *********************/

  if (!diet_call(fhandle, profile)) {
    if (!strcmp(path, PB[2])) { // C is OUT and thus must be set
      C = (double *)profile->parameters[2].value;
    }
    print_matrix(C, m, n);
  }
  
  profile_free(profile);
  diet_function_handle_destruct(fhandle);
  
  diet_finalize();

  return 0;
}

