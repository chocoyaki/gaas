#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "DIET_client.h"
int main(int argc, char **argv)
{
  diet_profile_t          *profile;
  char * result;
/* Initialize a DIET session */
  diet_initialize("./client.cfg", argc, argv);
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

