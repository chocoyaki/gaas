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
 * Revision 1.2  2011/02/01 22:49:57  bdepardo
 * Remove memory leak
 *
 * Revision 1.1  2010/08/06 14:25:29  glemahec
 * Cmake corrections + uuid lib module + fPIC error control
 *
 * Revision 1.19  2010/03/31 02:24:47  ycaniou
 * Warnings--
 * Remove unecessary includes
 *
 * Revision 1.18  2010/03/23 12:44:19  glemahec
 * Correction des exemples pour DAGDA
 *
 * Revision 1.17  2010/03/05 15:52:09  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.16  2007/06/11 09:34:48  ycaniou
 * Modified the example, so that the default behavior of the server is now
 *   to return the out file, and not a random behavior
 *
 * Revision 1.15  2006/09/18 19:46:07  ycaniou
 * Corrected a bug in file_transfer:server.c
 * Corrected memory leaks due to incorrect free of char *
 * ServiceTable prints if service is sequential or parallel
 * Fully complete examples, whith a batch, a parallel and a sequential server and
 *  a unique client
 *
 * Revision 1.14  2006/06/30 15:21:09  ycaniou
 * Commentaries
 *
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
  char* path1 = NULL;
  char* path2 = NULL;
  int status = 0;
  struct stat buf;
  size_t* s1 = NULL;
  size_t* s2 = NULL;

  fprintf(stderr, "Solve size ");
  
  diet_file_get(diet_parameter(pb, 0), NULL, &arg_size, &path1);
  fprintf(stderr, "on %s (%zd) ", path1, arg_size);
  if ((status = stat(path1, &buf)))
    return status;
  /* Regular file */
  if (!(buf.st_mode & S_IFREG))
    return 2;
  s1 = calloc(1, sizeof *s1);
  *s1 = buf.st_size;
  diet_scalar_set(diet_parameter(pb, 2), s1, DIET_VOLATILE, DIET_INT);
  
  diet_file_get(diet_parameter(pb, 1), NULL, &arg_size, &path2);
  fprintf(stderr, "and %s (%zd) ...", path2, arg_size);
  if ((status = stat(path2, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  s2 = calloc(1, sizeof *s2);
  *s2 = buf.st_size;
  diet_scalar_set(diet_parameter(pb, 3), s2, DIET_VOLATILE, DIET_INT);
  
  path2 = strdup(path1);
  
  if (diet_file_set(diet_parameter(pb, 4), DIET_VOLATILE, path2)) {
    printf("diet_file_desc_set error\n");
    return 1;
  }
  printf(" done\n");

  printf("Returned file: %s.\n", path2);

  /* **************
     Don't free the string names since they are not replicated from CORBA obj
     ************** */
  
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
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 3), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 4), DIET_FILE, DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solve_size)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}


