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
  size_t m, n;
  double *matrix;        /* The matrix to multiply */
  float  *time   = NULL; /* To check that time is set by the server */
  diet_function_handle_t *fhandle;
  diet_profile_t         *profile;

  m = 60;
  n = 100;

  /* Allocate the matrix: m lines, n columns */
  matrix = malloc(m * n * sizeof(double));
  /* Fill in the matrix with dummy values (who cares ?) */
  for (i = 0; i < (m * n); i++) {
    matrix[i] = 1.2 * i;
  }
  
  /* Initialize a DIET session */
  diet_initialize(argc, argv, argv[1]);

  /* Create the function_handle */
  fhandle = diet_function_handle_default("smprod");

  /* Create the profile */
  profile = diet_profile_alloc(0, 1, 2); // last_in, last_inout, last_out
  
  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile,0), &factor, DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile,1), matrix,  DIET_VOLATILE, DIET_DOUBLE,
		  m, n, DIET_COL_MAJOR);
  diet_scalar_set(diet_parameter(profile,2), NULL,    DIET_VOLATILE, DIET_FLOAT);
  
  /* Call DIET */
  if (!diet_call(fhandle, profile)) { /* If the call has succeeded ... */
     
    /* Get and print time */
    time = diet_value(float,diet_parameter(profile,2));
    if (time == NULL) {
      printf("Error: time not set !\n");
    } else {
      printf("time = %f\n", *time);
    }

    /* Check the first non-zero element of the matrix */
    if (fabs(matrix[1] - ((1.2 * 1) * M_PI)) > 1e-15) {
      printf("Error: matrix not correctly set !\n");
    }
  }

  /* Free profile and function handle */
  diet_profile_free(profile);
  diet_function_handle_destruct(fhandle);
  
  /* Finalize the DIET session */
  return (diet_finalize());
}
