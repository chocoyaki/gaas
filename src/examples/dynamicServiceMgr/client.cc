/**
 * @file client.cc
 *
 * @brief  Client used to dynamically add and remove a service using a library
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <iostream>
#include <cstring>
#include <cstdlib>

#include "DIET_client.h"
#include "DIET_Dagda.h"

void
usage(char *s) {
  std::cout << "Usage: " << s <<
  " <file.cfg> [add/rem/call] [lib/service Name/service Name] [//message]" <<
  std::endl;
  exit(1);
}


int
checkUsage(int argc, char **argv) {
  if (argc < 4) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  diet_profile_t *profile = NULL;
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
    if (diet_file_set(diet_parameter(profile, 0), argv[3], DIET_PERSISTENT)) {
      std::cerr << "Problem while sending file '" << argv[3] << "'" <<
      std::endl;
      return 1;
    }
  } else if (!strcmp(argv[2], "rem")) {
    service = "remDyn";
    std::cout << "*** removing '" << argv[3] << "'" << std::endl;
    profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);
    diet_string_set(diet_parameter(profile, 0), argv[3], DIET_VOLATILE);
  } else if (!strcmp(argv[2], "call")) {
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
} // main
