/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET server for transpose, RSUM a and RPROD       */
/*   problems                                                               */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.9  2002/10/25 14:31:18  ecaron
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.9  2002/10/25 11:00:24  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.8  2002/10/15 18:47:54  pcombes
 * Update to convertor API.
 *
 * Revision 1.7  2002/09/17 15:23:18  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
 *
 * Revision 1.6  2002/09/09 15:57:01  pcombes
 * Update for dgemm and bug fixes
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
 * Revision 1.4  2002/08/09 14:30:33  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 * Revision 1.3  2002/05/24 19:36:53  pcombes
 * Add BLAS/dgemm example (implied bug fixes)
 *
 * Revision 1.2  2002/05/17 20:35:18  pcombes
 * Version alpha without FAST
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

  m = *((int *)pb->parameters[0].value);
  n = *((int *)pb->parameters[1].value);
  A = (double *)pb->parameters[2].value;
  print_matrix(A, m, n);
  
  if ((res = T(m, n, A)))
    return res;
  
  pb->parameters[2].desc.specific.mat.nb_r = n;
  pb->parameters[2].desc.specific.mat.nb_c = m;
  
  print_matrix(A, n, m);
  //printf(" done\n");
  return 0;
}

int
solve_MatSUM(diet_profile_t *pb)
{
  size_t m, n;
  char tA, tB;
  double *A, *B, *C;
  int res;
  
  printf("Solve MatSUM ...");

  tA = *((char *)pb->parameters[0].value);
  tB = *((char *)pb->parameters[1].value);
  m = *((int *)pb->parameters[2].value);
  n = *((int *)pb->parameters[3].value);
  A = (double *)pb->parameters[4].value;
  if ((m != pb->parameters[5].desc.specific.mat.nb_r)
      || (n != pb->parameters[5].desc.specific.mat.nb_c)) {
    fprintf(stderr, "MatSUM error: mA=%d, nA=%d ; mB=%d, nB=%d\n",
	    m, n, pb->parameters[5].desc.specific.mat.nb_r,
	    pb->parameters[5].desc.specific.mat.nb_c);
    return 1;
  }
  A = (double *) pb->parameters[4].value;
  B = (double *) pb->parameters[5].value;
  C = (double *) pb->parameters[6].value;
  pb->parameters[6].desc.specific.mat.istrans = 0;
  
  if ((res = MatSUM(tA, tB, m, n, A, B, C)))
    return res;
  
  printf(" done\n");
  return 0;
}


int
solve_MatPROD(diet_profile_t *pb)
{
  size_t mA, nA, mB, nB;
  char tA, tB;
  double *A, *B, *C;
  
  printf("Solve MatPROD ...");

  tA = *((char *)pb->parameters[0].value);
  tB = *((char *)pb->parameters[1].value);
  mA = *((int *)pb->parameters[2].value);
  nA = *((int *)pb->parameters[3].value);
  mB = *((int *)pb->parameters[5].value);
  nB = *((int *)pb->parameters[6].value);
  if (nA != mB) {
    fprintf(stderr, "MatPROD error: mA=%d, nA=%d ; mB=%d, nB=%d\n", mA, nA, mB, nB);
    return 1;
  }  
  A = (double *) pb->parameters[4].value;
  B = (double *) pb->parameters[7].value;
  C = (double *) pb->parameters[8].value;
  pb->parameters[8].desc.specific.mat.istrans = 0;
  
  MatPROD(tA, tB, mA, nA, A, mB, nB, B, C);
  
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

  diet_profile_desc_t *profile;
  diet_convertor_t *cvt;
  
  if (argc < 3) {
    fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
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
      fprintf(stderr, "Usage: SeD <file> [T][MatSUM][MatPROD]\n");
      exit(1);
    }
  }
  

  /* This server can solve 3 pbs:  */
  /*   - T = matrix translation    */
  /*   - MatSUM = matrix addition  */
  /*   - MatPROD = matrix product  */

  diet_service_table_init(3);
  
  if (services[0]) {
    profile = profile_desc_alloc(-1, 0, 0);
    generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE);

    cvt = convertor_alloc("T", 1, 2, 2);
    diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_NB_ROW, 0, NULL);
    diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_NB_COL, 0, NULL);
    diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY,   0, NULL);
		     
    diet_service_table_add(SRV[0], profile, cvt, solve_T);
    profile_desc_free(profile);
    convertor_free(cvt);
  }
  
  if (services[1] || services[2]) {
    profile = profile_desc_alloc(1, 1, 2);
    generic_desc_set(&(profile->param_desc[0]), DIET_MATRIX, DIET_DOUBLE);
    generic_desc_set(&(profile->param_desc[1]), DIET_MATRIX, DIET_DOUBLE);
    generic_desc_set(&(profile->param_desc[2]), DIET_MATRIX, DIET_DOUBLE);

    if (services[1]) {
      cvt = convertor_alloc(SRV[1], 5, 5, 6);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_ISTRANS, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_ISTRANS, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_MAT_NB_ROW,  0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_MAT_NB_COL,  0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[5]), DIET_CVT_IDENTITY,    1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[6]), DIET_CVT_IDENTITY,    2, NULL);

      diet_service_table_add(SRV[1], profile, cvt, solve_MatSUM);
    }
    if (services[2]) {
      cvt = convertor_alloc(SRV[2], 7, 7, 8);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_MAT_ISTRANS, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_MAT_ISTRANS, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_MAT_NB_ROW,  0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[3]), DIET_CVT_MAT_NB_COL,  0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[4]), DIET_CVT_IDENTITY,    0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[5]), DIET_CVT_MAT_NB_ROW,  1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[6]), DIET_CVT_MAT_NB_COL,  1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[7]), DIET_CVT_IDENTITY,    1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[8]), DIET_CVT_IDENTITY,    2, NULL);

      diet_service_table_add(SRV[2], profile, cvt, solve_MatPROD);
    }

    profile_desc_free(profile);
    convertor_free(cvt);
  }
  
  print_table();
  res = DIET_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}
