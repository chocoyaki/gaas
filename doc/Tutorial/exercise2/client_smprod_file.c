/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client (file version)    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Eddy CARON                Eddy.Caron@ens-lyon.fr                    */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>

#include "DIET_client.h"

int main(int argc, char **argv)
{

  double  factor = M_PI;           /* Pi, why not ? */
  float  *time   = NULL;           /* To check that time is set by server */
  char   *matrix_file = "matrix1"; /* The matrix to multiply */
  diet_profile_t* profile;

  /* Initialize a DIET session */

  /* Create the profile as explained in chapter 3 of the User Manual */
  
  /* Set profile arguments */

  /* Call DIET */

  /* Get and print (IN)OUT arguments if possible */
  
  /* Free profile */

  /* Finalize the DIET session and return the result of this function */


}
