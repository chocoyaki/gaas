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

  double  factor = M_PI; /* Pi, why not ? */
  /*  float  *time   = NULL; /* To check that time is set by the server */
  diet_profile_t         *profile;
  char* problemname = NULL;

  /* Initialize a DIET session */
  diet_initialize(argv[1], argc, argv); 

  strcpy(problemname,"smprod");

  /* Create the profile */
  profile = diet_profile_alloc(problemname, 0, 1, 2); // last_in, last_inout, last_out
  
  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile,0), 
		  &factor, DIET_VOLATILE, DIET_DOUBLE);

  diet_file_set(diet_parameter(profile,1),DIET_VOLATILE, "matrix1");

  diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_FLOAT);

  /* Call DIET */
  if (!diet_call(profile)) { /* If the call has succeeded ... */
     
    /* Get and print time */
    /*    time = diet_value(float, diet_parameter(profile,2));
    if (time == NULL) {
      printf("Error: time not set !\n");
    } else {
      printf("time = %f\n", *time);
      }*/

    printf("The result matrix is in the file matrix1:\n");
    printf("\"cat matrix1\" to see that it has been modified !\n");
  }

  /* Free profile and function handle */
  diet_profile_free(profile);

  /* Finalize the DIET session */
  return (diet_finalize());
}
