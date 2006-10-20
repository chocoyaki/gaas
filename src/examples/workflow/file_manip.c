/****************************************************************************/
/* DIET scheduling - JPEG file manipulation (based on jpegtran command)     */
/*                                                                          */
/*  Author(s):                                                              */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id: 
 * $Log: 
 ****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>

#include "DIET_server.h"

char time_str[64];
long int t = 0;

void
performance_Exec_Time(diet_profile_t* pb ,estVector_t perfValues )
{
  t = atoi(time_str);
  if ( t == 0 )
    t = 10;
  diet_est_set(perfValues, 0, t);
}

void 
set_up_scheduler(diet_profile_desc_t* profile){ 
  diet_aggregator_desc_t *agg = NULL;

  agg = diet_profile_desc_aggregator(profile);

  diet_estimate_cori_add_collector(EST_COLL_EASY,NULL);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY); 
  diet_aggregator_priority_minuser(agg,0);
}


int
greyscale(diet_profile_t* pb)
{
  size_t arg_size  = 0;
  char* path1 = NULL;
  char* path_without_ext = NULL;
  char* path_result = NULL;
  char cmd[1024];

  fprintf(stderr, "GREYSCALE SOLVING\n");

  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path1) ;
  fprintf(stderr, "on %s (%d) \n", path1, (int) arg_size);


  path_result = (char*)malloc(strlen(path1) + 6);
  path_without_ext = (char*)malloc(strlen(path1) - 2);
  strncpy(path_without_ext, path1, strlen(path1) - 4);
  path_without_ext[strlen(path1) - 4] = 0;

  strcpy(path_result, path_without_ext);
  strcat(path_result, "-gray.jpg");

  sprintf(cmd, "jpegtran -grayscale -outfile %s-gray.jpg %s.jpg",
	  path_without_ext, path_without_ext);
  
  printf("%s\n", cmd);

  printf("%d\n", system(cmd));

  if (diet_file_desc_set(diet_parameter(pb,1), path_result)) {
    printf("diet_file_desc_set error\n");
    free(path_result);
    free(path_without_ext);
    return 1;
  }

  usleep(t*500000);
  free(path_without_ext);
  
  return 0;
}


int
flip(diet_profile_t* pb)
{
  size_t arg_size  = 0;
  char* path1 = NULL;
  char* path_without_ext = NULL;
  char* path_result = NULL;
  char cmd[1024];

  fprintf(stderr, "FLIP SOLVING\n");

  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path1) ;
  fprintf(stderr, "on %s (%d) \n", path1, (int) arg_size);

  path_result = (char*)malloc(strlen(path1) + 6);
  path_without_ext = (char*)malloc(strlen(path1) -2);
  strncpy(path_without_ext, path1, strlen(path1)-4);
  path_without_ext[strlen(path1) - 4] = 0;

  sprintf(cmd, "jpegtran -flip horizontal -outfile %s-flip.jpg %s.jpg",
	  path_without_ext, path_without_ext);

  strcpy(path_result, path_without_ext);
  strcat(path_result, "-flip.jpg");
  

  printf("flip cmd = %s\n", cmd);
  printf("%d\n", system(cmd));

  if (diet_file_desc_set(diet_parameter(pb,1), path_result)) {
    printf("diet_file_desc_set error\n");
    free(path_without_ext);
    return 1;
  }

  usleep(t*500000);
  free(path_without_ext);
  
  return 0;

}

int
duplicate(diet_profile_t* pb)
{
  size_t arg_size  = 0;
  char* path1 = NULL;
  char* path_without_ext = NULL;
  char* path_result1 = NULL;
  char* path_result2 = NULL;
  char cmd[1024];

  fprintf(stderr, "DUPLICATE SOLVING\n");

  diet_file_get(diet_parameter(pb,0), NULL, &arg_size, &path1) ;
  fprintf(stderr, "on %s (%d) \n", path1, (int) arg_size);


  path_result1 = (char*)malloc(strlen(path1) + 6);
  path_result2 = (char*)malloc(strlen(path1) + 6);
  path_without_ext = (char*)malloc(strlen(path1) - 2);
  strncpy(path_without_ext, path1, strlen(path1) - 4);
  path_without_ext[strlen(path1) - 4] = 0;

  strcpy(path_result1, path_without_ext);
  strcat(path_result1, "-1.jpg");

  strcpy(path_result2, path_without_ext);
  strcat(path_result2, "-1.jpg");

  sprintf(cmd, "cp %s.jpg %s-1.jpg",
	  path_without_ext, path_without_ext);
  
  printf("duplicate %d\n", system(cmd));

  sprintf(cmd, "cp %s.jpg %s-2.jpg",
	  path_without_ext, path_without_ext);
  
  printf("duplicate %d\n", system(cmd));

  if (diet_file_desc_set(diet_parameter(pb,1), path_result1)) {
    printf("diet_file_desc_set error\n");
    free(path_without_ext);
    return 1;
  }
  if (diet_file_desc_set(diet_parameter(pb,2), path_result2)) {
    printf("diet_file_desc_set error\n");
    free(path_without_ext);
    return 1;
  }

  usleep(t*500000);
  free(path_without_ext);
  
  return 0;
}

int main(int argc, char * argv[]) {
  int res;
  diet_profile_desc_t* profile = NULL;

  if (argc == 3) {
    strcpy (time_str, argv[2]);
  }
  else {
    strcpy (time_str, "10");
  }

  diet_service_table_init(3);

  profile = diet_profile_desc_alloc("greyscale", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, greyscale)) return 1;
  diet_profile_desc_free(profile);

  profile = diet_profile_desc_alloc("flip", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, flip)) return 1;
  diet_profile_desc_free(profile);

  profile = diet_profile_desc_alloc("duplicate", 0, 0, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_FILE, DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_FILE, DIET_CHAR);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, duplicate)) return 1;
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
