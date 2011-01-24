/****************************************************************************/
/* DIET server for BLAS functions                                           */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Yves Caniou (Yves.Caniou@ens-lyon.fr)                               */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.15  2011/01/24 23:14:19  bdepardo
 * Fixed memory leaks
 *
 * Revision 1.14  2010/03/05 15:52:07  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.13  2006/06/30 15:26:02  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.12  2006/04/22 19:21:33  ecaron
 * Update fprintf format according to size_t
 *
 * Revision 1.11  2006/03/03 14:47:46  ycaniou
 * Change int in size_t to compile well
 *
 * Revision 1.10  2005/06/24 15:03:59  ycaniou
 * NB_SRV=5 services and diet_service_table_init(3) corrected
 *
 * Revision 1.9  2003/08/09 17:32:47  pcombes
 * Update to the new diet_profile_desc_t.
 *
 * Revision 1.8  2003/04/10 13:33:21  pcombes
 * Apply new Coding Standards.
 *
 * Revision 1.7  2003/02/07 17:05:23  pcombes
 * Add SqMatSUM_opt with the new convertor API.
 * Use diet_free_data to properly free user's data.
 *
 * Revision 1.6  2003/01/23 19:13:44  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.5  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.3  2002/12/18 12:11:42  jylexcel
 * SqMatSUM bug fix related to the use of convertors. Philippe+JY.
 *
 * Revision 1.2  2002/12/12 18:17:04  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
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
extern void
dgemm_(char*   tA,    char* tB,  size_t* m,   size_t* n,    size_t* k,
       double* alpha, double* A, size_t* lda, double* B, size_t* ldb,
       double* beta,  double* C, size_t* ldc);


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


/* This server can offer 5 services (chosen by user at launch time): */
/*   - T = matrix translation                                        */
/*   - MatSUM = matrix addition (and thus SqMatSUM and SqMatSUM_opt) */
/*   - MatPROD = matrix product                                      */

#define NB_SRV 5
static const char* SRV[NB_SRV] =
  {"dgemm", "MatPROD", "SqMatSUM", "SqMatSUM_opt", "MatScalMult"};


/*
 * SOLVE FUNCTION for dgemm_
 * C MUST BE COLUMN-MAJOR
 */

int
solve_dgemm(diet_profile_t* pb)
{
  char    tA, tB;
  diet_matrix_order_t oA, oB, oC;
  size_t  k, k_, m, m_, n, n_;
  double* alpha = NULL;
  double* beta  = NULL;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int     IsSqMatSUM = 0;

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &m, &k, &oA);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &k_, &n, &oB);
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  diet_scalar_get(diet_parameter(pb,2), &alpha, NULL);
  diet_scalar_get(diet_parameter(pb,3), &beta,  NULL);
  diet_matrix_get(diet_parameter(pb,4), &C, NULL, &m_, &n_, &oC);
  
  /* A and B NULL => MatScalMult, then m and n are wrong
   *  only B NULL => SqMatSUM, then set B to identity (and k_ for later test) */
  if (!A && !B) {
    m = m_;
    n = n_;
    k = (m > n) ? m : n;
    k_ = k;
  }
  if ((IsSqMatSUM = (A && !B))) {
    size_t  i;
    k_ = k;
    n = n_;
    if (m != n || n != k || m != k) {
      fprintf(stderr, "dgemm Error: only square matrices can be summed.\n");
      return 1;
    }
    B = (double*) calloc(m * m, sizeof(double));
    for (i = 0; i < m; i++) {
      B[i + m * i] = 1.0;
    }
  }

  if ((k_ != k) || (m_ != m) || (n_ != n)) {
    fprintf(stderr, "dgemm Error: invalid matrix dimensions: ");
    fprintf(stderr, "%dx%d = %dx%d * %dx%d\n", (int)m_, (int)n_, (int)m, (int)k, (int)k_, (int)n);
    return 1;
  }

  /* DEBUG */
  
  printf("Solving dgemm_ ...");
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    fprintf(stderr,
	    "dgemm args : m=%zd, n=%zd, k=%zd, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	    n, m, k, *alpha, *beta, tB, tA);
    dgemm_(&tB, &tA, &n, &m, &k, alpha,
	   B, (tB == 'T') ? &k : &n,
	   A, (tA == 'T') ? &m : &k,
	   beta, C, &n);  
  } else {
    fprintf(stderr,
	    "dgemm args : m=%zd, n=%zd, k=%zd, alpha=%f, beta=%f, tA=%c, tB=%c\n",
	    m, n, k, *alpha, *beta, tA, tB);
    dgemm_(&tA, &tB, &m, &n, &k, alpha,
	   A, (tA == 'T') ? &k : &m,
	   B, (tB == 'T') ? &n : &k,
	   beta, C, &m);
  }
  printf(" done.\n");

  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));

  if (IsSqMatSUM) {
    free(B);
  }

  return 0;
}

