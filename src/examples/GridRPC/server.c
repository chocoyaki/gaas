/****************************************************************************/
/* Server used for GridRPC test cases (July 2006)                          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Abdelkader Amar (Abdelkader.Amar@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2006/09/11 11:12:29  ycaniou
 * Cosmetic changes: C++ commentaries to C commentaries for to discard warnings
 *
 * Revision 1.2  2006/08/09 21:42:23  aamar
 * Changing the error code of EXIT function to 10 (GRPC_SESSION_FAILED)
 *
 * Revision 1.1  2006/07/08 23:04:27  aamar
 * Adding the server used for GridRPC test cases.
 *
 ****************************************************************************/


#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "DIET_server.h"

#define NB_SRV 4

/*
 * The SUCC function
 */

int succ(diet_profile_t* pb) {
  int * i = NULL;
  int * j = NULL;
  diet_scalar_get(diet_parameter(pb, 0), &i, NULL);
  diet_scalar_get(diet_parameter(pb, 1), &j, NULL);
  *j = *i + 1;
  printf ("succ(%d) = %d \n", *i, *j);
  diet_scalar_desc_set(diet_parameter(pb, 1), j); 
  return 0;
}

/*
 * The SLEEP function
 */
int mySleep(diet_profile_t* pb) {
  int * i = NULL;
  diet_scalar_get(diet_parameter(pb, 0), &i, NULL);
  printf("Sleeping for %d seconds \n", *i);
  sleep(*i);
  return 0;
}

/*
 * The LOOP function
 */
int myLoop(diet_profile_t* pb) {
  int * i = NULL;
  diet_scalar_get(diet_parameter(pb, 0), &i, NULL);
  printf("Loop with parameter = %d \n", *i);
  sleep(10);
  printf("The loop is canceled\n");
  return 0;
}

/*
 * The EXIT function
 */
int myExit(diet_profile_t* pb) {
  int * i = NULL;
  diet_scalar_get(diet_parameter(pb, 0), &i, NULL);
  printf("Exit with parameter = %d \n", *i);
  return 10;
}
	
int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg>\n", cmd);
  return 1;
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
    return usage(argv[0]);
  }
  diet_service_table_init(NB_SRV);
  
  /* The succ function */
  profile = diet_profile_desc_alloc("SUCC", 0, 1, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0),
		  	DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 1),
                        DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, succ))
	 return 1;
  diet_profile_desc_free(profile);

  /* The sleep function */
  profile = diet_profile_desc_alloc("SLEEP", 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile, 0),
		        DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, mySleep))
         return 1;
  diet_profile_desc_free(profile);
	    
  /* The loop function */
  profile = diet_profile_desc_alloc("LOOP", 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile, 0),
		        DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, myLoop))
         return 1;
  diet_profile_desc_free(profile);
  
  /* The exit function */
  profile = diet_profile_desc_alloc("EXIT", 0, 0, 0);
  diet_generic_desc_set(diet_param_desc(profile, 0),
		        DIET_SCALAR, DIET_INT);
  if (diet_service_table_add(profile, NULL, myExit))
         return 1;
  diet_profile_desc_free(profile);
  
  diet_print_service_table();

  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
