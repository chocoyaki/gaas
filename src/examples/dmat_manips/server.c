/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET server for transpose, RSUM a and RPROD       */
/*   problems                                                               */
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
 * Revision 1.17  2003/01/22 17:13:44  pcombes
 * Use DIET_SERVER_LIBS and DIET_CLIENT_LIBS
 *
 * Revision 1.16  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.11  2002/11/15 17:15:32  pcombes
 * FAST integration complete ...
 *
 * Revision 1.10  2002/11/07 18:42:42  pcombes
 * Add includes and configured Makefile variables to install directory.
 * Update dgemm to the implementation that is hardcoded in FAST.
 *
 * Revision 1.8  2002/10/15 18:47:54  pcombes
 * Update to convertor API.
 *
 * Revision 1.7  2002/09/17 15:23:18  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
 *
 * Revision 1.5  2002/08/30 16:50:16  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 *
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"
#include "progs.h"

/*
 * SOLVE FUNCTIONS
 */

int
solve_T(diet_profile_t *pb)
{
  size_t m, n;
  double *A;
  int res;

  //printf("Solve T ...");

  //m = *((int *)pb->parameters[0].value);
  diet_scalar_get(diet_parameter(pb,0), &m, NULL);
  //n = *((int *)pb->parameters[1].value);
  diet_scalar_get(diet_parameter(pb,1), &n, NULL);
  //A = (double *)pb->parameters[2].value;
  /* This is equivalent to
     A = diet_value(double, diet_parameter(pb,2)); */
  diet_matrix_get(diet_parameter(pb,2), &A, NULL, NULL, NULL, NULL);
  print_matrix(A, m, n);
  
  if ((res = T(m, n, A)))
    return res;
  
  //pb->parameters[2].desc.specific.mat.nb_r = n;
  //pb->parameters[2].desc.specific.mat.nb_c = m;
  // no need to set order
  diet_matrix_desc_set(diet_parameter(pb,2), n, m, -1);
  
  print_matrix(A, n, m);
  //printf(" done\n");
  return 0;
}

int
solve_MatSUM(diet_profile_t *pb)
{
  size_t mA, nA, mB, nB;
  int tA, tB;
  double *A, *B, *C;
  int res;
  
  printf("Solve MatSUM ...");

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &mA, &nA, &tA);
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &mB, &nB, &tB);
  if ((mA != mB) || (nA != nB)) {
    fprintf(stderr, "MatSUM error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  //C = (double *) pb->parameters[2].value;  OR
  //diet_matrix_get(diet_parameter(pb,2), &C, NULL, NULL, NULL, NULL); OR
  C = diet_value(double,diet_parameter(pb,2));
  
  if ((res = MatSUM(tA, tB, mA, nA, A, B, C)))
    return res;
  
  printf(" done\n");
  return 0;
}


int
solve_MatPROD(diet_profile_t *pb)
{
  size_t mA, nA, mB, nB;
  int tA, tB;
  double *A, *B, *C;
  
  printf("Solve MatPROD ...");

  diet_matrix_get(diet_parameter(pb,0), &A, NULL, &mA, &nA, &tA);
  diet_matrix_get(diet_parameter(pb,1), &B, NULL, &mB, &nB, &tB);
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
    (long)mA, (long)nA, (long)mB, (long)nB);
    return 1;
  }
  //diet_matrix_get(diet_parameter(pb,2), &C, NULL, NULL, NULL, NULL); OR
  C = diet_value(double,diet_parameter(pb,2));
  
  MatPROD(tA, tB, mA, nA, A, nB, B, C);
  
  printf(" done\n");
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
  char *SRV[3]    = {"T", "MatSUM", "MatPROD"};
  int services[3];

  diet_profile_desc_t *profile = NULL;
  diet_convertor_t    *cvt     = NULL;
  
  if (argc < 3) {
    fprintf(stderr, "Usage: %s <file.cfg> [T][MatSUM][MatPROD]\n", argv[0]);
    return 1;
  }  
  
  for (i = 0; i < 3; i++) {
    services[i] = 0;
  }

  for (i = 2; i < argc; i++) {
    char *path = argv[i];
    if (!strcmp(SRV[0], path))
      services[0] = 1;
    else if (!strcmp(SRV[1], path))
      services[1] = 1;
    else if (!strcmp(SRV[2], path))
      services[2] = 1;
    else {
      fprintf(stderr, "Usage: %s <file.cfg> [T][MatSUM][MatPROD]\n", argv[0]);
      exit(1);
    }
  }
  

  /* This server can solve 3 pbs:  */
  /*   - T = matrix translation    */
  /*   - MatSUM = matrix addition  */
  /*   - MatPROD = matrix product  */

  diet_service_table_init(3);
  
  if (services[0]) {
    profile = diet_profile_desc_alloc(-1, 0, 0);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    cvt = diet_convertor_alloc("T", 1, 2, 2);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL);
    diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,   0, NULL);
		     
    diet_service_table_add(SRV[0], profile, cvt, solve_T);
    diet_profile_desc_free(profile);
    diet_convertor_free(cvt);
  }
  
  if (services[1] || services[2]) {
    profile = diet_profile_desc_alloc(1, 1, 2);
    diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
    diet_generic_desc_set(diet_param_desc(profile,2), DIET_MATRIX, DIET_DOUBLE);

    if (services[1]) {
      cvt = diet_convertor_alloc("base/plus", 1, 1, 2);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY, 2, NULL);
      diet_service_table_add(SRV[1], profile, cvt, solve_MatSUM);
      diet_convertor_free(cvt);
    }
    if (services[2]) {
      cvt = diet_convertor_alloc("base/mult", 1, 1, 2);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY, 2, NULL);
      diet_service_table_add(SRV[2], profile, cvt, solve_MatPROD);
      diet_convertor_free(cvt);
    }

    diet_profile_desc_free(profile);
  }
  
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}
