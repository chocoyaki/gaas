/**
 * @file sequential_server.c
 *
 * @brief   DIET server for sequential submission
 *
 * @author  Yves Caniou (Yves.Caniou@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <libgen.h> /* basename() */

#include "DIET_server.h"
/* #include "DIET_mutex.h" */
#include <sys/stat.h>

#define MAX_STRING_LENGTH 100

/****************************************************************************
* SOLVE FUNCTION
****************************************************************************/

int
solve_concatenation(diet_profile_t *pb) {
  size_t arg_size1 = 0;
  size_t arg_size2 = 0;
  char *path1 = NULL;
  char *path2 = NULL;
  char *path_result = NULL;
  double *ptr_nbreel = NULL;
  FILE *file, *output_file;
  int status = 0;
  size_t count;
  char chaine[MAX_STRING_LENGTH];
  struct stat buf;

  printf("Resolving sequential service 'concatenation'!\n\n");

  /* IN args */
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size1, &path1);
  if ((status = stat(path1, &buf))) {
    return status;
  }
  if (!(buf.st_mode & S_IFREG)) { /* regular file */
    return 2;
  }
  printf("Name of the first file: %s\n", path1);

  diet_scalar_get(diet_parameter(pb, 1), &ptr_nbreel, NULL);
  diet_file_get(diet_parameter(pb, 2), NULL, &arg_size2, &path2);
  if ((status = stat(path2, &buf))) {
    return status;
  }
  if (!(buf.st_mode & S_IFREG)) { /* regular file */
    return 2;
  }
  printf("Name of the second file: %s\n", path2);

  /* OUT args */
  path_result = strdup("/tmp/result.txt"); /* MUST NOT BE CONSTANT STRING */
  if (diet_file_desc_set(diet_parameter(pb, 3), path_result)) {
    printf("diet_file_desc_set() error\n");
    return 1;
  }
  printf("Name of result file: %s\n", path_result);

  /* Concatenation */
  file = fopen(path1, "r");
  output_file = fopen(path_result, "w+");
  while (!feof(file)) {
    count = fread(&chaine, sizeof(char), MAX_STRING_LENGTH, file);
    fwrite(chaine, sizeof(char), count, output_file);
  }
  fclose(file);
  file = fopen(path2, "r");
  while (!feof(file)) {
    count = fread(&chaine, sizeof(char), MAX_STRING_LENGTH, file);
    fwrite(chaine, sizeof(char), count, output_file);
  }
  fclose(file);
  fclose(output_file);

  /* Free memory */

  /* Don't free path1, path2 and path_result since not duplicated by CORBA */
  return 0;
} /* solve_concatenation */

/****************************************************************************
* MAIN
****************************************************************************/

int
main(int argc, char *argv[]) {
  int res = 0;
  int nb_max_services = 1;
  diet_profile_desc_t *profile = NULL;


  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /* Initialize table with maximum services */
  diet_service_table_init(nb_max_services);

  /* Allocate batch profile (IN, INOUT, OUT) */
  profile = diet_profile_desc_alloc("concatenation", 2, 2, 3);

  /* Set profile parameters:
     this job is submitted by a sequential system..
     Note that this is default, then line is unecessary
     diet_profile_desc_set_sequential(profile); */

  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_FILE, DIET_CHAR);
  /* All done */

  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, solve_concatenation)) {
    return 1;
  }

  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);

  /* Print the table to check */
  diet_print_service_table();

  /* Launch the server */
  res = diet_SeD(argv[1], argc, argv);

  return res;
} /* main */
