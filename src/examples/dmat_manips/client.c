/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
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


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"
#include "slimfast_api_local.h"


#define print_matrix(mat, m, n)            \
  {                                        \
    size_t i, j;                           \
    printf("%s = \n", #mat);               \
    for (i = 0; i < (m); i++) {            \
      for (j = 0; j < (n); j++) {          \
	printf("%3f ", (mat)[i*(n) + j]);  \
      }                                    \
      printf("\n");                        \
    }                                      \
    printf("\n");                          \
  }


/* argv[1]: client config file path
   argv[2]: trans, rsum, or rprod   */

int
main(int argc, char **argv)
{
  char *path;
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;
  double mat1[6] = {1.0,2.0,3.0,4.0,5.0,6.0};
  double mat2[6] = {7.0,8.0,9.0,10.0,11.0,12.0};
  double *A, *B, *C;

  char *PB[3] = {"T", "MatSUM", "MatPROD"};

  if (argc != 3) {
    fprintf(stderr, "Usage: client <file> [T|MatSUM|MatPROD]\n");
    return 1;
  }
  diet_initialize(argc, argv, argv[1]);

  path = strdup(argv[2]);
  
  A = mat1;
  B = mat2;

  if (!strcmp(path, PB[0])) {
    
    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(-1, 0, 0);
    matrix_set(&(profile->parameters[0]), A, DIET_VOLATILE,
	       DIET_DOUBLE, 3, 2, 0);
    print_matrix((double *)(profile->parameters[0].value),
		 profile->parameters[0].desc.specific.mat.nb_r,
		 profile->parameters[0].desc.specific.mat.nb_c);

  } else if (!(strcmp(path, PB[1]) && strcmp(path, PB[2]))) {

    fhandle = diet_function_handle_default(path);
    profile = profile_alloc(1, 1, 2);
    matrix_set(&(profile->parameters[0]), A, DIET_VOLATILE,
	       DIET_DOUBLE, 3, 2, 0);
    matrix_set(&(profile->parameters[0]), B, DIET_VOLATILE,
	       DIET_DOUBLE, 3, 2, 0);
    matrix_set(&(profile->parameters[0]), NULL, DIET_VOLATILE,
	       DIET_DOUBLE, 3, 2, 0);
    print_matrix((double *)(profile->parameters[0].value),
		 profile->parameters[0].desc.specific.mat.nb_r,
		 profile->parameters[0].desc.specific.mat.nb_c);
    print_matrix((double *)(profile->parameters[1].value),
		 profile->parameters[1].desc.specific.mat.nb_r,
		 profile->parameters[1].desc.specific.mat.nb_c);

  }
    
  if (!diet_call(fhandle, profile)) {
    if (!strcmp(path, PB[0])) {
      print_matrix(A,
		   profile->parameters[0].desc.specific.mat.nb_r,
		   profile->parameters[0].desc.specific.mat.nb_c);
    } else {
      C = profile->parameters[2].value;
      print_matrix(C,
		   profile->parameters[2].desc.specific.mat.nb_r,
		   profile->parameters[2].desc.specific.mat.nb_c);
    }
  }
  
  profile_free(profile);
  diet_function_handle_destruct(fhandle);
    
  free(path);

  diet_finalize();

  return 0;
}



