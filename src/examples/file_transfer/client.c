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
  int *size;


  if (argc != 3) {
    fprintf(stderr, "Usage: client <config_file> <file>\n");
    return 1;
  }
  path = "size";
  
  if (diet_initialize(argc, argv, argv[1])) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  fhandle = diet_function_handle_default(path);
  profile = profile_alloc(0, 0, 1);
  if (file_set(&(profile->parameters[0]), DIET_VOLATILE, argv[2])) {
    printf("file_set error\n");
    return 1;
  }
  scalar_set(&(profile->parameters[1]), NULL, DIET_VOLATILE, DIET_INT);

  if (!diet_call(fhandle, profile)) {
    size = ((int *)(profile->parameters[1].value));
    printf("Answered size is %d.\n", *size);
    free(size);
  }
  
  profile_free(profile);
  diet_function_handle_destruct(fhandle);
    
  diet_finalize();

  return 0;
}



