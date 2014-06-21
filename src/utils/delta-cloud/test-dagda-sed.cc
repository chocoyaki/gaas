/**
 * @file test-dagda-sed.cc
 *
 * @brief  exemple dagda example: server side
 *
 * @author  Lamiel Toch (Lamiel.Toch@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "DIET_server.h"
#include "DIET_Dagda.h"



int
solve_store(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path2 = NULL;
  int status = 0;
  struct stat buf;
  size_t *s1 = NULL;
  size_t *s2 = NULL;

  fprintf(stderr, "Solve store ");

  diet_file_get(diet_parameter(pb, 0), &path1, NULL, &arg_size);
  fprintf(stderr, "on %s (%lu) ", path1, arg_size);
  if ((status = stat(path1, &buf))) {
    return status;
  }



  path2 = strdup(path1);

  printf("\nadd %s in dagda\n", path2);
  char* dagda_id = NULL;
	int env = dagda_put_file(path2, DIET_PERSISTENT, &dagda_id);

	if (env == 0) {
		printf("success! %s\n", dagda_id);
	}


  printf(" done\n");

  printf("Returned file: %s.\n", path2);

  /* **************
     Don't free the string names since they are not replicated from CORBA obj
  ************** */

  return 0;
} /* solve_size */


/*
 * MAIN
 */

int
main(int argc, char *argv[]) {
  int res;
  diet_profile_desc_t *profile = NULL;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }

  /* This is for solve function (OUT parameter) */
  srand(time(NULL));


  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("store", 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solve_store)) {
    return 1;
  }

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
} /* main */
