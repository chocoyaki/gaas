#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"
#include "DIET_Dagda.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char *argv[]) {
  diet_profile_t *profile = NULL;
  size_t out_size = 0;
  char cmd[2048];

  if (argc < 3) {
    fprintf(stderr, "Usage: %s <file.cfg> dagda_id\n", argv[0]);
    return 1;
  }


  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

	char* dagda_id = argv[2];
	char* path = NULL;
	dagda_get_file(dagda_id,  &path);

	printf("file got : %s\n", path);


  diet_finalize();

  return 0;
} /* main */
