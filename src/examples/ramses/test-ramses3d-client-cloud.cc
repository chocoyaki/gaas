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

  if (argc < 5) {
    fprintf(stderr, "Usage: %s <file.cfg> nodeslist_path nbProcs rundir\n", argv[0]);
    return 1;
  }
  const char* service = "ramses3d";

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc( service, 2, 2, 3);
  if (diet_string_set(diet_parameter(profile, 0), argv[2], DIET_VOLATILE)) {
    printf("diet_string_set error : nodeslist_path\n");
    return 1;
  }
  if (diet_string_set(diet_parameter(profile, 1), argv[3], DIET_VOLATILE)) {
    printf("diet_string_set error : nbProcs\n");
    return 1;
  }

  if (diet_string_set(diet_parameter(profile, 2), argv[4], DIET_VOLATILE)) {
    printf("diet_string_set error : rundir\n");
    return 1;
  }


  if (diet_string_set(diet_parameter(profile, 3), NULL, DIET_VOLATILE)) {
    printf("diet_string_set error : output-path\n");
    return 1;
  }

	int env = diet_call(profile);

	if (env != 0) {
		printf("Erreur @ diet_call : %d\n", env);
		exit(env);
	}

    diet_string_get(diet_parameter(profile, 3), &path, NULL);
    printf("path=%p\n", path);
    if (path /*&& (*path != '\0')*/) {
		printf("output dir is %s\n", path);
    }


  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */

