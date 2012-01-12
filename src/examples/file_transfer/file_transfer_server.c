/**
 * @file file_transfer_server.c
 *
 * @brief  file_transfer example: server side
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
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




int
solve_size(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path2 = NULL;
  int status = 0;
  struct stat buf;
  size_t *s1 = NULL;
  size_t *s2 = NULL;

  fprintf(stderr, "Solve size ");

  diet_file_get(diet_parameter(pb, 0), &path1, NULL, &arg_size);
  fprintf(stderr, "on %s (%lu) ", path1, arg_size);
  if ((status = stat(path1, &buf))) {
    return status;
  }
  /* Regular file */
  if (!(buf.st_mode & S_IFREG)) {
    return 2;
  }
  s1 = (size_t *)calloc(1, sizeof *s1);
  *s1 = buf.st_size;
  diet_scalar_set(diet_parameter(pb, 2), s1, DIET_VOLATILE, DIET_INT);

  diet_file_get(diet_parameter(pb, 1), &path2, NULL, &arg_size);
  fprintf(stderr, "and %s (%lu) ...", path2, arg_size);
  if ((status = stat(path2, &buf))) {
    return status;
  }
  if (!(buf.st_mode & S_IFREG)) {
    return 2;
  }
  s2 = (size_t *)calloc(1, sizeof *s2);
  *s2 = buf.st_size;
  diet_scalar_set(diet_parameter(pb, 3), s2, DIET_VOLATILE, DIET_INT);

  path2 = strdup(path1);

  if (diet_file_set(diet_parameter(pb, 4), path2, DIET_VOLATILE)) {
    printf("diet_file_desc_set error\n");
    return 1;
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
  profile = diet_profile_desc_alloc("size", 1, 1, 4);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 4), DIET_FILE, DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solve_size)) {
    return 1;
  }

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
} /* main */
