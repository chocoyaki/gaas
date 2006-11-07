/****************************************************************************/
/* DIET scheduling - This server include all services needed to execute     */
/* the workflow described in scalar.xml                  .                  */
/*                                                                          */
/*  Author(s):                                                              */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/11/07 11:57:10  rbolze
 * Remove references to CORI.
 *
 * Revision 1.1  2006/10/26 15:22:45  aamar
 * Replace generic-client.c by generic_client.c and scalar-server.c by
 * scalar_server.c
 *
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
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY); 
  diet_aggregator_priority_minuser(agg,0);
}

/**
 * The SUCC service
 */
int
succ(diet_profile_t* pb)
{
  int * ix = NULL;
  int * jx = NULL;
  int * kx = NULL;

  fprintf(stderr, "SUCC SOLVING\n");
  
  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,1), &jx, NULL);
  diet_scalar_get(diet_parameter(pb,2), &kx, NULL);
  fprintf(stderr, "i = %d\n", *(int*)ix);
  *(int*)jx = *(int*)ix + 1;
  *(int*)kx = *(int*)ix + 1;
  fprintf(stderr, "succ(i) = %d\n", *(int*)jx);
  diet_scalar_desc_set(diet_parameter(pb,1), jx);
  diet_scalar_desc_set(diet_parameter(pb,2), kx);

  usleep(t*500000);
  
  return 0;
}

/**
 * The DOUBLE service
 */
int
double_int(diet_profile_t* pb)
{
  int * ix = NULL;
  int * jx = NULL;

  fprintf(stderr, "DOUBLE SOLVING\n");

  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,0), &jx, NULL);
  fprintf(stderr, "i = %d\n", *(int*)ix);
  *(int*)jx = *(int*)ix * 2;
  fprintf(stderr, "j = 2i = %d\n", *(int*)jx);
  diet_scalar_desc_set(diet_parameter(pb,1), jx);

  usleep(t*500000);

  return 0;
}


/**
 * The SUM service
 */ 
int
sum(diet_profile_t* pb)
{
  int * ix = NULL;
  int * jx = NULL;
  int * kx = NULL;

  fprintf(stderr, "SUM SOLVING\n");
  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,1), &jx, NULL);
  diet_scalar_get(diet_parameter(pb,2), &kx, NULL);
  fprintf(stderr, "i = %d, j =%d\n", *(int*)ix, *(int*)jx);
  *(int*)kx = *(int*)ix + *(int*)jx;
  fprintf(stderr, "k = i + j = %d\n", *(int*)kx);
  diet_scalar_desc_set(diet_parameter(pb,2), kx);

  usleep(t*500000);

  return 0;
}

/**
 * The SQUARE service
 */
int
square(diet_profile_t* pb)
{
  int * ix = NULL;
  double * jx = NULL;

  fprintf(stderr, "SQUARE SOLVING\n");
  
  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,1), &jx, NULL);
  fprintf(stderr, "i = %d\n", *(int*)ix);
  *(double*)jx = sqrt(*(int*)ix);
  fprintf(stderr, "sqrt(i) = %f\n", *(double*)jx);
  diet_scalar_desc_set(diet_parameter(pb,1), jx);

  usleep(t*500000);
  
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

  diet_service_table_init(4);

  /* Add the SUCC service */
  profile = diet_profile_desc_alloc("succ", 0, 0, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, succ)) return 1;
  diet_profile_desc_free(profile);

  /* Add the DOUBLE service */
  profile = diet_profile_desc_alloc("double", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, double_int)) return 1;
  diet_profile_desc_free(profile);

  /* Add the SUM service */
  profile = diet_profile_desc_alloc("sum", 1, 1, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_INT);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, sum)) return 1;
  diet_profile_desc_free(profile);

  /* Add the SQUARE service */
  profile = diet_profile_desc_alloc("square", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_DOUBLE);
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, square)) return 1;
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
