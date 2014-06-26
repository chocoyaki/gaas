#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"
#include <string.h>

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

static char *
constructVmName(int num, char **strings);

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;
  size_t out_size = 0;
  char cmd[2048];

  if (argc < 7) {
    fprintf(stderr, "Usage: %s <file.cfg> vm_count vm_name vm_profile vm_user is_ip_private\n", argv[0]);
    return 1;
  }
  const char* service = "homogeneous_vm_instanciation";


	int vm_count = atoi(argv[2]);
	//char* vm_image = argv[3];
	char* vm_profile = argv[4];
	char* vm_user = argv[5];
	int is_ip_private = atoi(argv[6]);
	char *vm_name =argv[3];

	printf("vm_count=%i\n", vm_count);

	 if (diet_initialize(argv[1], argc, argv)) {
		fprintf(stderr, "DIET initialization failed !\n");
		return 1;
	}

	profile = diet_profile_alloc( service, 4, 4, 5);
	diet_scalar_set(diet_parameter(profile, 0), &vm_count, DIET_VOLATILE, DIET_INT);
	//diet_string_set(diet_parameter(profile, 1), vm_image, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 2), vm_profile, DIET_VOLATILE);
	diet_string_set(diet_parameter(profile, 3), vm_user, DIET_VOLATILE);
	diet_scalar_set(diet_parameter(profile, 4), &is_ip_private, DIET_VOLATILE, DIET_INT);
	diet_file_set(diet_parameter(profile, 5), NULL, DIET_PERSISTENT_RETURN);

	//targetString = constructTargetString(argc - 2, &(argv[2]));
	  if (diet_paramstring_set(diet_parameter(profile,1 ),
			  vm_name,DIET_VOLATILE) != 0) {
	    fprintf(stderr, "%s: unable to set target vm_name\n", argv[0]);
	    exit(1);
	  }

	int env = diet_call(profile);

    diet_file_get(diet_parameter(profile, 5), &path, NULL, &out_size);
    if (path && (*path != '\0')) {
      printf("Location of returned file is %s, its size is %lu.\n",
             path, out_size);

        sprintf(cmd, "cat %s", path);

        int env = system(cmd);
    }

	diet_profile_free(profile);



	int s = 1;
	printf("sleeping for %i seconds\n", s);
	sleep(s);

	printf("destruction of VMs\n");

	const char* vm_destruction_service = "vm_destruction_by_ip";

	profile = diet_profile_alloc(vm_destruction_service, 1, 1, 1);
	diet_file_set(diet_parameter(profile, 0), path, DIET_VOLATILE);
	diet_scalar_set(diet_parameter(profile, 1), &is_ip_private, DIET_VOLATILE, DIET_INT);
	env = diet_call(profile);

	if (env == 0) {
		printf("vm destruction OK\n");
	}
	else {
		printf("error in vm destruction\n");
	}

	diet_profile_free(profile);

	diet_finalize();

	return 0;
} /* main */

///*
//** constructVmName: create colon-separated string from an
//**   argv-style series of elements
//*/
//static char *
//constructVmName(int num, char **strings) {
//  char *s;
//  int stringLengthCount = 0;
//  int strIter;
//
//  assert(num > 0);
//  assert(strings != NULL);
//
//  for (strIter = 0; strIter < num; strIter++) {
//    stringLengthCount += strlen(strings[strIter]);
//  }
//  s = calloc(num + stringLengthCount, sizeof(char));
//  assert(s != NULL);
//
//  for (strIter = 0; strIter < num - 1; strIter++) {
//    sprintf(s + strlen(s), "%s:", strings[strIter]);
//  }
//  sprintf(s + strlen(s), "%s", strings[num - 1]);
//
//  return (s);
//} /* constructVmName */

