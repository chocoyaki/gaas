/****************************************************************************/
/* DIET scheduling - CoRI example: a server for fibonacci.                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Peter Frauenkron (Peter.Frauenkron@gmail.com)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2009/08/06 14:00:53  bdepardo
 * Linear version of Fibonacci
 *
 * Revision 1.3  2009/08/04 12:11:35  bdepardo
 * Corrected data management
 *
 * Revision 1.2  2006/07/06 23:09:32  eboix
 *    Cori examples are now build (when required). --- Injay2461
 *
 * Revision 1.1  2006/01/25 21:39:06  pfrauenk
 * CoRI Example dmatmips replaced by a simple fibonacci example
 *
 ****************************************************************************/

#include "DIET_server.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* This server offers the fibonacci function:                  */

#define NB_SRV 1
static const char* SRV[NB_SRV] =
  {"FIBO"};


long (*fibonacci_algo)(long);

void
set_up_scheduler(char * schedulertype,diet_profile_desc_t* profile);

/*
 * performanceLOAD_AVG: the performance function to use in the DIET
 *   plugin scheduling facility
 * A simple call to the CORI_EASY_collector to receive the EST_AVGFREECPU:
 * the load average of the SeD
 * scheduling criteria is the loadavg: the SeD with the lowest 
 * load average receive the job
 */
void
performance_Load_Avg(diet_profile_t* pb,estVector_t perfValues);

void
performance_RRNBPROC(diet_profile_t* pb,estVector_t perfValues);

void
performance_FAST_freecpu(diet_profile_t* pb,estVector_t perfValues);

void 
add_new_value(double valToAdd, int size);

/* Recursive definition of function fibonacci */
long fibonacci( long n )
{
  if ( n == 0 || n == 1 )
          return n;
     else 
          return fibonacci( n - 1 ) + fibonacci( n - 2 );
}

long fibonacci_linear( long n )
{
  long i;
  long * map = (long*) malloc(sizeof(long)*(n+1));
  map[0] = 0;
  map[1] = 1;
  for (i = 2; i < n+1; ++i)
    map[i] = map[i-1] + map[i-2];

  i = map[n];
  free(map);
  return i;
}


/*
 * SOLVE FUNCTION
 */

int
solve_fib(diet_profile_t* pb)
{
  int res = 0;
  long * tmp = NULL;
  
  /* For integers, we can use the longest type to store values
     of smaller types. */
  long* l1 = NULL;
  diet_scalar_get(diet_parameter(pb,0), &l1, NULL);
  diet_scalar_get(diet_parameter(pb,1), &tmp, NULL);
  printf("Solve fibonacci %ld...\n", *l1);

/*   *tmp = fibonacci(*l1); */
  *tmp = (*fibonacci_algo)(*l1);

  diet_scalar_desc_set(diet_parameter(pb,1), tmp);
  
  diet_free_data(diet_parameter(pb,0));

  printf("Solve fibonacci ... done\n");
  return res;
}

int
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> <LOADAVG|DEFAULT> [linear|exponential]\n", cmd);
  return 1;
}

/*
 * MAIN
 */

int
main(int argc, char* argv[])
{
  int res;
  char * scheduler_name;

  diet_profile_desc_t* profile = NULL;

  if (argc < 3) {
    return usage(argv[0]);
  }
  scheduler_name=argv[2];
  diet_service_table_init(NB_SRV);


  fibonacci_algo = &fibonacci;
  if (argc == 4) {
    if (!strcmp("linear",argv[3])) {
      fibonacci_algo = &fibonacci_linear;
      printf("## Algorithm: linear\n");
    } else
      printf("## Algorithm: recursive\n");
  } else
      printf("## Algorithm: recursive\n");


  profile = diet_profile_desc_alloc(SRV[0], 0, 0, 1);
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_SCALAR, DIET_LONGINT);
  diet_generic_desc_set(diet_param_desc(profile,1),DIET_SCALAR, DIET_LONGINT);

  set_up_scheduler(scheduler_name,profile );

  if (diet_service_table_add(profile, NULL, solve_fib))
      return 1;
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  /* Not reached */
  return res;
}
/*******************SCHEDULING***********************/
static
struct liste_t{
  int actual_size;
  int maxsize;
  double *array;
}list_lastexec;

