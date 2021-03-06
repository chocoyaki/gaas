/**
 * @file file_manip.c
 *
 * @brief  DIET scheduling - JPEG file manipulation (based on jpegtran command)
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

#ifdef __WIN32__
#define msleep(value) (Sleep(value))
#endif

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
greyscale(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path_result = NULL;
  char cmd[1024];

  fprintf(stderr, "GREYSCALE SOLVING\n");

  diet_file_get(diet_parameter(pb, 0), &path1, NULL, &arg_size);
  fprintf(stderr, "on %s (%lu) \n", path1, arg_size);


  path_result = (char *) malloc(strlen(path1) + 10);

  strcpy(path_result, path1);
  strcat(path_result, "-gray.jpg");

  sprintf(cmd, "jpegtran -grayscale -outfile %s-gray.jpg %s",
          path1, path1);

  printf("%s\n", cmd);

  printf("%d\n", system(cmd));

  if (diet_file_desc_set(diet_parameter(pb, 1), path_result)) {
    printf("diet_file_desc_set error\n");
    free(path_result);
    return 1;
  }

#ifdef __WIN32__
	msleep(t*100);
#else
	usleep(t * 100000);
#endif

  return 0;
} /* greyscale */


int
flip(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path_result = NULL;
  char cmd[1024];

  fprintf(stderr, "FLIP SOLVING\n");

  diet_file_get(diet_parameter(pb, 0), &path1, NULL, &arg_size);
  fprintf(stderr, "on %s (%lu) \n", path1, arg_size);

  path_result = (char *) malloc(strlen(path1) + 10);

  sprintf(cmd, "jpegtran -flip horizontal -outfile %s-flip.jpg %s",
          path1, path1);

  strcpy(path_result, path1);
  strcat(path_result, "-flip.jpg");


  printf("flip cmd = %s\n", cmd);
  printf("%d\n", system(cmd));
  printf("@@@@@@@@@@@@@ Path of result: %s\n", path_result);
  if (diet_file_desc_set(diet_parameter(pb, 1), path_result)) {
    printf("diet_file_desc_set error\n");
    return 1;
  }

#ifdef __WIN32__
	msleep(t*100);
#else
	usleep(t * 100000);
#endif

  return 0;
} /* flip */

int
duplicate(diet_profile_t *pb) {
  size_t arg_size = 0;
  char *path1 = NULL;
  char *path_result1 = NULL;
  char *path_result2 = NULL;
  char cmd[1024];
  int ret = 0;

  fprintf(stderr, "DUPLICATE SOLVING\n");

  diet_file_get(diet_parameter(pb, 0), &path1, NULL, &arg_size);
  fprintf(stderr, "on %s (%lu) \n", path1, arg_size);


  path_result1 = (char *) malloc(strlen(path1) + 10);
  path_result2 = (char *) malloc(strlen(path1) + 10);

  strcpy(path_result1, path1);
  strcat(path_result1, "-1.jpg");

  strcpy(path_result2, path1);
  strcat(path_result2, "-2.jpg");

  sprintf(cmd, "cp %s %s-1.jpg",
          path1, path1);

  printf("duplicate %d\n", system(cmd));

  sprintf(cmd, "cp %s %s-2.jpg",
          path1, path1);

  printf("duplicate %d\n", system(cmd));

  if (diet_file_desc_set(diet_parameter(pb, 1), path_result1)) {
    printf("diet_file_desc_set error\n");
    ret = 1;
  }
  if (diet_file_desc_set(diet_parameter(pb, 2), path_result2)) {
    printf("diet_file_desc_set error\n");
    ret = 1;
  }

#ifdef __WIN32__
	msleep(t*100);
#else
	usleep(t * 100000);
#endif

  return ret;
} /* duplicate */

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

  diet_service_table_init(3);

  profile = diet_profile_desc_alloc("greyscale", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, greyscale)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  profile = diet_profile_desc_alloc("flip", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, flip)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  profile = diet_profile_desc_alloc("duplicate", 0, 0, 2);
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, duplicate)) {
    return 1;
  }
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return res;
} /* main */
