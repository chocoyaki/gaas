/****************************************************************************/
/* DIET tutorial scalar by matrix product example: client                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Eddy CARON                Eddy.Caron@ens-lyon.fr                    */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*    - Christophe PERA           Christophe.Pera@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "DIET_client.h"

int main(int argc, char **argv)
{
  int i;
  double  factor = M_PI; /* Pi, why not ? */
  double *matrix[5];        /* The matrix to multiply */
  float  *time   = NULL; /* To check that time is set by the server */

  diet_profile_t         *profile[5];
  diet_reqID_t rst[5] = {0,0,0,0,0};

  
  /* Iinitialize a DIET session */
  diet_initialize("./client.cfg", argc, argv);

  /* Create the profile as explained in Chapter \ref{\ref{ch:data} */
  for (i = 0; i < 5; i++){
    /* Allocate the matrix: 60 lines, 100 columns */
    matrix[i] = malloc(60 * 100 * sizeof(double));
    /* Fill in the matrix with dummy values (who cares ?) */
    for (j = 0; j < (60 * 100); j++) {
      matrix[i][j] = 1.2 * j;
    }
    profile = diet_profile_alloc("smprod",0, 1, 2); // last_in, last_inout, last_out
  
    /* Set profile arguments */
    diet_scalar_set(diet_parameter(profile[i],0), &factor, 0, DIET_DOUBLE);
    diet_matrix_set(diet_parameter(profile[i],1), matrix,  0, DIET_DOUBLE, 60, 100, DIET_COL_MAJOR);
    diet_scalar_set(diet_parameter(profile[i],2), NULL,    0, DIET_FLOAT);
  }
  
  int rst_call = 0;
  if ((rst_call = diet_call_async(profile[i], &rst[i])) != 0)  printf("Error in diet_call_async return -%d-\n", rst_call);
  else {
    printf("request ID value = -%d- \n", rst[i]);
    if (rst[i] < 0) {
      printf("error in request value ID\n");
      return;
    }
  }
  rst_call = 0;
  if ((rst_call = diet_wait_and((diet_reqID_t*)&rst, (unsigned int)5)) != 0) printf("Error in diet_wait_and\n");
  else {
    printf("Result data for requestID");
    for (i = 0; i < 5; i++) printf(" %d ", rst[i]);
    printf(" and omnithreadID %d \n", omni_thread::self()->id());
    for (i = 0; i < 5; i++){
      sprintf(requestID, "%d", rst[i]);
      /* Get and print time */
      diet_scalar_get(diet_parameter(profile[i],2), &time, NULL);
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
}

