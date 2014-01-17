#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;
  size_t out_size = 0;
  char cmd[2048];

  if (argc != 4) {
    fprintf(stderr, "Usage: %s <file.cfg> <matrix_file1> <matrix_file2>\n", argv[0]);
    return 1;
  }
  const char* service = "matrixAdd";

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc( service, 1, 1, 2);
  if (diet_file_set(diet_parameter(profile, 0), argv[2], DIET_VOLATILE)) {
    printf("diet_file_set error : matrix 1\n");
    return 1;
  }
  if (diet_file_set(diet_parameter(profile, 1), argv[3], DIET_VOLATILE)) {
    printf("diet_file_set error : matrix 2\n");
    return 1;
  }

  if (diet_file_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE)) {
    printf("diet_file_set error : result\n");
    return 1;
  }

  int env = diet_call(profile);



  diet_file_get(diet_parameter(profile, 2), &path, NULL, &out_size);
  if (path && (*path != '\0')) {
    printf("Location of returned file is %s, its size is %lu.\n",
        path, out_size);

    sprintf(cmd, "cat %s", path);

    int ret_value = system(cmd);
    if (! ret_value) {
      printf("Error while running %s", cmd);
    }
  }


  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */

