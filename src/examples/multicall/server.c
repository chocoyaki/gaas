#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "DIET_server.h"


static void metric(diet_profile_t * profile, estVector_t values) 
{
  int i;
  int *NS;

  NS = diet_scalar_get_desc(diet_parameter(profile, 0))->value;
  //fullfil the estim vector with dummy values.
  for (i = 0; i < *NS; i++) {
    diet_est_set(values, i, *NS * i);
  }
}

/*
 * SOLVE FUNCTION
 */

//the SeD sleeps NS seconds depending on how many simulations it is given
int
solve_cerfacs(diet_profile_t* pb)
{
  int res = 0;
  int* NR;
  int* NS;
  char* target;
  
  diet_scalar_get(diet_parameter(pb, 0), &NS, NULL);
  diet_scalar_get(diet_parameter(pb, 1), &NR, NULL);
  diet_paramstring_get(diet_parameter(pb, 2), &target, NULL);
  
  fprintf(stdout, "NS = %d\n", *NS);
  fprintf(stdout, "NR = %d\n", *NR);
  fprintf(stdout, "simulations = %s\n", target);
  
  sleep(*NS);
  
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg>\n", cmd);
  return 1;
}


/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  int res;

  diet_profile_desc_t* profile = NULL;

  if (argc < 2) {
    return usage(argv[0]);
  }
  diet_service_table_init(1);

  diet_aggregator_desc_t *agg;
  
  profile = diet_profile_desc_alloc("CERFACS", 2, 2, 2);
  //NS
  diet_generic_desc_set(diet_param_desc(profile, 0), DIET_SCALAR, DIET_INT);
  //NR
  diet_generic_desc_set(diet_param_desc(profile, 1), DIET_SCALAR, DIET_INT);
  //string of names separated by '#'
  diet_generic_desc_set(diet_param_desc(profile, 2), DIET_PARAMSTRING, DIET_CHAR);
  
  agg = diet_profile_desc_aggregator(profile);
  
  diet_service_use_perfmetric(metric);
  
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_minuser(agg, 0);
  
  if (diet_service_table_add(profile, NULL, solve_cerfacs))
    return 1;
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
