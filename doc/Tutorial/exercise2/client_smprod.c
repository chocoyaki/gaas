/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Eddy CARON                Eddy.Caron@ens-lyon.fr                    */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "DIET_client.h"

int main(int argc, char **argv)
{
  int i;
  double  factor = M_PI; /* Pi, why not ? */
  size_t m, n;           /* Matrix size */
  double *matrix;        /* The matrix to multiply */
  float  *time   = NULL; /* Computation time */
  diet_profile_t         *profile;

  m=60;
  n = 100;

  /* Allocate the matrix: m lines, n columns */
  matrix = malloc(m * n * sizeof(double));
  /* Fill in the matrix with dummy values (who cares ?) */
  for (i = 0; i < (m * n); i++) {
    matrix[i] = 1.2 * i;
  }
  
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
