/****************************************************************************/
/* dmat_manips example: a DIET server for transpose, MatPROD, MatSUM,       */
/*   SqMatSUM, SqMatSUM_opt services.                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.24  2003/09/10 12:43:06  pcombes
 * Compatibility between the convertors and FAST 0.4.3 and FAST 0.8.
 *
 * Revision 1.23  2003/08/09 17:32:47  pcombes
 * Update to the new diet_profile_desc_t.
 *
 * Revision 1.22  2003/07/04 09:48:05  pcombes
 * Fix bug in getting arguments (only 1 service was declred).
 *
 * Revision 1.19  2003/02/07 17:05:23  pcombes
 * Add SqMatSUM_opt with the new convertor API.
 * Use diet_free_data to properly free user's data.
 *
 * Revision 1.18  2003/01/23 19:13:45  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.17  2003/01/22 17:13:44  pcombes
 * Use DIET_SERVER_LIBS and DIET_CLIENT_LIBS
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/**
 * As the services base/mult and base/plus are hardcoded in FAST 0.4.x, we
 * include DIET_config.h here, to access the __FAST_O_4__ macro.
 */
#include "DIET_config.h"

#include "DIET_server.h"
#include "progs.h"

/* This server can offer 5 services (chosen by at launch time):      */
/*   - T = matrix translation                                        */
/*   - MatSUM = matrix addition (and thus SqMatSUM and SqMatSUM_opt) */
/*   - MatPROD = matrix product                                      */

#define NB_SRV 5
static const char* SRV[NB_SRV] =
  {"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};


/*
 * SOLVE FUNCTIONS
 */

int
solve_T(diet_profile_t* pb)
{
  size_t* m = NULL;
  size_t* n = NULL;
  double* A = NULL;
  diet_matrix_order_t o;
  int res;

  printf("Solve T ...");

  diet_scalar_get(diet_parameter(pb,0), &m, NULL);
  diet_scalar_get(diet_parameter(pb,1), &n, NULL);
  diet_matrix_get(diet_parameter(pb,2), &A, NULL, NULL, NULL, &o);
  
  if ((res = T(*m, *n, A, (o == DIET_ROW_MAJOR))))
    return res;
  
  // no need to set order
  diet_matrix_desc_set(diet_parameter(pb,2), *n, *m, DIET_MATRIX_ORDER_COUNT);
  
  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));

  printf(" done\n");
  return 0;
}

int
solve_MatSUM(diet_profile_t* pb)
{
  size_t mA, nA, mB, nB, mC, nC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int arg_idx, res;
  
  printf("Solve MatSUM ...");


#if defined(__FAST_0_4__)
  /**
   * In FAST 0.4.x, base/plus is declared to the LDIF base with only the two
   * matrix dimensions as arguments. But the library expects the user to give
   * two matrices, and it extracts itself their dimensions.
   */
  arg_idx = 0;

#else  // __FAST_0_4__
  /**
   * In FAST 0.8 and later, base/plus is also declared to the LDIF base with
   * only the two matrix dimensions as arguments. But there is no more
   * extraction of the dimensions, and then the two first arguments must be
   * these dimensions.
   */
  arg_idx = 2;

#endif // __FAST_0_4__

  diet_matrix_get(diet_parameter(pb,arg_idx), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,(arg_idx+1)), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,(arg_idx+2)), &C, NULL, &mC, &nC, &oC);

  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatSUM(tB, tA, nA, mA, B, A, C);
  } else {
    res = MatSUM(tA, tB, mA, nA, A, B, C);
  }
  if (res)
    return res;
  
  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
#if !defined(__FAST_0_4__)
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));
#endif // ! __FAST_0_4__

  printf(" done\n");
  return res;
}


int
solve_MatPROD(diet_profile_t* pb)
{
  size_t mA, nA, mB, nB;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int arg_idx, res;
  
  printf("Solve MatPROD ...");

#if defined(__FAST_0_4__)
  /**
   * In FAST 0.4.x, base/mult is declared to the LDIF base with only the three
   * relevant matrix dimensions as arguments. But the library expects the user
   * to give two matrices, and it extracts itself their dimensions.
   */
  arg_idx = 0;

#else  // __FAST_0_4__
  /**
   * In FAST 0.8 and later, base/plus is also declared to the LDIF base with
   * only the three relevant matrix dimensions as arguments. But there is no
   * more extraction of the dimensions, and then the three first arguments must
   * be these dimensions.
   */
  arg_idx = 3;

#endif // __FAST_0_4__

  diet_matrix_get(diet_parameter(pb,arg_idx), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,(arg_idx+1)), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,(arg_idx+2)), &C, NULL, NULL, NULL, &oC);
  
  
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatPROD(tB, tA, nB, mB, B, mA, A, C);
  } else {
    res = MatPROD(tA, tB, mA, nA, A, nB, B, C);
  }

  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));
#if !defined(__FAST_0_4__)
  diet_free_data(diet_parameter(pb,2));
  diet_free_data(diet_parameter(pb,3));
  diet_free_data(diet_parameter(pb,4));
