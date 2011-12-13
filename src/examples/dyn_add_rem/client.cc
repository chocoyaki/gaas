/**
 * @file client.cc
 *
 * @brief  Client used to dynamically add and remove a service
 *
 * @author  Benjamin Depardon (benjamin.depardon@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "DIET_client.h"
#include "DIET_Dagda.h"

void
usage(char *s) {
  std::cout << "Usage: " << s <<
  "<file.cfg> <service number> <number of services to spawn>" << std::endl;
  exit(1);
}

int
checkUsage(int argc, char **argv) {
  if (argc != 4) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  diet_profile_t *profile = NULL;
  std::string service;
  int p1;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    std::cerr << "DIET initialization failed !" << std::endl;
    return 1;
  }

  service = boost::str(boost::format("dyn_add_rem_%0%") % argv[2]);
  p1 = boost::lexical_cast<int>(argv[3]);

  std::cout << "Will call service '" << service << "', with " << p1 <<
  std::endl;

  profile = diet_profile_alloc(strdup(service.c_str()), 0, 0, 0);

  diet_scalar_set(diet_parameter(profile, 0), &p1, DIET_VOLATILE, DIET_INT);

  /* Submit */
  if (diet_call(profile)) {
    return 1;
  }
  std::cout << "DIET CALL finished" << std::endl;


  /* End */
  diet_profile_free(profile);
  diet_finalize();

  return 0;
} // main