void 
set_up_scheduler(char * schedulertype, diet_profile_desc_t* profile)
{
  diet_aggregator_desc_t *agg;

  if (strcmp(schedulertype, "DEFAULT")==0){  
    /* nothing to do */
    return;
  }
  agg = diet_profile_desc_aggregator(profile);
  
  if (strcmp(schedulertype, "LOADAVG")==0)
  {  
    /* initialize your collector: cori_easy */
    diet_estimate_cori_add_collector(EST_COLL_EASY,NULL);
    diet_service_use_perfmetric(performance_Load_Avg);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY); 
    diet_aggregator_priority_min(agg,EST_AVGFREECPU);
  } else
  if (strcmp(schedulertype,"FASTCPU")==0)
  {
    /* initialize your collector: cori_fast */
    diet_estimate_cori_add_collector(EST_COLL_FAST,NULL);
    diet_service_use_perfmetric(performance_FAST_freecpu);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_max(agg,EST_FREECPU);
  } else
  if (strcmp(schedulertype,"RRNBPROC")==0)
  {
    /* initialize your collector: cori_easy */
    diet_estimate_cori_add_collector(EST_COLL_EASY,NULL);
    list_lastexec.actual_size = 0;
    list_lastexec.array       = NULL;
    diet_service_use_perfmetric(performance_RRNBPROC);
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_maxuser(agg,0);
  }
  else{
    usage("programName");
  }
}

void
performance_Load_Avg(diet_profile_t* pb,estVector_t perfValues )
{
  int minute = 15;
  diet_estimate_cori(perfValues,EST_AVGFREECPU, EST_COLL_EASY,&minute);
}
 
void
performance_FAST_freecpu(diet_profile_t* pb,estVector_t perfValues){
  diet_estimate_cori(perfValues,EST_FREECPU, EST_COLL_FAST,pb);
}

void
performance_RRNBPROC(diet_profile_t* pb,estVector_t perfValues)
{
  int nbcpu;

  diet_estimate_cori(perfValues,EST_NBCPU, EST_COLL_EASY,NULL);
  nbcpu=(int)diet_est_get_system(perfValues,EST_NBCPU,1);

  /* store the timestamp since last execution */
  diet_estimate_lastexec(perfValues, pb);

  /* stock the value in list_lastexec (the most recent is on index 0) */
  add_new_value(diet_est_get_system(perfValues,
                EST_TIMESINCELASTSOLVE,
                HUGE_VAL),
                nbcpu);
 
  /* if the stocked number of executions is less then the number of CPUs, then
   * stock in the perfValues on tag userdefine only the number of cpu * 10000
   * this is to prevent that an low number-CPU-SeD receive too fast a new task
   */
  if (list_lastexec.actual_size<nbcpu)
{
    diet_est_set(perfValues,0,nbcpu*100000);
  }
  else{
    /* stock the last element of vector_values in the scalar of perfValues */
  diet_est_set(perfValues,0,list_lastexec.array[list_lastexec.actual_size-1]);
  }
}

void 
add_new_value(double valToAdd, int size){
  if (list_lastexec.actual_size==0){
    list_lastexec.actual_size = 1;
    list_lastexec.maxsize     = size;
    list_lastexec.array       = (double *)calloc(size,sizeof(double));
    list_lastexec.array[0]    = valToAdd;
  }
  else
    if (list_lastexec.maxsize == list_lastexec.actual_size)
    { /* the maximal size is reached */
      int i;
      for (i=list_lastexec.maxsize-1;i<0;i--)
      list_lastexec.array[i] = list_lastexec.array[i-1];
    }
    else
    {
      int i;
      list_lastexec.actual_size++;
      for (i=list_lastexec.actual_size;i<0;i--)
         list_lastexec.array[i] = list_lastexec.array[i-1];
    }
      list_lastexec.array[0] = valToAdd;   
}
