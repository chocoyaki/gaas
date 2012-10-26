/**
*  @file  server.c
*  @brief   DIET tutorial scalar by matrix product example: server 
*  @author  - Ludovic BERTSCH  (Ludovic.Bertsch@ens-lyon.fr)
*           - Eddy CARON  (Eddy.Caron@ens-lyon.fr)
*           - Philippe COMBES  (Philippe.Combes@ens-lyon.fr)
*  @section Licence 
*    |LICENSE|
*/

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
  FILE* f;
  double* matrix;

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
  size_t m, n;
  double* coeff;
  double* mat;
  float  time;

  /* Get arguments */
  diet_scalar_get(diet_parameter(pb, 0), &coeff, NULL);
  diet_matrix_get(diet_parameter(pb, 1), &mat, NULL, &m, &n, NULL);

  /* Launch computation */
  scal_mat_prod(*coeff, mat, m, n, &time);
  printf("Computation done in %fs\n", time);
  
  /* Set OUT arguments */
  diet_scalar_desc_set(diet_parameter(pb, 2), &time);

  return 0;
}

int
solve_smprod_file(diet_profile_t *pb)
{
  double* coeff;
  char*   path;
  float   time;
  
  /* Get arguments */
  diet_scalar_get(diet_parameter(pb, 0), &coeff, NULL);

  diet_file_get(diet_parameter(pb, 1), &path, NULL, NULL);

  /* Launch computation */
  scal_mat_prod_file(*coeff, path, &time);
  printf("Computation done in %fs\n", time);
  
  /* Set OUT arguments */
  diet_scalar_desc_set(diet_parameter(pb, 2), &time);

  return 0;
}


/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  int nb_max_services = 2;
  int res;
  diet_profile_desc_t* profile;
  
  /* Initialize table with maximum 2 services */
  diet_service_table_init(nb_max_services);

  /* Allocate smprod profile */
  profile = diet_profile_desc_alloc("smprod", 0, 1, 2);

  /* Set profile parameters */
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_FLOAT);

  /* Add the smprod to the service table */
  diet_service_table_add(profile, NULL, solve_smprod);

  /* Free the smprod profile, since it was deep copied */
  diet_profile_desc_free(profile);
  
  /* Allocate smprod_file profile */
  profile = diet_profile_desc_alloc("smprod_file", 0, 1, 2);

  /* Set profile parameters */
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE,   DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_FLOAT);
  
  /* Add the smprod_file to the service table */
  diet_service_table_add(profile, NULL, solve_smprod_file);

  /* Free the smprod_file profile, since it was deep copied */
  diet_profile_desc_free(profile);

  // Print the table to check
  diet_print_service_table();

  // Launch the server
  res = diet_SeD(argv[1], argc, argv);

  // Not reached
  return res;
}
