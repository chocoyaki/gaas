/****************************************************************************/
/* $Id$ */
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
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
 * Revision 1.11  2002/12/24 08:25:38  lbertsch
 * Added a way to execute n tests by a command line argument :
 * Usage is : client --repeat <n> <cfg file> <op>
 *
 * Revision 1.10  2002/12/12 18:17:05  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 * Revision 1.8  2002/11/07 18:42:42  pcombes
 * Add includes and configured Makefile variables to install directory.
 * Update dgemm to the implementation that is hardcoded in FAST.
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


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

void usage() {
  fprintf(stderr, "Usage: client [--repeat <n>] <config file> [T|MatSUM|MatPROD]\n");
  fprintf(stderr, "       ex: client client.cfg T\n");
  fprintf(stderr, "           client --repeat 1000 client.cfg MatSUM\n");
  exit(1);
}

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

/* argv[1]: client config file path
   argv[2]: T, MatSUM, or MatPROD   */

int
main(int argc, char **argv)
{
  int i;
  int n_loops = 1;
  char *path;
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;
  double mat1[6] = {1.0,2.0,3.0,4.0,5.0,6.0};
  double mat2[6] = {7.0,8.0,9.0,10.0,11.0,12.0};
  double *A, *B, *C;

  char *PB[3] = {"T", "MatSUM", "MatPROD"};

  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = atoi(argv[i + 1]);
      i++;
      memcpy(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
      i -= 2;
      argc -= 2;
    } else {
      fprintf(stderr, "Unrecognized option %s\n", argv[i]);
      usage();
    }
  }

  if (argc - i != 2) {
    usage();
  }
  path = argv[argc - 1];
  
  A = mat1;
  B = mat2;

  for (i = 0; i < n_loops; i++) {
    if (diet_initialize(argc, argv, argv[1])) {
      fprintf(stderr, "DIET initialization failed !\n");
      return 1;
    } 
    
    if (!strcmp(path, PB[0])) {
      
      fhandle = diet_function_handle_default(path);
      profile = profile_alloc(-1, 0, 0);
      matrix_set(&(profile->parameters[0]),
		 A, DIET_VOLATILE, DIET_DOUBLE, 3, 2, 0);
      print_matrix(A,
		   profile->parameters[0].desc.specific.mat.nb_r,
		   profile->parameters[0].desc.specific.mat.nb_c);
      
    } else if (!(strcmp(path, PB[1]) && strcmp(path, PB[2]))) {
      
      fhandle = diet_function_handle_default(path);
      profile = profile_alloc(1, 1, 2);
      matrix_set(&(profile->parameters[0]),
		 A, DIET_VOLATILE, DIET_DOUBLE, 3, 2, 0);
      if (!(strcmp(path, PB[1]))) {
	matrix_set(&(profile->parameters[1]),
		   B, DIET_VOLATILE, DIET_DOUBLE, 3, 2, 0);
	matrix_set(&(profile->parameters[2]),
		   NULL, DIET_VOLATILE, DIET_DOUBLE, 3, 2, 0);
      } else {
	matrix_set(&(profile->parameters[1]),
		   B, DIET_VOLATILE, DIET_DOUBLE, 2, 3, 0);
	matrix_set(&(profile->parameters[2]),
		   NULL, DIET_VOLATILE, DIET_DOUBLE, 3, 3, 0);
      }
      
      print_matrix(A,
		   profile->parameters[0].desc.specific.mat.nb_r,
		   profile->parameters[0].desc.specific.mat.nb_c);
      print_matrix(B,
		   profile->parameters[1].desc.specific.mat.nb_r,
		   profile->parameters[1].desc.specific.mat.nb_c);
      
    } else {
      fprintf(stderr, "DIET initialization failed !\n");
      return 1;
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
    
    diet_finalize();
  }

  return 0;
}
