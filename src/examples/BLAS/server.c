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
 * Revision 1.1  2002/12/03 19:05:12  pcombes
 * Clean CVS logs in file.
 * Separate BLAS and SCALAPACK examples.
 *
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "DIET_server.h"


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
  int     i, m, n, k;
  double  alpha, beta;
  double *A, *B, *C;

  tA    = (pb->parameters[0].desc.specific.mat.istrans) ? 'T' : 'N';
  tB    = (pb->parameters[1].desc.specific.mat.istrans) ? 'T' : 'N';
  m     = (pb->parameters[0].desc.specific.mat.nb_r);
  n     = (pb->parameters[1].desc.specific.mat.nb_c);
  k     = (pb->parameters[1].desc.specific.mat.nb_r);
  alpha = *((double *) pb->parameters[2].value);
  A     =   (double *) pb->parameters[0].value;
  B     =   (double *) pb->parameters[1].value;
  beta  = *((double *) pb->parameters[3].value);
  C     =   (double *) pb->parameters[4].value;
  
  // DEBUG
  printf("dgemm args : m=%d, n=%d, k=%d, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	 m, n, k, alpha, beta, tA, tB);

  // Set B to identity in case of SUM only
  if ((pb->parameters[7].desc.specific.mat.nb_r == 1)
      && (pb->parameters[7].desc.specific.mat.nb_c == 1)
      && (B[0] == 1)) {
    if (m != n || n != k || m != k) {
      printf("dgemm Error: only square matrices can be summed.\n");
      return 1;
    }
    B = (double *) calloc(m * m, sizeof(double));
    for (i = 0; i < m; i++) {
      B[i + m * i] = 1.0;
    }
  }
  
  printf("Solving dgemm_ ...");
  dgemm_(&tA, &tB, &m, &n, &k, &alpha, A, &m, B, &k, &beta, C, &m);
  printf(" done.\n");

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
    fprintf(stderr, "Usage: server <file>\n");
    return 1;
  }

  diet_service_table_init(3);

  cvt = convertor_alloc("BLAS/dgemm", 3, 4, 4);


  /*
   * Adding dgemm
   */
  /* Set profile */
  profile = profile_desc_alloc(3, 4, 4);
  generic_desc_set(&(profile->param_desc[0]), DIET_SCALAR, DIET_DOUBLE); // alpha
  generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE); // A
  generic_desc_set(&(profile->param_desc[2]), DIET_MATRIX, DIET_DOUBLE); // B
  generic_desc_set(&(profile->param_desc[3]), DIET_SCALAR, DIET_DOUBLE); // beta
  generic_desc_set(&(profile->param_desc[4]), DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY,    1, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY,    2, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,    0, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_IDENTITY,    3, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    4, NULL);
  /* Add */
  diet_service_table_add("dgemm", profile, cvt, solve_dgemm);
  profile_desc_free(profile);


  /*
   * Adding SqMatSUM
   */
  /* Set profile */
  profile = profile_desc_alloc(0, 1, 1);
  generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE); // A
  generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY,    0, NULL);
  {
    double B00 = 1.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    matrix_set(arg, &B00, DIET_VOLATILE, DIET_DOUBLE, 1, 1, 0);
  }
  diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY,   -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,   -1, arg);
  // beta is the same
  diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    1, NULL);
  /* Add */
  diet_service_table_add("SqMatSUM",  profile, cvt, solve_dgemm);
  profile_desc_free(profile);


 /*
   * Adding MatPROD
   */
  /* Set profile */
  profile = profile_desc_alloc(1, 1, 2);
  generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE); // A
  generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE); // B
  generic_desc_set(&(profile->param_desc[2]), DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY,    0, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY,    1, NULL);
  {
    double alpha = 1.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,   -1, arg);
  {
    double beta = 0.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    scalar_set(arg, &beta, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    2, NULL);
  /* Add */
  diet_service_table_add("MatPROD", profile, cvt, solve_dgemm);
  profile_desc_free(profile);


  /*
   * Adding MatScalMult
   */
  /* Set profile */
  profile = profile_desc_alloc(0, 1, 1);
  generic_desc_set(&(profile->param_desc[0]), DIET_SCALAR, DIET_DOUBLE); // beta
  generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE); // C
  /* Set convertor */
  {
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 0, 0, 0);
  }  
  diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY,   -1, arg);
  {
    double alpha = 0.0;
    arg = (diet_arg_t *) malloc(sizeof(diet_arg_t));
    scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,   -1, arg);
  diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_IDENTITY,    0, NULL);
  diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    1, NULL);
  /* Add */
  diet_service_table_add("MatScalMult", profile, cvt, solve_dgemm);
  profile_desc_free(profile);


  print_table();

  convertor_free(cvt);
  
  res = DIET_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


