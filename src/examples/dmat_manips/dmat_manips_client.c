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
 * Revision 1.1  2010/08/06 14:25:29  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.30  2010/03/31 02:24:47  ycaniou
 * Warnings--
 * Remove unecessary includes
 *
 * Revision 1.29  2010/03/26 15:15:36  bdepardo
 * Print usage when the parameters are not correct.
 *
 * Revision 1.28  2010/03/23 12:44:18  glemahec
 * Correction des exemples pour DAGDA
 *
 * Revision 1.27  2006/06/30 15:26:02  ycaniou
 * C++ commentaries -> C commentaries to remove compilation warnings
 *
 * Revision 1.26  2005/11/13 05:21:21  ecaron
 * Cast tv_sec and tv_usec (for Mac)
 *
 * Revision 1.25  2004/11/26 17:08:12  hdail
 * Correct error in memory management.  We were trying to free an INOUT parameter
 * with diet_free_data, but it can only be used for OUT parameters on the client
 * and IN parameters on the server.
 *
 * Revision 1.24  2004/10/08 12:19:59  hdail
 * Correcting warnings with mismatched types (size_t and int) on 64-bit.
 *
 * Revision 1.23  2004/09/16 09:52:36  hdail
 * Corrected bug whereby the option --repeat results in seg fault or server not
 * found.  Current fix is to not re-use the profile; in longer-term we should
 * verify why DIET fails when profile is re-used.
 *
 * Revision 1.22  2004/02/25 22:35:34  ecaron
 * Clean Bruno's version (DIET 1.0 without data persistency)
 *
 * Revision 1.20  2003/09/26 14:07:01  pcombes
 * Get the examples out of the automake chain, to make Makefiles more readable.
 * The generated before.mk includes the right Makefile.inc.
 *
 * Revision 1.19  2003/08/01 19:35:11  pcombes
 * Fix bugs on memcpy (misused) and DIET_STAT_FILE_NAME.
 *
 * Revision 1.18  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 ****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

#define print_condition 1
#define print_matrix(mat, m, n, rm)        \
  if (print_condition) {                   \
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


#define NB_PB 3
static const char* PB[NB_PB] =
  {"T", "MatPROD", "MatSUM"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s %s <file.cfg> [%s|%s|%s]\n", cmd,
	  "[--repeat <n>] [--pause <n �s>]", PB[0], PB[1], PB[2]);
  fprintf(stderr, "    ex: %s client.cfg T\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 client.cfg MatSUM\n", cmd);
  fprintf(stderr, "        %s --repeat 1000 --pause 1000 %s\n",
	  cmd, "client.cfg MatSUM (1ms between two steps of the loop)");
  exit(1);
}

int
main(int argc, char* argv[])
{
  int i;
  size_t m, n;
  int n_loops = 1;
  char* path = NULL;
  diet_profile_t* profile = NULL;
  /* double mat1[9] = {1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0}; */
  /* double mat2[9] = {10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0}; */
  double A[9];
  double B[9];
  double* C = NULL;
  diet_matrix_order_t oA, oB, oC;

  int   pb[NB_PB] = {0, 0, 0};

  /* STATS */
  char* STAT_FILE_NAME = NULL;
  FILE* STAT_FILE      = NULL;
  size_t nb_of_requests;
  int pause = 0;
  
  srand(time(NULL));

  for (i = 1; i < argc - 2; i++) {
    if (strcmp("--repeat", argv[i]) == 0) {
      n_loops = atoi(argv[i + 1]);
      if (n_loops < 0) {
        fprintf(stderr, "Argument to option %s must be >= 0.\n", argv[i]);
        usage(argv[0]);
      }
      i++;
      memmove(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
      i -= 2;
      argc -= 2;
    } else if (strcmp("--pause", argv[i]) == 0) {
      pause = atoi(argv[i + 1]);
      i++;
      memmove(argv + i - 2, argv + i, (argc - i)*sizeof(char*));
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
  
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) break;
  }
  

  if ((STAT_FILE_NAME = getenv("DIET_STAT_FILE_NAME")))
    STAT_FILE = fopen(STAT_FILE_NAME, "wc");
  nb_of_requests = 0;

  oA = ((double)rand()/(double)RAND_MAX <= 0.5) ? DIET_ROW_MAJOR : 
                                                  DIET_COL_MAJOR;
  oB = ((double)rand()/(double)RAND_MAX <= 0.5) ? DIET_ROW_MAJOR : 
                                                  DIET_COL_MAJOR;
  oC = ((double)rand()/(double)RAND_MAX <= 0.5) ? DIET_ROW_MAJOR : 
                                                  DIET_COL_MAJOR;
  /* printf("oA: %d oB: %d oC: %d\n", oA, oB, oC); */

  A[0] = 1.0; A[1] = 2.0; A[2] = 3.0; A[3] = 4.0; A[4] = 5.0;
  A[5] = 6.0; A[6] = 7.0; A[7] = 8.0; A[8] = 9.0;

  B[0] = 10.0; B[1] = 11.0; B[2] = 12.0; B[3] = 13.0; B[4] = 14.0;
  B[5] = 15.0; B[6] = 16.0; B[7] = 17.0; B[8] = 18.0;

  C = NULL;

  if (pb[0]) { 
    /* Each repetition transposes result of last */
    /* Set values only once */
    n = 2; m = 3; 
  }

  for (i = 0; i < n_loops; i++) {
    if (pb[1] || pb[2]){
      n = 2; m = 3;
    }
  
    if (pb[0]) {
      profile = diet_profile_alloc(path, 0, 0, 1);
      diet_matrix_set(diet_parameter(profile, 0),
		      A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
      diet_matrix_set(diet_parameter(profile, 1),
                      NULL, DIET_VOLATILE, DIET_DOUBLE, n, m, oA);
      print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
    } else if (pb[1] || pb[2]){
      profile = diet_profile_alloc(path, 1, 1, 2);
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
    } else {
      fprintf(stderr, "Unknown problem: %s !\n", path);
      return 1;
    } 
   
    nb_of_requests++;
    if (!diet_call(profile)) {

      if (pb[0]) {
        diet_matrix_get(diet_parameter(profile, 1), &C, NULL, &m, &n, &oA);
        print_matrix(C, m, n, (oA == DIET_ROW_MAJOR));
        diet_free_data(diet_parameter(profile, 1));
      } else {
        diet_matrix_get(diet_parameter(profile, 2), &C, NULL, &m, &n, &oC);
        print_matrix(C, m, n, (oC == DIET_ROW_MAJOR));
        diet_free_data(diet_parameter(profile, 2));
      }
    }
    diet_profile_free(profile);
  }

  diet_finalize();

  return 0;
}