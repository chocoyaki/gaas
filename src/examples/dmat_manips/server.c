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
  int res;
  
  printf("Solve MatSUM ...");

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,2), &C, NULL, &mC, &nC, &oC);

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

  printf(" done\n");
  return 0;
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
  
  printf("Solve MatPROD ...");

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb,2), &C, NULL, NULL, NULL, &oC);
  
  
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    MatPROD(tB, tA, nB, mB, B, mA, A, C);
  } else {
    MatPROD(tA, tB, mA, nA, A, nB, B, C);
  }

  diet_free_data(diet_parameter(pb,0));
  diet_free_data(diet_parameter(pb,1));

  printf(" done\n");
  return 0;
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
    const char* path =
      (services[1]) ? SRV[1] : ((services[2]) ? SRV[2] : SRV[3]);
    profile = diet_profile_desc_alloc(path, 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);

    if (services[1]) {
      cvt = diet_convertor_alloc("base/mult", 1, 1, 2);
      diet_arg_cvt_short_set(&(cvt->arg_convs[0]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[1]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 2, NULL);
      if (diet_service_table_add(profile, cvt, solve_MatPROD)) return 1;
      diet_convertor_free(cvt);
    }
    if (services[2] || services[3]) {
      cvt = diet_convertor_alloc("base/plus", 1, 1, 2);
      diet_arg_cvt_short_set(&(cvt->arg_convs[0]), 0, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[1]), 1, NULL);
      diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 2, NULL);
      if (diet_service_table_add(profile, cvt, solve_MatSUM)) return 1;
      diet_convertor_free(cvt);
    }

    diet_profile_desc_free(profile);
  }

  if (services[4]) {
    profile = diet_profile_desc_alloc(SRV[4], 0, 1, 1);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
    cvt = diet_convertor_alloc("base/plus", 1, 1, 2);
    diet_arg_cvt_short_set(&(cvt->arg_convs[0]), 0, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[1]), 1, NULL);
    diet_arg_cvt_short_set(&(cvt->arg_convs[2]), 1, NULL);
    if (diet_service_table_add(profile, cvt, solve_MatSUM)) return 1;
    diet_convertor_free(cvt);
    diet_profile_desc_free(profile);    
  }

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}
