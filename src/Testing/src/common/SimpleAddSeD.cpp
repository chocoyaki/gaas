#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/test/unit_test.hpp>
#include "DIET_server.h"

int
solve(diet_profile_t *pb);

int
main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: "
              << argv[0]
              << " <configuration file>\n";
    std::cerr << "Please provide a configuration file\n";

    return EXIT_SUCCESS;
  }

  std::string conf(argv[1]);

  BOOST_TEST_MESSAGE("configuration file " << conf);


  diet_profile_desc_t *profile;

  // initialize table with 1 service maximum
  diet_service_table_init(1);

  // allocate profile
  profile = diet_profile_desc_alloc("simple_add", 1, 1, 2);

  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_DOUBLE);

  // add our service to the service table (deep copy)
  diet_service_table_add(profile, NULL, solve);

  // free the profile descriptor
  diet_profile_desc_free(profile);

  // launch the SeD
  diet_SeD(argv[1], argc, argv);

  return EXIT_SUCCESS;
} // main


int
solve(diet_profile_t *pb) {
  double *a = 0;
  double *b = 0;
  double *res = 0;

  // get IN arguments
  diet_scalar_get(diet_parameter(pb, 0), &a, NULL);
  diet_scalar_get(diet_parameter(pb, 1), &b, NULL);

  // get OUT arguments
  diet_scalar_get(diet_parameter(pb, 2), &res, NULL);

  // *COMPUTE something excessively complicated*
  *res = *a + *b;

  // set OUT argument
  diet_scalar_desc_set(diet_parameter(pb, 2), res);

  // free IN arguments
  diet_free_data(diet_parameter(pb, 0));
  diet_free_data(diet_parameter(pb, 1));

  return 0;
} // solve
