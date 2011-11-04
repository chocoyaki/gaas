/**
*  @file  client_smprodAsync.c
*  @brief   DIET tutorial scalar by matrix product example: client 
*  @author  Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           Eddy CARON  (Eddy.Caron@ens-lyon.fr)
*           Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
*  @section Licence 
*    |LICENCE|
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "DIET_client.h"

int main(int argc, char **argv)
{
  int i, j;
  double  factor = M_PI; /* Pi, why not ? */
  size_t m, n;           /* Matrix size */
  double *matrix[5];        /* The matrix to multiply */
  float  *time   = NULL; /* Computation time */
  
  diet_profile_t         *profile[5];
  diet_reqID_t rst[5] = {0, 0, 0, 0, 0};

  m = 60;
  n = 100;

  for (i = 0; i < 5; i++) {
     /* Allocate the matrix: m lines, n columns */
     matrix[i] = malloc(m * n * sizeof(double));
     /* Fill in the matrix with dummy values (who cares ?) */
     for (j = 0; j < (m * n); j++) {
        matrix[i][j] = 1.2 * j;
     }
  }
  
  /* Initialize a DIET session */

  /* Create the function_handle */
  
  /* Create the profile as explained in chapter 3 of the User Manual */
  
  /* Set profile arguments */

  /* Call DIET with asynchronous mecanism */

  /* Get and print (IN)OUT arguments if possible */
  
  /* Free profile and function handle */

  /* Finalize the DIET session */

  return 0;
}
