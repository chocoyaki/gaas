/****************************************************************************/
/* $Id$ */
/* file_transfer example: client side                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.6  2003/01/17 18:05:37  pcombes
 * Update to API 0.6.3
 *
 * Revision 1.5  2002/12/12 18:17:05  pcombes
 * Small bug fixes on prints (special thanks to Jean-Yves)
 *
 * Revision 1.3  2002/10/18 18:13:21  pcombes
 * Bug fixes for files in OUT parameters.
 *
 * Revision 1.2  2002/10/17 15:36:37  pcombes
 * Two files are transfered for size.
 *
 * Revision 1.1.1.1  2002/10/15 18:52:03  pcombes
 * Example of a file transfer: size of the file.
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
  int *size1, *size2, out_size;


  if (argc != 4) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <file2>\n", argv[0]);
    return 1;
  }
  path = "size";
  
  if (diet_initialize(argc, argv, argv[1])) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  fhandle = diet_function_handle_default(path);
  profile = diet_profile_alloc(1, 1, 4);
  if (diet_file_set(diet_parameter(profile,0), DIET_VOLATILE, argv[2])) {
    printf("diet_file_set error\n");
    return 1;
  }
  if (diet_file_set(diet_parameter(profile,1), DIET_VOLATILE, argv[3])) {
    printf("diet_file_set error\n");
    return 1;
  }
  diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_INT);
  diet_scalar_set(diet_parameter(profile,3), NULL, DIET_VOLATILE, DIET_INT);
  if (diet_file_set(diet_parameter(profile,4), DIET_VOLATILE, NULL)) {
    printf("diet_file_set error\n");
    return 1;
  }

  if (!diet_call(fhandle, profile)) {
    size1 = diet_value(int, diet_parameter(profile,2));
    size2 = diet_value(int, diet_parameter(profile,3));
    printf("Answered sizes are %d and %d.\n", *size1, *size2);
    free(size1);
    free(size2);
    diet_file_get(diet_parameter(profile,4), NULL, &out_size, &path);
    printf("Location of returned file is %s, its size is %d.\n",
	   path, out_size);
    if (path)
      free(path);
  }
  
  diet_profile_free(profile);
  diet_function_handle_destruct(fhandle);
    
  diet_finalize();

  return 0;
}
