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
 * Revision 1.14  2003/01/27 17:55:49  pcombes
 * Bug fix on OUT matrix: C was not initialized.
 *
 * Revision 1.13  2003/01/23 19:13:45  pcombes
 * Update to API 0.6.4
 *
 * Revision 1.12  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.11  2002/12/24 08:25:38  lbertsch
 * Added a way to execute n tests by a command line argument :
 * Usage is : client --repeat <n> <cfg file> <op>
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
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

void usage() {
  fprintf(stderr, "Usage: client [--repeat <n>] <config file> [T|MatSUM|MatPROD]\n");
  fprintf(stderr, "       ex: client client.cfg T\n");
  fprintf(stderr, "           client --repeat 1000 client.cfg MatSUM\n");
  exit(1);
}

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

/* argv[1]: client config file path
   argv[2]: T, MatSUM, or MatPROD   */

int
main(int argc, char **argv)
{
  int i, m, n;
  int n_loops = 1;
  char *path;
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;
  double mat1[6] = {1.0,2.0,3.0,4.0,5.0,6.0};
  double mat2[6] = {7.0,8.0,9.0,10.0,11.0,12.0};
  double *A, *B, *C;
  diet_matrix_order_t oA, oB, oC;

  char *PB[3] = {"T", "MatSUM", "MatPROD"};

  srand(time(NULL));

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

  m = 3;
  n = 2;
  for (i = 0; i < n_loops; i++) {
    if (diet_initialize(argc, argv, argv[1])) {
      fprintf(stderr, "DIET initialization failed !\n");
      return 1;
    } 
    
    oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    
    if (!strcmp(path, PB[0])) {
      
      fhandle = diet_function_handle_default(path);
      profile = diet_profile_alloc(-1, 0, 0);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      
    } else if (!(strcmp(path, PB[1]) && strcmp(path, PB[2]))) {
      
      fhandle = diet_function_handle_default(path);
      profile = diet_profile_alloc(1, 1, 2);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      if (!(strcmp(path, PB[1]))) {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
	print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
      } else {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
	print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
      }
      
    } else {
      fprintf(stderr, "DIET initialization failed !\n");
      return 1;
    } 
    
    if (!diet_call(fhandle, profile)) {
      if (!strcmp(path, PB[0])) {
	diet_matrix_get(diet_parameter(profile,0), NULL, NULL, &m, &n, &oA);
	print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      } else {
	diet_matrix_get(diet_parameter(profile,2), &C, NULL, &m, &n, &oC);
	print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
	free(C);
      }
    }
    
    diet_profile_free(profile);
    diet_function_handle_destruct(fhandle);
    
    diet_finalize();
  }

  return 0;
}
