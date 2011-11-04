/**
*  @file  client.c
*  @brief   DIET  tutorial dgemm example: client  
*  @author  Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
*  @section Licence 
*    |LICENCE|
*/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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


int
main(int argc, char **argv)
{
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;

  int    i, j, m, n, k;
  double alpha, beta;
  double *A, *B, *C = NULL;

  /* Initialize a DIET session */

  /* Create the function_handle */
  
  /* Create the profile as explained in chapter 3 of the User Manual */
  
  /* Set profile arguments */

  /* Call DIET */

  /* Get and print (IN)OUT arguments if possible */
  
  /* Free profile and function handle */

  /* Finalize the DIET session */

  return 0;
}

