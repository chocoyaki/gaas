/****************************************************************************/
/* Workflow example : a server that convert an integer to a string          */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $@Id$
 * $@Log$
 ****************************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

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
int2str(diet_profile_t* pb)
{
  char * str = NULL;
  int  * len = NULL;

  fprintf(stderr, "STRLEN SOLVING\n");

  diet_scalar_get(diet_parameter(pb,0), &len, NULL);
  diet_string_get(diet_parameter(pb,1), &str, NULL);
  sprintf(str, "%d", *len);
  fprintf(stderr, "itoa(%d) = %s\n", *len, str);

  diet_string_set(diet_parameter(pb,1), str, DIET_PERSISTENT);

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

  diet_service_table_init(1);
  profile = diet_profile_desc_alloc("int2str", 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_INT);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_STRING, DIET_CHAR);

  set_up_scheduler(profile );

  if (diet_service_table_add(profile, NULL, int2str)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
