/****************************************************************************/
/* $Id$ */
/* DIET server for BLAS functions                                           */
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
 * Revision 1.5  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.3  2002/12/18 12:11:42  jylexcel
 * SqMatSUM bug fix related to the use of convertors. Philippe+JY.
 *
 * Revision 1.2  2002/12/12 18:17:04  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "DIET_server.h"

/**
 * dgemm_ prototype
 */
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

void displayArg(const diet_data_t *arg)
{
      
  switch((int) arg->desc.generic.type) {
  case DIET_SCALAR: fprintf(stdout, "scalar");                break;
  case DIET_VECTOR: fprintf(stdout, "vector (%d)",
			    arg->desc.specific.vect.size);    break;
  case DIET_MATRIX: fprintf(stdout, "matrix (%dx%d)",
			    arg->desc.specific.mat.nb_r,
			    arg->desc.specific.mat.nb_c);   break;
  case DIET_STRING: fprintf(stdout, "string (%d)",
			    arg->desc.specific.str.length); break;
  case DIET_FILE:   fprintf(stdout, "file (%d)",
			    arg->desc.specific.file.size);  break;
  }
  if ((arg->desc.generic.type != DIET_STRING)
      && (arg->desc.generic.type != DIET_FILE)) {
    fprintf(stdout, " of ");
    switch ((int) arg->desc.generic.base_type) {
    case DIET_CHAR:     fprintf(stdout, "char");           break;
    case DIET_BYTE:     fprintf(stdout, "byte");           break;
    case DIET_INT:      fprintf(stdout, "int");            break;
    case DIET_LONGINT:  fprintf(stdout, "long int");       break;
    case DIET_FLOAT:    fprintf(stdout, "float");          break;
    case DIET_DOUBLE:   fprintf(stdout, "double");         break;
    case DIET_SCOMPLEX: fprintf(stdout, "float complex");  break;
    case DIET_DCOMPLEX: fprintf(stdout, "double complex"); break;
    }
  }
  fprintf(stdout, "\n");
}


/*
 * SOLVE FUNCTIONS
 */

int
solve_dgemm(diet_profile_t *pb)
{
  char    tA, tB;
  int     istA, istB;
  size_t  i, m, n, k, k_;
  double  alpha, beta;
  double *A, *B, *C;
  int     IsSqMatSUM=0;

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &m, &k, &istA);
  tA = istA ? 'T' : 'N';
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &k_, &n, &istB);
  tB = istB ? 'T' : 'N';
  if (k_ != k) {
    fprintf(stderr, "dgemm Error: invalid matrix dimensions !\n");
    return 1;
  }
  diet_scalar_get(diet_parameter(pb,2), &alpha, NULL);
  diet_scalar_get(diet_parameter(pb,3), &beta,  NULL);
  C = diet_value(double,diet_parameter(pb,4));
  
  /* Set B to identity in case of SUM only */
  if ((IsSqMatSUM = (! B))) {
    diet_matrix_get(diet_parameter(pb,4), NULL, NULL, NULL, &n, NULL);
    if (m != n || n != k || m != k) {
      fprintf(stderr, "dgemm Error: only square matrices can be summed.\n");
      return 1;
    }
    B = (double *) calloc(m * m, sizeof(double));
    for (i = 0; i < m; i++) {
      B[i + m * i] = 1.0;
    }
  }

  // DEBUG
  printf("dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	 m, n, k, alpha, beta, tA, tB);
  
  printf("Solving dgemm_ ...");
  dgemm_(&tA, &tB, &m, &n, &k, &alpha, A, &m, B, &k, &beta, C, &m);
  printf(" done.\n");

  if (IsSqMatSUM) {
    free(B);
  }

  return 0;
}

/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  size_t i,j;
  int res;

  diet_profile_desc_t *profile;
  diet_convertor_t *cvt;
  diet_arg_t *arg;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  diet_service_table_init(3);

  cvt = diet_convertor_alloc("BLAS/dgemm", 3, 4, 4);


  /*
   * Adding dgemm
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(3, 4, 4);
  diet_generic_desc_set(diet_param_desc(profile,0),
			DIET_SCALAR, DIET_DOUBLE); // alpha
  diet_generic_desc_set(diet_param_desc(profile,1),
			DIET_MATRIX, DIET_DOUBLE); // A
  diet_generic_desc_set(diet_param_desc(profile,2),
			DIET_MATRIX, DIET_DOUBLE); // B
  diet_generic_desc_set(diet_param_desc(profile,3),
			DIET_SCALAR, DIET_DOUBLE); // beta
  diet_generic_desc_set(diet_param_desc(profile,4),
			DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  diet_arg_cvt_set(diet_arg_conv(cvt,0), DIET_CVT_IDENTITY, 1, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,1), DIET_CVT_IDENTITY, 2, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,2), DIET_CVT_IDENTITY, 0, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,3), DIET_CVT_IDENTITY, 3, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY, 4, NULL);
  /* Add */
  diet_service_table_add("dgemm", profile, cvt, solve_dgemm);
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_set(diet_arg_conv(cvt,0), DIET_CVT_IDENTITY, 0, NULL);
  {
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 0, 0, 0);
  }
  diet_arg_cvt_set(diet_arg_conv(cvt,1), DIET_CVT_IDENTITY, -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(diet_arg_conv(cvt,2), DIET_CVT_IDENTITY, -1, arg);
  // beta is the same
  diet_arg_cvt_set(diet_arg_conv(cvt,3), DIET_CVT_IDENTITY, -1, arg);
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY,  1, NULL);
  /* Add */
  diet_service_table_add("SqMatSUM",  profile, cvt, solve_dgemm);
  diet_profile_desc_free(profile);


 /*
   * Adding MatPROD
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_set(diet_arg_conv(cvt,0), DIET_CVT_IDENTITY, 0, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,1), DIET_CVT_IDENTITY, 1, NULL);
  {
    double alpha = 1.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(diet_arg_conv(cvt,2), DIET_CVT_IDENTITY, -1, arg);
  {
    double beta = 0.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &beta, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(diet_arg_conv(cvt,3), DIET_CVT_IDENTITY, -1, arg);
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY,  2, NULL);
  /* Add */
  diet_service_table_add("MatPROD", profile, cvt, solve_dgemm);
  diet_profile_desc_free(profile);


  /*
   * Adding MatScalMult
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(0, 1, 1);
  // beta and then, C
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  {
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 0, 0, 0);
  }  
  diet_arg_cvt_set(diet_arg_conv(cvt,0), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(diet_arg_conv(cvt,1), DIET_CVT_IDENTITY,   -1, arg);
  {
    double alpha = 0.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(diet_arg_conv(cvt,2), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(diet_arg_conv(cvt,3), DIET_CVT_IDENTITY,    0, NULL);
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY,    1, NULL);
  /* Add */
  diet_service_table_add("MatScalMult", profile, cvt, solve_dgemm);
  diet_profile_desc_free(profile);


  diet_print_service_table();

  diet_convertor_free(cvt);
  
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