#endif // ! __FAST_0_4__

  printf(" done\n");
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> [all | [%s][%s][%s][%s][%s] ]\n",
	  cmd, SRV[0], SRV[1], SRV[2], SRV[3], SRV[4]);
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  size_t i,j;
  int res;
  int services[NB_SRV] = {0, 0, 0, 0, 0};

  diet_profile_desc_t* profile = NULL;
  diet_convertor_t*    cvt     = NULL;
  
  if (argc < 3) {
    return usage(argv[0]);
  }  
  
  for (i = 2; i < argc; i++) {
    char* path = argv[i];
    if (!strcmp("all", path)) {
      for (j = 0; j < NB_SRV; (services[j++] = 1));
      break;
    } else {
      for (j = 0; j < NB_SRV; j++) {
	if (!strcmp(SRV[j], path)) {
	  services[j] = 1;
	  break;
	}
      }
      if (j == NB_SRV)
	exit(usage(argv[0]));
    }
  }
  
  diet_service_table_init(NB_SRV);
  
  if (services[0]) {
    profile = diet_profile_desc_alloc(SRV[0], -1, 0, 0);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    /**
     * Example of convertor:
     *  solve_T assumes that the profile is (IN m, IN n, INOUT A), but we
     *  declare a simpler service as (INOUT A). The convertor will let DIET
     *  extract automatically m and n before calling solve_T.
     */
    cvt = diet_convertor_alloc("T", 1, 2, 2);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
    diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 0, NULL);
    
    diet_service_table_add(profile, cvt, solve_T);
    diet_profile_desc_free(profile);
    diet_convertor_free(cvt);
  }
  
  if (services[1] || services[2] || services[3]) {
    const char* path = "still undefined";
    profile = diet_profile_desc_alloc(path, 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);

#if defined(__FAST_0_4__)
    /**
     * As FAST 0.4.x performs the conversion matrices -> dimensions, there is no
     * convertor to define for the arguments. But the names of the services
     * offered differ from the names used in the LDIF base. So let us define
     * convertors that only convert the paths.
     */
    cvt = diet_convertor_alloc("still undefined", 1, 1, 2);
    diet_arg_cvt_short_set(&(cvt->arg_convs[0]), 0, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[1]), 1, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 2, NULL);

    for (i = 1; i <= 3; i++) {
      if (services[i]) {
	free(profile->path);
	profile->path = strdup(SRV[i]);
	free(cvt->path);
	cvt->path = strdup((i == 1) ? "base/mult" : "base/plus");
	if (diet_service_table_add(profile, cvt,
				   (i == 1) ? solve_MatPROD : solve_MatSUM))
	  return 1;
      }
    }
    
#else  // __FAST_0_4__
    {
      diet_convertor_t* cvt_SUM = NULL;
      /**
       * solve_MatPROD assumes that the profile is
       * (IN mA, IN nA, IN nB, IN A, IN B, OUT C), but we declare a simpler
       * service as (IN A, IN B, OUT C). The convertor will let DIET extract
       * automatically mA, nA and nB before calling solve_MatPROD.
       */
      cvt = diet_convertor_alloc("base/mult", 4, 4, 5);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_MAT_NB_COL, 1, NULL, 1);
      diet_arg_cvt_short_set(&(cvt->arg_convs[3]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[4]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[5]), 2, NULL);

      /**
       * solve_MatSUM assumes that the profile is
       * (IN mA, IN nA, IN A, IN B, OUT C), but we declare simpler services as
       * (IN A, IN B, OUT C). The convertor will let DIET extract automatically
       * mA and nA before calling solve_MatSUM.
       */
      cvt_SUM = diet_convertor_alloc("base/plus", 3, 3, 4);
      diet_arg_cvt_set(&(cvt_SUM->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
      diet_arg_cvt_set(&(cvt_SUM->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[2]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[3]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt_SUM->arg_convs[4]), 2, NULL);

      for (i = 1; i <= 3; i++) {
	if (i == 2) {
	  diet_convertor_free(cvt);
	  cvt = cvt_SUM;
	}
	if (services[i]) {
	  free(profile->path);
	  profile->path = strdup(SRV[i]);
	  if (diet_service_table_add(profile, cvt,
				     (i == 1) ? solve_MatPROD : solve_MatSUM))
	    return 1;
	}
      }
    }
#endif // __FAST_0_4__

    diet_convertor_free(cvt);
    diet_profile_desc_free(profile);

  } // if (services[1] || services[2] || services[3])


  if (services[4]) {
    profile = diet_profile_desc_alloc(SRV[4], 0, 1, 1);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);

#if defined(__FAST_0_4__)
    /* Profile expected by solve_MatSUM: (IN A, IN B, OUT C) */
    cvt = diet_convertor_alloc("base/plus", 1, 1, 2);
    i = 0;
#else  // __FAST_0_4__
    /* Profile expected by solve_MatSUM: (IN mA, IN nA, IN A, IN B, OUT C)
     * Thus, we must extract the first two dimensions. */
    cvt = diet_convertor_alloc("base/plus", 3, 3, 4);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL, 0);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL, 0);
    i = 2;
#endif // __FAST_0_4__
    /**
     * As SqMatSUM_opt is declared with the profile (IN A, INOUT B) (to match a
     * sub-service of the BLAS dgemm), the convertor must tell DIET to
     * "duplicate" the (i+2)th argument. Actually, "duplicate" concerns only the
     * description of the argument, not the memory space needed for the whole
     * matrices.
     */
    diet_arg_cvt_short_set(&(cvt->arg_convs[i]), 0, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[i+1]), 1, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[i+2]), 1, NULL);
    if (diet_service_table_add(profile, cvt, solve_MatSUM))
      return 1;
    diet_convertor_free(cvt);
    diet_profile_desc_free(profile);    
  }

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}
