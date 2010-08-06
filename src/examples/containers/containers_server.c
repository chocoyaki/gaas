#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#include "DIET_server.h"
#include "DIET_Dagda.h"

double ratio=1;

/* begin function prototypes*/
int service(diet_profile_t *pb);
int add_service(char* service_name);
void set_up_scheduler(diet_profile_desc_t* profile);
void performance_eval(diet_profile_t* pb,estVector_t perfValues);
/* end function prototypes*/


/*
 * usage function:
 * tell how to launch the SeD
 */
int
usage(char* cmd)
{
	printf("Usage : %s <file.cfg> <ratio> <service_name>\n", cmd);
	printf("\texample: %s SeD.cfg 1 s1 s2\n", cmd);
	return -1;
}
/*
 * add_service function :
 * declare the DIET's service
 */
int
add_service(char* service_name)
{
  diet_profile_desc_t* profile = NULL;
  printf("Service_name : %s\n",service_name);
  profile = diet_profile_desc_alloc(service_name,1,1,2);
  /* setup scheduler */
  set_up_scheduler(profile);
  /* Set profile parameters: */
  diet_generic_desc_set(diet_param_desc(profile,0),DIET_SCALAR,DIET_LONGINT);
  diet_generic_desc_set(diet_param_desc(profile,1),DIET_CONTAINER,DIET_CHAR);
  diet_generic_desc_set(diet_param_desc(profile,2),DIET_CONTAINER,DIET_CHAR);
  /* Add service to the service table */
  if (diet_service_table_add(profile, NULL, service )) return 1;
  /* Free the profile, since it was deep copied */
  diet_profile_desc_free(profile);
}
/*
 * set_up_scheduler :
 * define the plugins scheduler
 */
void
set_up_scheduler(diet_profile_desc_t* profile){
  diet_aggregator_desc_t *agg;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_eval);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_min(agg,EST_USERDEFINED);
}

/*
 * comptime_eval :
 * calculates the computation time estimation for a given profile
 * the return value is in millisecond;
 */
double comptime_eval(diet_profile_t* pb) {
  const long* sleepTime;
  long outsleepTime;
  double comp_time=0;
  sleepTime = (diet_scalar_get_desc(diet_parameter(pb, 0)))->value;
  outsleepTime=*sleepTime;
  comp_time=ratio*(double)outsleepTime;
  printf("@@ sleepTime        = %ld \n",*sleepTime);
  printf("@@ ratio        = %f \n",ratio);
  printf("@@ comtime_eval = %f \n",comp_time);
  return comp_time;
}

/*
 * EFT_eval :
 * calculate the earliest finish time using the SeDs jobqueue & this job's estimations
 * the return value is in milliseconds and is a relative time (interval until finish)
 */
double eft_eval(diet_profile_t* pb, double computationTimeEstim) {
  double         EFT, tcomp;
  jobVector_t    jobVect = NULL;
  int            jobNb,i ;
  struct timeval currentTime;

  EFT = computationTimeEstim; /* init with current job's computation time */
  /* add the computation time for all other jobs on the SeD */
  if (!diet_estimate_list_jobs(&jobVect, &jobNb, pb)) {
    /************** EFT computation VALID FOR MAXCONCJOBS=1 ONLY !! *********/
    printf("%d active job(s) in the SeD queue\n", jobNb);
    for (i=0; i<jobNb; i++) {
      /*  computation time for each job is added to EFT */
      tcomp = diet_est_get_system(jobVect[i].estVector, EST_TCOMP, 10000000);
      printf("\033[1;32m tcomp =%f \033[0m \n",tcomp);
      EFT += tcomp;
      /* if job is already running, substract the time since it started */
      if (jobVect[i].status == DIET_JOB_RUNNING) {
        gettimeofday(&currentTime, NULL);
        /* use minimum in case computation time is longer than expected */
        double already_done = (double)(currentTime.tv_sec*1000 + currentTime.tv_usec/1000) - jobVect[i].startTime;
        EFT -= (already_done > tcomp) ? tcomp : already_done;
        printf("\033[0;33m jobVect[%d] already_done=%f \033[0m \n",i,already_done);
      }
    }

    free(jobVect);
  }
  return EFT;
}

