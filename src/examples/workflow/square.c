/****************************************************************************/
/* DIET scheduling - Square of an integer                .                  */
/*                                                                          */
/*  Author(s):                                                              */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2008/05/05 13:54:18  bisnard
 * new computation time estimation get/set functions
 *
 * Revision 1.3  2006/11/07 12:44:48  aamar
 * *** empty log message ***
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
  diet_estimate_comptime(perfValues, t);
}

void
set_up_scheduler(diet_profile_desc_t* profile){
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_minuser(agg,0);
}


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

  usleep(t*100000);

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
