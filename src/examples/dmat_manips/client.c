/****************************************************************************/
/* dmat_manips example: a DIET client for transpose, MatSUM and MatPROD     */
/*   problems (MatSUM is expanded to SqMatSUM and SqMatSUM_bis).            */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.16  2003/04/10 13:28:15  pcombes
 * Apply new Coding Standards.
 *
 * Revision 1.15  2003/02/07 17:05:23  pcombes
 * Add SqMatSUM_opt with the new convertor API.
 * Use diet_free_data to properly free user's data.
 *
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
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"


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


#define NB_PB 5
static const char* PB[NB_PB] =
  {"T", "MatPROD", "MatSUM", "SqMatSUM", "SqMatSUM_opt"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char* cmd)
{
   fprintf(stderr, "Usage: %s [--repeat <n>] <file.cfg> [%s|%s|%s|%s|%s]\n",
	   cmd, PB[0], PB[1], PB[2], PB[3], PB[4]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 client.cfg MatSUM\n", cmd);
  exit(1);
}

int
main(int argc, char* argv[])
{
  int i, m, n;
  int n_loops = 1;
  char* path = NULL;
  diet_function_handle_t* fhandle = NULL;
  diet_profile_t* profile = NULL;
  double mat1[9] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0};
  double mat2[9] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0};
  double* A = NULL;
  double* B = NULL;
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;

  int   pb[NB_PB] = {0, 0, 0, 0, 0};

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
      usage(argv[0]);
    }
  }

  if (argc - i != 2) {
    usage(argv[0]);
  }
  path = argv[argc - 1];
  
  A = mat1;
  B = mat2;

  m = 3;
  n = 2;

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) break;
  }
  // Square matrix problems:
  if (pb[3] || pb[4])
    n = m;

  for (i = 0; i < n_loops; i++) {
    
    oA = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oB = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    oC = (rand() & 1) ? DIET_ROW_MAJOR : DIET_COL_MAJOR;
    
    if (pb[0]) {
      
      fhandle = diet_function_handle_default(path);
      profile = diet_profile_alloc(-1, 0, 0);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      
    } else if (pb[1] || pb[2] || pb[3]) {
      
      fhandle = diet_function_handle_default(path);
      profile = diet_profile_alloc(1, 1, 2);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      if (pb[1]) {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
	print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, m, oC);
      } else {
	diet_matrix_set(diet_parameter(profile,1),
			B, DIET_VOLATILE, DIET_DOUBLE, m, n, oB);
	print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
	diet_matrix_set(diet_parameter(profile,2),
			NULL, DIET_VOLATILE, DIET_DOUBLE, m, n, oC);
      }
      
    } else if (pb[4]) {
      
      fhandle = diet_function_handle_default(path);
      profile = diet_profile_alloc(0, 1, 1);
      diet_matrix_set(diet_parameter(profile,0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, m, oA);
      print_matrix(A, m, m, (oA == DIET_ROW_MAJOR));
      diet_matrix_set(diet_parameter(profile,1),
		      B, DIET_VOLATILE, DIET_DOUBLE, m, m, oB);
      print_matrix(B, m, m, (oB == DIET_ROW_MAJOR));
      
    } else {
      fprintf(stderr, "Unknown problem: %s !\n", path);
      return 1;
    } 
    
    if (!diet_call(fhandle, profile)) {
      if (pb[0]) {
	diet_matrix_get(diet_parameter(profile,0), NULL, NULL, &m, &n, &oA);
	print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
      } else if (pb[4]) {
	diet_matrix_get(diet_parameter(profile,0), NULL, NULL, &m, &n, &oB);
	print_matrix(B, m, n, (oB == DIET_ROW_MAJOR));
      } else {
	diet_matrix_get(diet_parameter(profile,2), &C, NULL, &m, &n, &oC);
	print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
	diet_free_data(diet_parameter(profile,2));
      }
    }
    
    diet_profile_free(profile);
    diet_function_handle_destruct(fhandle);
    
  }

  diet_finalize();
  return 0;
}
