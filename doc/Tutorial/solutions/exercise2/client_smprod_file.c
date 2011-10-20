/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client (file version)    */
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
#include <unistd.h>
#include <string.h>

#include "DIET_client.h"

int main(int argc, char **argv)
{

  double  factor = M_PI; /* Pi, why not ? */
  float*  time   = NULL; /* To check that time is set by the server */
  char*   matrix_file = "matrix1";
  diet_profile_t* profile;

  /* Initialize a DIET session */
  diet_initialize(argv[1], argc, argv); 

  /* Create the profile */
  profile = diet_profile_alloc("smprod_file", 0, 1, 2); // last_in, last_inout, last_out
  
  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile, 0), 
		  &factor, DIET_VOLATILE, DIET_DOUBLE);
  diet_file_set(diet_parameter(profile, 1), DIET_PERSISTENT, matrix_file);

  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_FLOAT);

  /* Call DIET */
  if (!diet_call(profile)) { /* If the call has succeeded ... */
     
    /* Get and print time */
    diet_scalar_get(diet_parameter(profile, 2), &time, NULL);
    if (time == NULL) {
      printf("Error: time not set !\n");
    } else {
      printf("time = %f\n", *time);
      /* Free the DIET-alloc'd time parameter: do not use time from this point ! */
      diet_free_data(diet_parameter(profile, 2));
      time = NULL; /* not to leave a pending reference */
    }

    printf("The result matrix is in the file matrix1:\n");
    printf("\"cat matrix1\" to see that it has been modified !\n");
  }

  /* Free profile */
  diet_profile_free(profile);

  /* Finalize the DIET session */
  return (diet_finalize());
}
