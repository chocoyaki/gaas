/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client (file version)    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>

#include "DIET_client.h"


int main(int argc, char **argv)
{
  int i;
  double  factor = M_PI;           /* Pi, why not ? */
  char   *matrix_file_path = NULL; /* The matrix to multiply */
  float  *time   = NULL;           /* Computation time */

  diet_function_handle_t *fhandle;
  diet_profile_t         *profile;

  /* Initialize a DIET session */

  /* Create the function_handle */
  
  /* Create the profile  as explained in chapter 3 of the User Manual */
  
  /* Set profile arguments */

  /* Call DIET */

  /* Get and print (IN)OUT arguments if possible */
  
  /* Free profile and function handle */

  /* Finalize the DIET session */

  return 0;
}
