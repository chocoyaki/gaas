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
  double  factor = M_PI; /* Pi, why not ? */
  float  *time   = NULL; /* To check that time is set by the server */
  diet_function_handle_t *fhandle;
  diet_profile_t         *profile;

  /* Initialize a DIET session */
  diet_initialize(argc, argv, argv[1]);

  /* Create the function_handle */
  fhandle = diet_function_handle_default("smprod_file");
  
  /* Create the profile */
  profile = diet_profile_alloc(0, 1, 2); // last_in, last_inout, last_out
  
  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile,0), &factor, DIET_VOLATILE, DIET_DOUBLE);
  diet_file_set(diet_parameter(profile,1),            DIET_VOLATILE, "matrix1");
  diet_scalar_set(diet_parameter(profile,2), NULL,    DIET_VOLATILE, DIET_FLOAT);

  /* Call DIET */
  if (!diet_call(fhandle, profile)) { /* If the call has succeeded ... */
     
    /* Get and print time */
    time = diet_value(float, diet_parameter(profile,2));
    if (time == NULL) {
      printf("Error: time not set !\n");
    } else {
      printf("time = %f\n", *time);
    }

    printf("The result matrix is in the file matrix1:\n");
    printf("\"cat matrix1\" to see that it has been modified !\n");
  }

  /* Free profile and function handle */
  diet_profile_free(profile);
  diet_function_handle_destruct(fhandle);

  /* Finalize the DIET session */
  return (diet_finalize());
}