/*
 *  performance_eval :
 *  define the function which set the perf value.
 */
void
performance_eval(diet_profile_t* pb, estVector_t perfValues)
{
  double         perf_val, EFT;

  // set the value for COMPTIME
  // perf_val in millisecond;
  perf_val = comptime_eval(pb);
  diet_estimate_comptime(perfValues,perf_val);
  printf("TCOMP=%f\n",perf_val);

  // compute EFT and set the value as USERDEFINED
  EFT = eft_eval(pb,perf_val);
  diet_est_set(perfValues,0, EFT);
  printf("EFT=%f\n",EFT);
}

/*
 * SOLVE FUNCTION
 */

int
service(diet_profile_t* pb)
{
	int res = 0;
        char * ID1;
        char * ID2;
        char * ID3;
        char * ID4;
        char * path1 = NULL;
	long* outsleepTime = (long*) malloc(sizeof(long));
	printf("###############\n");
        diet_container_t content1, content2;
       /* no need to call dagda_get_container for root container as it is
        * downloaded automatically by DIET */
        printf("Get PARENT container element list\n");
        dagda_get_container_elements((*diet_parameter(pb,1)).desc.id, &content1);
        if (content1.size != 1) {
          printf("Container does not contain expected nb of elements\n");
          printf("It contains %ld elements\n",content1.size);
        } else {
          printf("Retrieve CHILD container (not downloaded by DIET)\n");
          dagda_get_container(content1.elt_ids[0]);
          printf("Get CHILD container element list\n");
          dagda_get_container_elements(content1.elt_ids[0], &content2);
          if (content2.size !=2) {
            printf("Container does not contain expected nb of elements\n");
            printf("It contains %ld elements\n",content2.size);
          } else {
            printf("Get elements\n");
            long *sleepTime1 = NULL;
            dagda_get_scalar(content2.elt_ids[0],&sleepTime1, NULL);
            dagda_get_file(content2.elt_ids[1],&path1);
            printf("Container contains: %ld, %s\n", *sleepTime1, path1);
            *outsleepTime = *sleepTime1*ratio;
          }
        }
	printf("Time to Sleep =%ld ms\n",*outsleepTime);
	usleep(*outsleepTime*1000);
        printf("INIT PARENT OUTPUT container\n");
        dagda_init_container(diet_parameter(pb,2));
        printf("CREATE CHILD OUTPUT container\n");
        dagda_create_container(&ID4);
        printf("PUT CHILD container as 1st element of OUTPUT container\n");
        dagda_add_container_element((*diet_parameter(pb,2)).desc.id, ID4, 0);
        printf("PUT first OUT element on platform (SCALAR)\n");
        dagda_put_scalar(outsleepTime, DIET_LONGINT, DIET_PERSISTENT, &ID1);
        printf("PUT second OUT element on platform (FILE)\n");
        dagda_put_file(path1, DIET_PERSISTENT, &ID2);
        printf("Add CHILD container elements\n");
        dagda_add_container_element(ID4, ID1, 2);
        dagda_add_container_element(ID4, ID2, 3);
        printf("###############\n");
        free(content1.elt_ids);
        free(content2.elt_ids);
	return res;
}

/*
 * MAIN
 */
int
main( int argc, char* argv[])
{
	int res,i;
	 if (argc <4) {
	    return usage(argv[0]);
	 }
	diet_service_table_init(argc-3);
	ratio=atof(argv[2]);
	 for(i =0;i< argc-3;i++){
		 char* service_name = NULL;
		 service_name = argv[3+i];
		 add_service(service_name);
	 }
	diet_print_service_table();
	res = diet_SeD(argv[1],argc,argv);
	return res;
}

