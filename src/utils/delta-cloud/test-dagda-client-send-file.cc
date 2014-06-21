/**
 * @file test-dagda-client-send-file.cc
 *
 * @brief  store a file with dagda example: client side
 *
 * @author  Lamiel Toch (Lamiel.Toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <string.h>

#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"
#include "DIET_Dagda.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char *argv[]) {
  char *path = NULL;
  diet_profile_t *profile = NULL;
  int *size1 = NULL;
  int *size2 = NULL;
  size_t out_size = 0;


  if (argc < 3) {
    fprintf(stderr, "Usage: %s <file.cfg> <file_to_store> \n", argv[0]);
    return 1;
  }

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc("store", 0, 0, 0);
  if (diet_file_set(diet_parameter(profile, 0), argv[2], DIET_PERSISTENT)) {
    printf("diet_file_set error\n");
    return 1;
  }

  if (!diet_call(profile)) {

  }

  diet_profile_free(profile);


	char* dagda_id = NULL;
	int env = dagda_put_file("/home/lamiel/cmd/truc.txt", DIET_PERSISTENT, &dagda_id);

	if (env == 0) {
		printf("success! %s\n", dagda_id);
	}

	//save the platform data state
	env = dagda_save_platform();

	if (env == 0 ){
		printf("save platform data: success!!\n");
	}
	else {
		printf("fails to save platform data\n");
	}

  diet_finalize();





  return 0;
} /* main */
