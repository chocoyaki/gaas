/****************************************************************************/
/* $Id$             */
/* DIET users'day server example (TP - exo3)                                */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

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

  tA = (pb->parameters[0].desc.specific.mat.istrans) ? 'T' : 'N';
  tB = (pb->parameters[1].desc.specific.mat.istrans) ? 'T' : 'N';
  m  = pb->parameters[0].desc.specific.mat.nb_r;
  n  = pb->parameters[0].desc.specific.mat.nb_c;
  A  = (double *) pb->parameters[0].value;
  B  = (double *) pb->parameters[1].value;
  if ((m != pb->parameters[1].desc.specific.mat.nb_r)
      || (n != pb->parameters[1].desc.specific.mat.nb_c)) {
    fprintf(stderr, "MatSUM error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)m, (long)n, (long)pb->parameters[1].desc.specific.mat.nb_r,
	    (long)pb->parameters[1].desc.specific.mat.nb_c);
    return 1;
  }
  C = (double *) pb->parameters[2].value;
  pb->parameters[2].desc.specific.mat.istrans = 0;
  
  if ((res = MatSUM(tA, tB, m, n, A, B, C)))
    return res;
  
  printf(" done\n");
  return 0;
}


int
solve_MatPROD(diet_profile_t *pb)
{
  size_t mA, nA, nB;
  char tA, tB;
  double *A, *B, *C;
  
  printf("Solve MatPROD ...");

  tA = (pb->parameters[0].desc.specific.mat.istrans) ? 'T' : 'N';
  tB = (pb->parameters[1].desc.specific.mat.istrans) ? 'T' : 'N';
  mA = pb->parameters[0].desc.specific.mat.nb_r;
  nA = pb->parameters[0].desc.specific.mat.nb_c;
  nB = pb->parameters[1].desc.specific.mat.nb_c;
  A = (double *) pb->parameters[0].value;
  B = (double *) pb->parameters[1].value;
  if (nA != pb->parameters[1].desc.specific.mat.nb_r) {
    fprintf(stderr, "MatPROD error: mA=%ld, nA=%ld ; mB=%ld, nB=%ld\n",
	    (long)mA, (long)nA, (long)pb->parameters[1].desc.specific.mat.nb_r, (long)nB);
    return 1;
  }  
  C = (double *) pb->parameters[2].value;
  pb->parameters[2].desc.specific.mat.istrans = 0;
  
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
      cvt = convertor_alloc("base/plus", 1, 1, 2);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY, 2, NULL);
      diet_service_table_add(SRV[1], profile, cvt, solve_MatSUM);
      convertor_free(cvt);
    }
    if (services[2]) {
      cvt = convertor_alloc("base/mult", 1, 1, 2);
      diet_arg_cvt_set(&(cvt->arg_convs[0]), DIET_CVT_IDENTITY, 0, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[1]), DIET_CVT_IDENTITY, 1, NULL);
      diet_arg_cvt_set(&(cvt->arg_convs[2]), DIET_CVT_IDENTITY, 2, NULL);
      diet_service_table_add(SRV[2], profile, cvt, solve_MatPROD);
      convertor_free(cvt);
    }

    profile_desc_free(profile);
  }
  
  print_table();
  res = DIET_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}
