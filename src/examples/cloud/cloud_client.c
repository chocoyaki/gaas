/****************************************************************************/
/* Example client for the Cloud Batch System                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Adrian Muresan (adrian.muresan@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2010/08/23 09:44:28  amuresan
 * Fixed cmake conflicting target problem
 *
 * Revision 1.3  2010/06/15 08:10:54  amuresan
 * Added cloud configuration files to src/examples/cfgs/
 *
 * Revision 1.2  2010/06/14 21:27:24  amuresan
 * Added header to cloud example files and client config file.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "DIET_client.h"
int main(int argc, char **argv)
{
  diet_profile_t          *profile;
  char * result;
/* Initialize a DIET session */
  diet_initialize("./client_cloud.cfg", argc, argv);
  /* Create the profile as explained in Chapter 3 */
  profile = diet_profile_alloc("cloud-demo",-1, -1, 0); // last_in, last_inout, last_out
  /* Set profile arguments */
  diet_string_set(diet_parameter(profile,0), NULL,    DIET_VOLATILE);
	printf("here\n");
  if (!diet_parallel_call(profile)) { /* If the call has succeeded ... */
    /* Get and print time */
    diet_string_get(diet_parameter(profile,0), &result, NULL);
    if (result == NULL) {
      printf("Error: result is null !\n");
    } else {
      printf("Result is %s\n", result);
    }
  }
  else
  {
		printf("calling error...\n");
  }

  /* Free profile */
  diet_profile_free(profile);
  diet_finalize();
}

