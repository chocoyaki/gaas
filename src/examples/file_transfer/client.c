/****************************************************************************/
/* $Id$ */
/* file_transfer example: client side                                       */
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


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char **argv)
{
  char *path;
  diet_function_handle_t *fhandle;
  diet_profile_t *profile;
  int *size1, *size2;


  if (argc != 4) {
    fprintf(stderr, "Usage: client <config_file> <file1> <file2>\n");
    return 1;
  }
  path = "size";
  
  if (diet_initialize(argc, argv, argv[1])) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  fhandle = diet_function_handle_default(path);
  profile = profile_alloc(1, 1, 4);
  if (file_set(&(profile->parameters[0]), DIET_VOLATILE, argv[2])) {
    printf("file_set error\n");
    return 1;
  }
  if (file_set(&(profile->parameters[1]), DIET_VOLATILE, argv[3])) {
    printf("file_set error\n");
    return 1;
  }
  scalar_set(&(profile->parameters[2]), NULL, DIET_VOLATILE, DIET_INT);
  scalar_set(&(profile->parameters[3]), NULL, DIET_VOLATILE, DIET_INT);
  if (file_set(&(profile->parameters[4]), DIET_VOLATILE, NULL)) {
    printf("file_set error\n");
    return 1;
  }

  if (!diet_call(fhandle, profile)) {
    size1 = ((int *)(profile->parameters[2].value));
    size2 = ((int *)(profile->parameters[3].value));
    printf("Answered sizes are %d and %d.\n", *size1, *size2);
    printf("Location of returned file is %s.\n",
	   profile->parameters[4].desc.specific.file.path);
    free(size1);
    free(size2);
  }
  
  profile_free(profile);
  diet_function_handle_destruct(fhandle);
    
  diet_finalize();

  return 0;
}



