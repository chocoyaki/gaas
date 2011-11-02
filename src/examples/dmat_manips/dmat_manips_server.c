/**
* @file dmat_manips_server.c
* 
* @brief  DIET server for transpose, MatSUM and MatPROD SqMatSUM, SqMatSUM_opt 
* 
* @author  - Philippe COMBES (Philippe.Combes@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.2  2011/01/23 19:20:00  bdepardo
 * Fixed memory and resources leaks, variables scopes, unread variables
 *
 * Revision 1.1  2010/08/06 14:25:29  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.31  2010/03/31 02:24:47  ycaniou
 * Warnings--
 * Remove unecessary includes
 *
 * Revision 1.30  2010/03/23 12:44:18  glemahec
 * Correction des exemples pour DAGDA
 *
 * Revision 1.29  2010/03/05 15:52:08  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.28  2006/11/16 09:55:55  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.27  2006/06/30 15:26:03  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.25  2005/05/27 08:18:17  mjan
 * Moving JuxMem in a more appropriate place (src/utils)
 * Added log messages for VizDIET
 * Added use of JuxMem in the client side
 *
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

#include "DIET_server.h"
#include "progs.h"

/* This server can offer 5 services (chosen by at launch time):      */
/*   - T = matrix translation                                        */
/*   - MatSUM = matrix addition (and thus SqMatSUM and SqMatSUM_opt) */
/*   - MatPROD = matrix product                                      */

#define NB_SRV 3
static const char* SRV[NB_SRV] =
  {"T", "MatPROD", "MatSUM"};

/*
 * SOLVE FUNCTIONS
 */

int
solve_T(diet_profile_t* pb)
{
  size_t m;
  size_t n;
  double* A = NULL;
  double* C = NULL;
  diet_matrix_order_t o;
  int res, i;

  printf("Solve T ...");

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &m, &n, &o);
  C = malloc(n*m*sizeof(double));
  for (i = 0; i<n*m; ++i)
    C[i]=A[i];
  if ((res = T(m, n, A, (o == DIET_ROW_MAJOR)))) {
    free(C);
    return res;
  }
  
  /* no need to set order */
  diet_matrix_set(diet_parameter(pb, 1), C, DIET_VOLATILE, DIET_DOUBLE, n, m, o);
  print_matrix(A, m, n, o);

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

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb, 1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%zd, nA=%zd; mB=%zd, nB=%zd\n",
            mA, nA, mB, nB);
    return 1;
  }
  
  diet_matrix_get(diet_parameter(pb, 2), &C, NULL, &mC, &nC, &oC);
  C = calloc(mC*nC, sizeof *C);
  
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatSUM(tB, tA, nA, mA, B, A, C);
  } else {
    res = MatSUM(tA, tB, mA, nA, A, B, C);
  }
  diet_matrix_set(diet_parameter(pb, 2), C, DIET_VOLATILE, DIET_DOUBLE, mC, nC, oC);
  if (res)
    return res;

  printf(" done\n");
  return res;
}


int
solve_MatPROD(diet_profile_t* pb)
{
  size_t mA, nA, mB, nB, nC, mC;
  char tA, tB;
  diet_matrix_order_t oA, oB, oC;
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  int res;
  
  printf("Solve MatPROD ...");

  diet_matrix_get(diet_parameter(pb, 0), &A, NULL, &mA, &nA, &oA);
  diet_matrix_get(diet_parameter(pb, 1), &B, NULL, &mB, &nB, &oB);
  tA = (oA == DIET_ROW_MAJOR) ? 'T' : 'N';
  tB = (oB == DIET_ROW_MAJOR) ? 'T' : 'N';
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld; mB=%ld, nB=%ld\n",
            (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  diet_matrix_get(diet_parameter(pb, 2), &C, NULL, &mC, &nC, &oC);
  C = calloc(mC*nC, sizeof *C);
  
  if (oC == DIET_ROW_MAJOR) {
    tA = (tA == 'T') ? 'N' : 'T';
    tB = (tB == 'T') ? 'N' : 'T';
    res = MatPROD(tB, tA, nB, mB, B, mA, A, C);
  } else {
    res = MatPROD(tA, tB, mA, nA, A, nB, B, C);
  }

  diet_matrix_set(diet_parameter(pb, 2), C, DIET_VOLATILE, DIET_DOUBLE, mC, nC, oC);

  printf(" done\n");
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> [all | [%s][%s][%s] ]\n",
          cmd, SRV[0], SRV[1], SRV[2]);
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  size_t i, j;
  int res;
  int services[NB_SRV] = {0, 0, 0};
  diet_profile_desc_t* profile = NULL;
  
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
    profile = diet_profile_desc_alloc("T", 0, 0, 1);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);

    diet_service_table_add(profile, NULL, solve_T);
    diet_profile_desc_free(profile);
  }
  
  if (services[1]) {
    profile = diet_profile_desc_alloc("MatPROD", 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 2), DIET_MATRIX, DIET_DOUBLE);

    diet_service_table_add(profile, NULL, solve_MatPROD);
    diet_profile_desc_free(profile);
  }
  
  if (services[2]) {
    profile = diet_profile_desc_alloc("MatSUM", 1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile, 0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile, 2), DIET_MATRIX, DIET_DOUBLE);
    
    diet_service_table_add(profile, NULL, solve_MatSUM);
    diet_profile_desc_free(profile);
  } 

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