/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  int res;
  diet_profile_desc_t* profile = NULL;
  diet_convertor_t* cvt = NULL;
  diet_arg_t* arg = NULL;

  if (argc != 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  diet_service_table_init(NB_SRV);

  cvt = diet_convertor_alloc("BLAS/dgemm", 3, 4, 4);


  /*
   * Adding dgemm
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[0], 3, 4, 4);
  diet_generic_desc_set(diet_param_desc(profile,0),
			DIET_SCALAR, DIET_DOUBLE); /* alpha */
  diet_generic_desc_set(diet_param_desc(profile,1),
			DIET_MATRIX, DIET_DOUBLE); /* A */
  diet_generic_desc_set(diet_param_desc(profile,2),
			DIET_MATRIX, DIET_DOUBLE); /* B */
  diet_generic_desc_set(diet_param_desc(profile,3),
			DIET_SCALAR, DIET_DOUBLE); /* beta */
  diet_generic_desc_set(diet_param_desc(profile,4),
			DIET_MATRIX, DIET_DOUBLE); /* C */
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 1, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), 2, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), 0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), 3, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), 4, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatPROD
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[1], 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), 1, NULL);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  {
    double beta = 0.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &beta, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4),  2, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM (IN, IN, OUT)
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[2], 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor
     NB: the difficulty here is that the INOUT parameter of the dgemm
         corresponds to the second IN AND the OUT parameter of the profile. */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE,
		    DIET_DOUBLE, 0, 0, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  /* beta is the same */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  /* Use all arguments of diet_arg_cvt_set, since out_arg_idx == 2 (the OUT
     parameter of the profile) and in_arg_idx == 1 (the second IN parameter).
  */
  diet_arg_cvt_set(diet_arg_conv(cvt,4), DIET_CVT_IDENTITY, 1, NULL, 2);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding SqMatSUM_opt (IN, INOUT)
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[3], 0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), 0, NULL);
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_matrix_set(arg, NULL, DIET_VOLATILE,
		    DIET_DOUBLE, 0, 0, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 1.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  /* beta is the same */
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4), 1, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);


  /*
   * Adding MatScalMult
   */
  /* Set profile */
  profile = diet_profile_desc_alloc(SRV[4], 0, 1, 1);
  /* beta and C */
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  /* Set convertor */
  {
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    /* 1x1 matrix (to force dimensions set, because 0 is ignored) */
    diet_matrix_set(arg, NULL, DIET_VOLATILE, DIET_DOUBLE, 1, 1, DIET_COL_MAJOR);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,0), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,1), -1, arg);
  {
    double alpha = 0.0;
    arg = (diet_arg_t*) calloc(1, sizeof(diet_arg_t));
    diet_scalar_set(arg, &alpha, DIET_VOLATILE, DIET_DOUBLE);
  }
  diet_arg_cvt_short_set(diet_arg_conv(cvt,2), -1, arg);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,3),  0, NULL);
  diet_arg_cvt_short_set(diet_arg_conv(cvt,4),  1, NULL);
  /* Add */
  if (diet_service_table_add(profile, cvt, solve_dgemm)) return 1;
  diet_profile_desc_free(profile);

  /* The same cvt has been used for all services, free it now */
  diet_convertor_free(cvt);
  
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}


