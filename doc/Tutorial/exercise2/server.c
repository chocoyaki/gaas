/****************************************************************************/
/* DIET tutorial scalar by matrix product example: server                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH           Ludovic.Bertsch@ens-lyon.fr               */
/*    - Eddy CARON                Eddy.Caron@ens-lyon.fr                    */
/*    - Philippe COMBES           Philippe.Combes@ens-lyon.fr               */
/*                                                                          */
/****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "DIET_server.h"


/*
 * "LIBRARY" FUNCTIONS
 * They are defined here to simplify compilation.
 */

int scal_mat_prod(double alpha, double *M, int nb_rows, int nb_cols, float *time)
{
  size_t i;
  struct timeval t1, t2;

  gettimeofday(&t1, NULL);
  for (i = 0; i < nb_rows * nb_cols; i++) {
    M[i] *= alpha;
  }
  gettimeofday(&t2, NULL);
  *time = (t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000;

  return 0;
}


int scal_mat_prod_file(double alpha, char *path, float *time)
{
  int i, m, n;
  struct timeval t1, t2;
  FILE *f;
  double *matrix;

  gettimeofday(&t1, NULL);
  
  /* Load matrix in memory */
  f = fopen(path, "r");
  if (f == NULL) {
    fprintf(stderr, "Error opening file %s!\n", path);
    perror("scal_mat_prod_file");
    return 1;
  }
  fscanf(f, "%d %d", &m, &n);
  matrix = (double*) malloc(sizeof(double)*(m*n));
  for (i = 0; i < m*n; i++) {
    fscanf(f, "%lf", &(matrix[i]));
  }
  fclose(f);

  /* Computation */
  for (i = 0; i < m*n; i++) {
    matrix[i] *= alpha;
  }

  /* Dump matrix into the file */
  f = fopen(path, "w");
  if (f == NULL) {
    fprintf(stderr, "Error opening file %s!\n", path);
    perror("scal_mat_prod_file");
    return 1;
  }
  fprintf(f, "%d %d ", m, n);
  for (i = 0; i < m*n; i++) {
    fprintf(f, "%g ", matrix[i]);
  }
  fclose(f);
  
  /* Set time */
  gettimeofday(&t2, NULL);
  *time = (t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000;

  return 0;
}



/*
 * SOLVE FUNCTIONS
 */


int 
solve_smprod(diet_profile_t *pb)
{
  size_t m,n;
  double* coeff;
  double *mat;
  float  time;

  /* Get arguments */

  /* Launch computation */

  /* Set OUT arguments */

  return res;
}
  
int
solve_smprod_file(diet_profile_t *pb)
{
  double* coeff;
  char  *path;
  float  time;

  /* Get arguments */
  
  /* Launch computation */

  /* Set OUT arguments */

  return 0;
}


/*
 * MAIN
 */

int 
main(int argc, char **argv)
{
  diet_profile_desc_t *profile;
  
  /* Initialize table with maximum 2 services */

  /* Allocate smprod profile */

  /* Set profile parameters */

  /* Add the smprod to the service table */

  /* Free the smprod profile, since it was deep copied */

  /* Allocate smprod_file profile */

  /* Set profile parameters */

  /* Add the smprod_file to the service table */

  /* Free the smprod_file profile, since it was deep copied */

  /* Launch the SeD: 
     argv[1] is the path of the configuration file */

  return res;
}
