/**
 * @file strlen.c
 *
 * @brief  Workflow example : a server that computes the length of a stringl
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "DIET_server.h"

#define MAX_TIME_SIZE 64

char time_str[MAX_TIME_SIZE];
long int t = 0;
void
performance_Exec_Time(diet_profile_t *pb, estVector_t perfValues) {
  t = atoi(time_str);
  if (t == 0) {
    t = 10;
  }
  diet_estimate_comptime(perfValues, t * 1000);
  diet_estimate_eft(perfValues, t * 1000, pb);
}

void
set_up_scheduler(diet_profile_desc_t *profile) {
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_min(agg, EST_EFT);
}

int
STRLEN(diet_profile_t *pb) {
  char *str = NULL;
  int *len = NULL;

  fprintf(stderr, "STRLEN SOLVING\n");

  diet_string_get(diet_parameter(pb, 0), &str, NULL);
  diet_scalar_get(diet_parameter(pb, 1), &len, NULL);
  fprintf(stderr, "strlen(%s) = %d\n", str, (int) strlen(str));
  *(int *) len = strlen(str);

  diet_scalar_desc_set(diet_parameter(pb, 1), len);

  usleep(t * 100000);

  return 0;
} /* STRLEN */

int
main(int argc, char *argv[]) {
  int res;
  diet_profile_desc_t *profile = NULL;

  if (argc == 3) {
    strncpy(time_str, argv[2], MAX_TIME_SIZE - 1);
    time_str[MAX_TIME_SIZE - 1] = '\0';
  } else {
    strcpy(time_str, "10");
  }

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("strlen", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_STRING, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);

  set_up_scheduler(profile);

  if (diet_service_table_add(profile, NULL, STRLEN)) {
    return 1;
  }

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return res;
} /* main */
