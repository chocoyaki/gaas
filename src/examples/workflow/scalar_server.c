/**
 * @file scalar_server.c
 *
 * @brief  Include services needed to execute the workflow in scalar.xml
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __WIN32__
#include <unistd.h>
#else
#include <Winsock2.h>
#include <windows.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

#include "DIET_server.h"

#define MAX_TIME_SIZE 64

char time_str[MAX_TIME_SIZE];
long int t = 0;

void
performance_Exec_Time(diet_profile_t *pb, estVector_t perfValues) {
  double eft, tcomp;
  t = atoi(time_str);
  if (t == 0) {
    t = 10;
  }
  /* Set the job duration and compute SeD's EFT (results stored in EV) */
  diet_estimate_comptime(perfValues, t * 1000);
  diet_estimate_eft(perfValues, t * 1000, pb);
  /* Get the values from EV */
  tcomp = diet_est_get_system(perfValues, EST_TCOMP, 10000000);
  eft = diet_est_get_system(perfValues, EST_EFT, 10000000);
  printf("TCOMP=%f / EFT=%f\n", tcomp, eft);
}  /* performance_Exec_Time */

void
set_up_scheduler(diet_profile_desc_t *profile) {
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_min(agg, EST_EFT);
}

/**
 * The SUCC service
 */
int
succ(diet_profile_t *pb) {
  int *ix = NULL;
  int *jx = NULL;
  int *kx = NULL;

  fprintf(stderr, "SUCC SOLVING\n");

  diet_scalar_get(diet_parameter(pb, 0), &ix, NULL);
  fprintf(stderr, "i = %d\n", *(int *) ix);
  jx = (int *) malloc(sizeof(int));
  kx = (int *) malloc(sizeof(int));
  *(int *) jx = *(int *) ix + 1;
  *(int *) kx = *(int *) ix + 1;
  fprintf(stderr, "succ(i) = %d\n", *(int *) jx);
  diet_scalar_desc_set(diet_parameter(pb, 1), jx);
  diet_scalar_desc_set(diet_parameter(pb, 2), kx);

  /* diet::usleep(t * 100000); */

  return 0;
}  /* succ */

/**
 * The DOUBLE service
 */
int
double_int(diet_profile_t *pb) {
  int *ix = NULL;
  int *jx = NULL;

  fprintf(stderr, "DOUBLE SOLVING\n");

  diet_scalar_get(diet_parameter(pb, 0), &ix, NULL);
  fprintf(stderr, "i = %d\n", *(int *) ix);
  jx = (int *) malloc(sizeof(int));
  *(int *) jx = *(int *) ix * 2;
  fprintf(stderr, "j = 2i = %d\n", *(int *) jx);
  diet_scalar_desc_set(diet_parameter(pb, 1), jx);

  /* usleep(t * 100000); */

  return 0;
}  /* double_int */


/**
 * The SUM service
 */
int
sum(diet_profile_t *pb) {
  int *ix = NULL;
  int *jx = NULL;
  int *kx = NULL;

  fprintf(stderr, "SUM SOLVING\n");
  diet_scalar_get(diet_parameter(pb, 0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb, 1), &jx, NULL);
  fprintf(stderr, "i = %d, j =%d\n", *(int *) ix, *(int *) jx);
  kx = (int *) malloc(sizeof(int));
  *(int *) kx = *(int *) ix + *(int *) jx;
  fprintf(stderr, "k = i + j = %d\n", *(int *) kx);
  diet_scalar_desc_set(diet_parameter(pb, 2), kx);

  /* usleep(t * 100000); */

  return 0;
}  /* sum */

/**
 * The SQUARE service
 */
int
square(diet_profile_t *pb) {
  int *ix = NULL;
  double *jx = NULL;

  fprintf(stderr, "SQUARE SOLVING\n");

  diet_scalar_get(diet_parameter(pb, 0), &ix, NULL);
  fprintf(stderr, "i = %d\n", *(int *) ix);
  jx = (double *) malloc(sizeof(double));
  *(double *) jx = sqrt(*(int *) ix);
  fprintf(stderr, "sqrt(i) = %f\n", *(double *) jx);
  diet_scalar_desc_set(diet_parameter(pb, 1), jx);

  /* usleep(t * 100000); */

  return 0;
}  /* square */

int
main(int argc, char *argv[]) {
  int res;
  diet_profile_desc_t *profile = NULL;

  if (argc >= 3) {
    strncpy(time_str, argv[2], MAX_TIME_SIZE - 1);
    time_str[MAX_TIME_SIZE - 1] = '\0';
  } else {
    strcpy(time_str, "10");
  }

  diet_service_table_init(4);

  /* Add the SUCC service */
  profile = diet_profile_desc_alloc("succ", 0, 0, 2);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, succ)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  /* Add the DOUBLE service */
  profile = diet_profile_desc_alloc("double", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, double_int)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  /* Add the SUM service */
  profile = diet_profile_desc_alloc("sum", 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, sum)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  /* Add the SQUARE service */
  profile = diet_profile_desc_alloc("square", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_DOUBLE);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, square)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return res;
}  /* main */
