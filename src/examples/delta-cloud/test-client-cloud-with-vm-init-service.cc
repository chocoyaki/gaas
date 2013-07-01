#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"
#include <string.h>

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;
  size_t out_size = 0;
  char cmd[2048];

  if (argc != 8) {
    fprintf(stderr, "Usage: %s <file.cfg> vm_collection_name vm_count vm_image vm_profile vm_user is_ip_private\n", argv[0]);
    return 1;
  }
  const char* service = "homogeneous_vm_instanciation";

	char* vm_collection_name = argv[2];
	int vm_count = atoi(argv[3]);
	char* vm_image = argv[4];
	char* vm_profile = argv[5];
	char* deltacloud_api_url = strdup("http://localhost:3001/api");
	char* deltacloud_user_name = strdup("oneadmin");
	char* deltacloud_passwd = strdup("mypassword");
	char* vm_user = argv[6];
	int is_ip_private = atoi(argv[7]);


	printf("vm_count=%i\n", vm_count);

	 if (diet_initialize(argv[1], argc, argv)) {
		fprintf(stderr, "DIET initialization failed !\n");
		return 1;
	}

	profile = diet_profile_alloc( service, 8, 8, 9);
	diet_string_set(diet_parameter(profile, 0), vm_collection_name, DIET_VOLATILE);
	diet_scalar_set(diet_parameter(profile, 1), &vm_count, DIET_VOLATILE, DIET_INT);
	diet_string_set(diet_parameter(profile, 2), vm_image, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 3), vm_profile, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 4), deltacloud_api_url, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 5), deltacloud_user_name, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 6), deltacloud_passwd, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 7), vm_user, DIET_VOLATILE);
	diet_scalar_set(diet_parameter(profile, 8), &is_ip_private, DIET_VOLATILE, DIET_INT);
	diet_file_set(diet_parameter(profile, 9), NULL, DIET_PERSISTENT_RETURN);

  int env = diet_call(profile);



    diet_file_get(diet_parameter(profile, 9), &path, NULL, &out_size);
    if (path && (*path != '\0')) {
      printf("Location of returned file is %s, its size is %lu.\n",
             path, out_size);

        sprintf(cmd, "cat %s", path);

        int env = system(cmd);
    }

	free(deltacloud_api_url);
	free(deltacloud_user_name);
	free(deltacloud_passwd);

  diet_profile_free(profile);

  diet_finalize();

  return 0;
} /* main */

