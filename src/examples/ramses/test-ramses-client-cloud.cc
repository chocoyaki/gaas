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

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <file.cfg> file\n", argv[0]);
    return 1;
  }
  const char* service = "tar";

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc( service, 0, 0, 1);
  if (diet_string_set(diet_parameter(profile, 0), argv[2], DIET_VOLATILE)) {
    printf("diet_string_set error : file_in\n");
    return 1;
  }

  if (diet_file_set(diet_parameter(profile, 1), NULL, DIET_PERSISTENT_RETURN)) {
	printf("diet_file_set error : file_out");
	return 1;
  }

	int env = diet_call(profile);

	if (env != 0) {
		printf("Erreur @ diet_call : %d\n", env);
		exit(env);
	}


    diet_file_get(diet_parameter(profile, 1), &path, NULL, &out_size);
    printf("path=%p\n", path);
    if (path) {
		printf("output file is %s\n", path);
    }


  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */

