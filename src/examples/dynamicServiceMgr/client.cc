/****************************************************************************/
/* Client used to dynamically add and remove a service using a library      */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Depardon (benjamin.depardon@ens-lyon.fr)                      */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2010/01/14 10:58:09  bdepardo
 * Changes to compile with gcc 4.4
 *
 * Revision 1.1  2008/12/08 15:32:43  bdepardo
 * Added an example to dynamically load a service given a library:
 * the library is sent by the client, and the SeD loads it and uses the new
 * service(s) added.
 *
 *
 ****************************************************************************/

#include <iostream>
#include <cstring>
#include <cstdlib>

#include "DIET_client.h"
#include "DIET_Dagda.h"

void usage(char * s) {
  std::cout << "Usage: " << s << " <file.cfg> [add/rem/call] [lib/service Name/service Name] [//message]" << std::endl;
  exit(1);
}


int checkUsage(int argc, char ** argv) {
  if (argc < 4) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
  diet_profile_t* profile = NULL;
  std::string service;
  std::string tmp;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    std::cerr << "DIET initialization failed !" << std::endl;
    return 1;
  } 

  if (!strcmp(argv[2], "add")) {
    service = "addDyn";
    profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);
    if (diet_file_set(diet_parameter(profile, 0), DIET_PERSISTENT, argv[3])) {
      std::cerr << "Problem while sending file '" << argv[3] << "'" << std::endl;
      return 1;
    }
  } else if (!strcmp(argv[2], "rem")) {
    service = "remDyn";
    std::cout << "*** removing '" << argv[3] << "'" << std::endl;
    profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);
    diet_string_set(diet_parameter(profile, 0), argv[3], DIET_VOLATILE);
  } else if (!strcmp(argv[2], "call")){
    service = argv[3];
    profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);
    diet_string_set(diet_parameter(profile, 0), argv[4], DIET_VOLATILE);
  } else {
    diet_finalize();
    return -1;
  }

  /* Submit */
  std::cout << "*** Profile set, will now call the service ***" << std::endl;
  if (diet_call(profile)) {
    return 1;
  }
  std::cout << "DIET CALL finished" << std::endl;


  /* End */
  diet_profile_free(profile);
  diet_finalize();

  return 0;
}
