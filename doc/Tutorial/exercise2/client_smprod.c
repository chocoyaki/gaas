/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "DIET_client.h"

int main(int argc, char **argv)
{
  int i;
  double  factor = M_PI; /* Pi, why not ? */
  double *matrix;        /* The matrix to multiply */
  float  *time   = NULL; /* Computation time */

  diet_function_handle_t *fhandle;
  diet_profile_t         *profile;

  /* Allocate the matrix: 60 lines, 100 columns */
  matrix = malloc(60 * 100 * sizeof(double));
  /* Fill in the matrix with dummy values (who cares ?) */
  for (i = 0; i < (60 * 100); i++) {
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
