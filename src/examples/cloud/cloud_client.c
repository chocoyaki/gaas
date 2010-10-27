/****************************************************************************/
/* Example client for the Cloud Batch System                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Adrian Muresan (adrian.muresan@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2010/10/27 06:56:30  bdepardo
 * Solved compilation problems
 *
 * Revision 1.2  2010/10/27 06:41:25  amuresan
 * modified Eucalyptus_BatchSystem to be able to use existing VMs also
 *
 * Revision 1.1  2010/08/23 09:44:28  amuresan
 * Fixed cmake conflicting target problem
 *
 * Revision 1.3  2010/06/15 08:10:54  amuresan
 * Added cloud configuration files to src/examples/cfgs/
 *
 * Revision 1.2  2010/06/14 21:27:24  amuresan
 * Added header to cloud example files and client config file.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
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

int main(int argc, char **argv)
{
  diet_profile_t          *profile;
  char * result;
  double A[9];
  double B[9];
  double* C = NULL;
  char*rC = NULL;
  diet_matrix_order_t oA, oB, oC;
  int n = 2;
  int m = 3;
/* Initialize a DIET session */
  diet_initialize("./client_cloud.cfg", argc, argv);
  /* Create the profile as explained in Chapter 3 */
  profile = diet_profile_alloc("cloud-demo",1, 1, 3); /* last_in, last_inout, last_out */
  /* Set profile arguments */
  diet_string_set(diet_parameter(profile,3), NULL,    DIET_VOLATILE);
  
  /* init matrixes */
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

  diet_matrix_set(diet_parameter(profile,0),
    A, DIET_VOLATILE, DIET_DOUBLE, m, n, oA);
  print_matrix(A, m, n, (oA == DIET_ROW_MAJOR));
  diet_matrix_set(diet_parameter(profile,1),
    B, DIET_VOLATILE, DIET_DOUBLE, n, m, oB);
  print_matrix(B, n, m, (oB == DIET_ROW_MAJOR));
  diet_string_set(diet_parameter(profile,2), NULL,    DIET_VOLATILE);
  
  if (!diet_parallel_call(profile)) { /* If the call has succeeded ... */
    /* Get and print time */
    diet_string_get(diet_parameter(profile,3), &result, NULL);
    if (result == NULL) {
      printf("Error: result is null !\n");
    } else {
      printf("Host info: %s\n", result);
    }
    diet_string_get(diet_parameter(profile, 2), &rC, NULL);
    printf("C = %s\n", rC);
  }
  else
  {
		printf("calling error...\n");
  }

  /* Free profile */
  diet_profile_free(profile);
  diet_finalize();

  return 0;
}

