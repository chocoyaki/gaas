/****************************************************************************/
/* $Id$ */
/* file_transfer example: server side                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.3  2002/10/18 18:13:21  pcombes
 * Bug fixes for files in OUT parameters.
 *
 * Revision 1.2  2002/10/17 15:36:37  pcombes
 * Two files are transfered for size.
 *
 * Revision 1.1.1.1  2002/10/15 18:52:03  pcombes
 * Example of a file transfer: size of the file.
 *
 * Revision 1.1.1.1  2002/10/15 18:48:18  pcombes
 * Add new file transfer example.
 *
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "DIET_server.h"


/*
 * SOLVE FUNCTIONS
 */


int
solve_size(diet_data_seq_t *in, diet_data_seq_t *inout, diet_data_seq_t *out)
{
  size_t arg_size, comp_size;
  char *path;
  int status = 0;
  struct stat buf;

  printf("Solve size ");

  arg_size = in->seq[0].desc.specific.file.size;
  path = in->seq[0].desc.specific.file.path;
  printf("on %s ", path);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  *((size_t *)out->seq[0].value) = (size_t) buf.st_size;
  
  arg_size = in->seq[1].desc.specific.file.size;
  path = in->seq[1].desc.specific.file.path;
  printf("and %s ...", path);
  if ((status = stat(path, &buf)))
    return status;
  if (!(buf.st_mode & S_IFREG))
    return 2;
  *((size_t *)out->seq[1].value) = (size_t) buf.st_size;
  
  if (file_set(&(out->seq[2]), DIET_VOLATILE, path)) {
    printf("file_set error\n");
    return 1;
  }

  printf(" done\n");
  return 0;
}


/*
 * MAIN
 */

int
main(int argc, char **argv)
{
  size_t i,j;
  int res;

  diet_profile_desc_t *profile;

  if (argc < 2) {
    fprintf(stderr, "Usage: SeD <file>\n");
    return 1;
  }  

  diet_service_table_init(1);
  profile = profile_desc_alloc(1, 1, 4);
  generic_desc_set(&(profile->param_desc[0]), DIET_FILE, DIET_CHAR);
  generic_desc_set(&(profile->param_desc[1]), DIET_FILE, DIET_CHAR);
  generic_desc_set(&(profile->param_desc[2]), DIET_SCALAR, DIET_INT);
  generic_desc_set(&(profile->param_desc[3]), DIET_SCALAR, DIET_INT);
  generic_desc_set(&(profile->param_desc[4]), DIET_FILE, DIET_CHAR);
  diet_service_table_add("size", profile, NULL, solve_size);

  profile_desc_free(profile);
  print_table();
  res = DIET_SeD(argv[1], argc, argv);
  // Not reached
  return res;
}


