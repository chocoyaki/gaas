/****************************************************************************/
/* Client used to dynamically add and remove a service                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/11/18 10:18:13  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 *
 ****************************************************************************/
#include <iostream>
#include <cstring>
#include <cstdlib>

#include "DIET_client.h"
#include "DIET_Dagda.h"

void usage(char * s) {
  std::cout << "Usage: " << s << "<file.cfg> <service number> <number of services to spawn>" << std::endl;
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if (argc != 4) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
  diet_profile_t* profile = NULL;
  std::string service;
  int p1;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  } 

  service = "dyn_add_rem_" + std::string(argv[2]);
  p1 = atoi(argv[3]);

  std::cout << "Will call service '" << service << "', with " << p1 << std::endl;

  profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);
  
  diet_scalar_set(diet_parameter(profile, 0), &p1, DIET_VOLATILE, DIET_INT);

  /* Submit */
  if (diet_call(profile)) {
    return 1;
  }
  printf("DIET CALL finished\n");


  /* End */
  diet_profile_free(profile);
  diet_finalize();

  return 0;
}