/**
*  @file  client_smprod_file.c
*  @brief   DIET tutorial scalar by matrix product example: client 
*  @author  - Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           - Eddy CARON  (Eddy.Caron@ens-lyon.fr)
*           - Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
*  @section Licence 
*    |LICENSE|
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "DIET_client.h"

int main(int argc, char **argv)
{
  int i, j;
  double  factor = M_PI; /* Pi, why not ? */
  size_t m, n; /* Matrix size */
  double *matrix[5];        /* The matrix to multiply */
  float  *time   = NULL; /* To check that time is set by the server */

  diet_profile_t         *profile[5];
  diet_reqID_t rst[5] = {0, 0, 0, 0, 0};

  m = 60;
  n = 100;
  
  /* Initialize a DIET session */
  diet_initialize("./client.cfg", argc, argv);

  /* Create the profile as explained in Chapter 3 */
  for (i = 0; i < 5; i++){
    /* Allocate the matrix: 60 lines, 100 columns */
    matrix[i] = malloc(m * n * sizeof(double));
    /* Fill in the matrix with dummy values (who cares ?) */
    for (j = 0; j < (m * n); j++) {
      matrix[i][j] = 1.2 * j;
    }
    profile[i] = diet_profile_alloc("smprod", 0, 1, 2); // last_in, last_inout, last_out
  
    /* Set profile arguments */
    diet_scalar_set(diet_parameter(profile[i], 0), &factor, 0, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile[i], 1), matrix[i],  0, DIET_DOUBLE,
                    m, n, DIET_COL_MAJOR);
    diet_scalar_set(diet_parameter(profile[i], 2), NULL,    0, DIET_FLOAT);
  }
  
  /* Call Diet */
  int rst_call = 0;
  
  for (i = 0; i < 5; i++){
     if ((rst_call = diet_call_async(profile[i], &rst[i])) != 0)  
        printf("Error in diet_call_async return -%d-\n", rst_call);
     else {
       printf("request ID value = -%d- \n", rst[i]);
       if (rst[i] < 0) {
         printf("error in request value ID\n");
         return 1;
       }
     }
     rst_call = 0;
  }   

  /* Wait for Diet answers */
  if ((rst_call = diet_wait_and((diet_reqID_t*)&rst, (unsigned int)5)) != 0)
     printf("Error in diet_wait_and\n");
  else {
    printf("Result data for requestID");
    for (i = 0; i < 5; i++) printf(" %d ", rst[i]);
    for (i = 0; i < 5; i++){
      /* Get and print time */
      diet_scalar_get(diet_parameter(profile[i], 2), &time, NULL);
      if (time == NULL) {
        printf("Error: time not set !\n");
      } else {
        printf("time = %f\n", *time);
      }

      /* Check the first non-zero element of the matrix */
      if (fabs(matrix[i][1] - ((1.2 * 1) * factor)) > 1e-15) {
        printf("Error: matrix not correctly set !\n");
      }
    }
  }
  /* Free profiles */
  for (i = 0; i < 5; i++){
    diet_cancel(rst[i]);
    diet_profile_free(profile[i]);
  }
  diet_finalize();
  return 0;
}
