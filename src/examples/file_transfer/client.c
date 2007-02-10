/****************************************************************************/
/* file_transfer example: client side                                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.12  2007/02/10 13:10:25  ycaniou
 * Memory weirdos: data were freed but CORBA managed
 * + comments
 *
 * Revision 1.11  2005/11/09 18:39:33  alsu
 * casting size_t arguments to int to deal with the storage size difference on 64-bit architectures
 *
 * Revision 1.10  2005/10/05 14:02:29  alsu
 * eliminating gcc warning about signedness
 *
 * Revision 1.9  2003/07/25 20:37:37  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.8  2003/04/10 13:31:21  pcombes
 * Update to empty (not only NULL) file paths.
 *
 * Revision 1.7  2003/02/07 17:04:42  pcombes
 * Use diet_free_data to properly free user's data.
 *
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
 ****************************************************************************/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the file to transfer */

int
main(int argc, char* argv[])
{
  char* path = NULL;
  diet_profile_t* profile = NULL;
  int *size1 = NULL;
  int *size2 = NULL;
  size_t out_size = 0;


  if (argc != 4) {
    fprintf(stderr, "Usage: %s <file.cfg> <file1> <file2>\n", argv[0]);
    return 1;
  }
  path = "size";
  
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  profile = diet_profile_alloc(path, 1, 1, 4);
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

  if (!diet_call(profile)) {
    diet_scalar_get(diet_parameter(profile,2), &size1, NULL);
    diet_scalar_get(diet_parameter(profile,3), &size2, NULL);
    if (size1 && size2) {
      printf("Answered sizes are %d and %d.\n", *size1, *size2);
      // No need to free size1 and size2: CORBA takes care of them
    } else {
      fprintf(stderr, "Error: Cannot get answered sizes !\n");
    }
    diet_file_get(diet_parameter(profile,4), NULL, &out_size, &path);
    if (path && (*path != '\0')) {
      printf("Location of returned file is %s, its size is %d.\n",
	     path, (int) out_size);
      // If uncommented, next line unlink file
      // diet_free_data(diet_parameter(profile,4));
    }
  }
  
  diet_profile_free(profile);
    
  diet_finalize();

  return 0;
}
