/****************************************************************************/
/* file_transfer example: server side                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.13  2005/11/09 18:39:33  alsu
 * casting size_t arguments to int to deal with the storage size difference on 64-bit architectures
 *
 * Revision 1.12  2003/09/27 07:53:38  pcombes
 * Remove unused displayArg function.
 *
 * Revision 1.11  2003/08/09 17:32:47  pcombes
 * Update to the new diet_profile_desc_t.
 *
 * Revision 1.10  2003/04/10 13:31:38  pcombes
 * Use diet_file_get.
 *
 * Revision 1.9  2003/02/07 17:04:42  pcombes
 * Use diet_free_data to properly free user's data.
 *
 * Revision 1.6  2002/12/12 18:17:05  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 * Revision 1.2  2002/10/17 15:36:37  pcombes
 * Two files are transfered for size.
 *
 * Revision 1.1.1.1  2002/10/15 18:52:03  pcombes
 * Example of a file transfer: size of the file.
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "DIET_server.h"


/*
 * SOLVE FUNCTIONS
 */


int
solve_size(diet_profile_t* pb)
{
  size_t arg_size  = 0;
  char* path = NULL;
  int status = 0;
  struct stat buf;

  fprintf(stderr, "Solve size ");
  
  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path);
  fprintf(stderr, "on %s (%d) ", path, (int) arg_size);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  diet_scalar_desc_set(diet_parameter(pb,2), &buf.st_size);
  
  diet_free_data(diet_parameter(pb,0));
  
  diet_file_get(diet_parameter(pb,1), NULL, &arg_size, &path);
  fprintf(stderr, "and %s (%d) ...", path, (int) arg_size);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  diet_scalar_desc_set(diet_parameter(pb,3), &buf.st_size);

  // do not apply diet_free_data on param 1, since it is also the OUT file.

  if (diet_file_desc_set(diet_parameter(pb,4), (rand()&1) ? path : NULL)) {
    printf("diet_file_desc_set error\n");
    return 1;
  }
  printf(" done\n");
  diet_file_get(diet_parameter(pb,4), NULL, NULL, &path);
  if (path)
    printf("Returned file: %s.\n", path);
  return 0;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  int res;
  diet_profile_desc_t* profile = NULL;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <file.cfg>\n", argv[0]);
    return 1;
  }  

  /* This is for solve function (OUT parameter) */
  srand(time(NULL));


  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("size", 1, 1, 4);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,4), DIET_FILE, DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solve_size)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


