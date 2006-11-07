/****************************************************************************/
/* DIET scheduling - SeD using CORI                      .                  */
/*                                                                          */
/*  Author(s):                                                              */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2006/11/07 12:44:48  aamar
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

#include "DIET_server.h"

char time_str[64];
long int t = 0;

void
performance_Exec_Time(diet_profile_t* pb ,estVector_t perfValues )
{
  t = atoi(time_str);
  if ( t == 0 )
    t = 10;
  double d;
  diet_scalar_get(diet_parameter(pb, 0), &d, NULL);
  printf("The Magic number is %lf\n", d);
  printf("The second number is %lf\n", 
	 *((double*)(pb->parameters[0].desc.specific.scal.value))
	 );

  diet_est_set(perfValues, 0, t);
}

void 
set_up_scheduler(diet_profile_desc_t* profile){ 
  diet_aggregator_desc_t *agg;
  agg = diet_profile_desc_aggregator(profile);
  diet_estimate_cori_add_collector(EST_COLL_EASY,NULL);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY); 
  diet_aggregator_priority_minuser(agg,0);
}


int
succ_float(diet_profile_t* pb)
{
  fprintf(stderr, "SUCC SOLVING\n");
  
  double * ix = NULL;
  double * jx = NULL;
  double * kx = NULL;
  
  diet_scalar_get(diet_parameter(pb,0), &ix, NULL);
  diet_scalar_get(diet_parameter(pb,1), &jx, NULL);
  diet_scalar_get(diet_parameter(pb,2), &kx, NULL);
  fprintf(stderr, "i = %lg\n", *(double*)ix);
  *(double*)jx = *(double*)ix + 1;
  *(double*)kx = *(double*)ix + 1;
  fprintf(stderr, "succ(i) = %lg\n", *(double*)jx);
  diet_scalar_desc_set(diet_parameter(pb,1), jx);
  diet_scalar_desc_set(diet_parameter(pb,2), kx);

  usleep(1000000);
  
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
  profile = diet_profile_desc_alloc("succ_float", 0, 0, 2);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_SCALAR, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_SCALAR, DIET_DOUBLE);

  set_up_scheduler(profile);


  if (diet_service_table_add(profile, NULL, succ_float)) return 1;

  diet_profile_desc_free(profile);
  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
