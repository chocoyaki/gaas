/**
 * @file file_transfer_client.c
 *
 * @brief  file_transfer example: client side
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;
  int *size1 = NULL;
  int *size2 = NULL;
  size_t out_size = 0;


  if (argc != 4) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <file2>\n", argv[0]);
    return 1;
  }
  path = "size";

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc(path, 1, 1, 4);
  if (diet_file_set(diet_parameter(profile, 0), DIET_PERSISTENT, argv[2])) {
    printf("diet_file_set error\n");
    return 1;
  }
  if (diet_file_set(diet_parameter(profile, 1), DIET_VOLATILE, argv[3])) {
    printf("diet_file_set error\n");
    return 1;
  }
  diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile, 3), NULL, DIET_VOLATILE, DIET_INT);
  if (diet_file_set(diet_parameter(profile, 4), DIET_VOLATILE, NULL)) {
    printf("diet_file_set error\n");
    return 1;
  }

  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile, 2), &size1, NULL);
    diet_scalar_get(diet_parameter(profile, 3), &size2, NULL);
    if (size1 && size2) {
      printf("Answered sizes are %d and %d.\n", *size1, *size2);
      /* No need to free size1 and size2: CORBA takes care of them */
    } else {
      fprintf(stderr, "Error: Cannot get answered sizes !\n");
    }
    diet_file_get(diet_parameter(profile, 4), NULL, &out_size, &path);
    if (path && (*path != '\0')) {
      printf("Location of returned file is %s, its size is %zd.\n",
             path, out_size);
      /* If uncommented, next line unlink file */
      /* diet_free_data(diet_parameter(profile, 4)); */
    }
  }

  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */
