/**
*  @file  client_smprod.c
*  @brief   DIET tutorial scalar by matrix product example: client 
*  @author  - Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           - Eddy CARON  (Eddy.Caron@ens-lyon.fr)
*           - Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
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
  int i;
  double  factor = M_PI; /* Pi, why not ? */
  size_t m, n;           /* Matrix size */
  double* matrix;        /* The matrix to multiply */
  float*  time   = NULL; /* To check that time is set by the server */
  diet_profile_t* profile;

  m = 60;
  n = 100;

  /* Allocate the matrix: m lines, n columns */
  matrix = malloc(m * n * sizeof(double));
  /* Fill in the matrix with dummy values (who cares ?) */
  for (i = 0; i < (m * n); i++) {
    matrix[i] = 1.2 * i;
  }
  
  /* Initialize a DIET session */
  diet_initialize(argv[1], argc, argv);

  /* Create the profile */
  profile = diet_profile_alloc("smprod", 0, 1, 2); // last_in, last_inout, last_out

  /* Set profile arguments */
  diet_scalar_set(diet_parameter(profile, 0),
		  &factor, DIET_VOLATILE, DIET_DOUBLE);
  diet_matrix_set(diet_parameter(profile, 1),
		  matrix, DIET_VOLATILE, DIET_DOUBLE, m, n, DIET_COL_MAJOR);
  diet_scalar_set(diet_parameter(profile, 2),
		  NULL, DIET_VOLATILE, DIET_FLOAT);

  /* Call DIET */
  if (!diet_call(profile)) { /* If the call has succeeded ... */
     
    /* Get and print time */
    diet_scalar_get(diet_parameter(profile, 2), &time, DIET_VOLATILE);
    if (time == NULL) {
      printf("Error: time not set !\n");
   } else {
      printf("time = %f\n", *time);
    }

    /* Check the first non-zero element of the matrix */
    if (fabs(matrix[1] - ((1.2 * 1) * factor)) > 1e-15) {
      printf("Error: matrix not correctly set: matrix[1] = %f !\n", matrix[1]);
    }
  }

  /* Free profile 
   * This does NOT free the data themselves ! */
  diet_profile_free(profile);
  
  /* Finalize the DIET session */
  return (diet_finalize());
}
