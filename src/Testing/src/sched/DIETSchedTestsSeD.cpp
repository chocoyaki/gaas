#include <cstddef>
#include <DIET_server.h>
#include <DIET_grpc.h>
#include "DIETSchedTestsCommon.hpp"

int testsched_handler(diet_profile_t *p) {
  return GRPC_NO_ERROR;
}

void testsched_perfmetric(diet_profile_t *profile, estVector_t estvec) {
  diet_est_set(estvec, EST_CUSTOMMETRIC1, 1.0);
  diet_est_set(estvec, EST_CUSTOMMETRIC2, 2.0);
  diet_est_set(estvec, EST_CUSTOMMETRIC3, 3.0);
  diet_est_set(estvec, EST_CUSTOMMETRIC4, 4.0);
  diet_est_set(estvec, EST_CUSTOMMETRIC5, 5.0);
}

int main(int argc, char **argv) {

  diet_profile_desc_t *profile;
  diet_aggregator_desc_t *agg;

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("testsched", 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_DOUBLE);
  agg = diet_profile_desc_aggregator(profile);
  diet_aggregator_set_type(agg, DIET_AGG_USER);
  diet_service_use_perfmetric(testsched_perfmetric);
  diet_service_table_add(profile, NULL, testsched_handler);
  diet_profile_desc_free(profile);
  diet_SeD(argv[1], argc, argv);
  return 0;
}
