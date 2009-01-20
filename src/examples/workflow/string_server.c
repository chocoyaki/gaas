/****************************************************************************/
/* DIET scheduling - This server include all services needed to execute     */
/* the workflow described in func_string.xml                  .             */
/*                                                                          */
/*  Author(s):                                                              */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/01/20 16:13:18  bisnard
 * new example to test functional workflows
 *
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
#include "DIET_Dagda.h"

#define CONTAINER_ELT_NB 2  /* must be 1 digit */

char time_str[64], name_str[64];
short nb_in, nb_out, nb_ports;
short *ports_depth_table;
long int t = 0;

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
    for (i=0; i<jobNb; i++) {
      /*  computation time for each job is added to EFT */
      tcomp = diet_est_get_system(jobVect[i].estVector, EST_TCOMP, 10000000);
      EFT += tcomp;
      /* if job is already running, substract the time since it started */
      if (jobVect[i].status == DIET_JOB_RUNNING) {
        gettimeofday(&currentTime, NULL);
        double already_done = (double)(currentTime.tv_sec*1000 + currentTime.tv_usec/1000) - jobVect[i].startTime;
        /* use minimum in case computation time is longer than expected */
        EFT -= (already_done > tcomp) ? tcomp : already_done;
      }
    }
    free(jobVect);
  }
  return EFT;
}

void
performance_Exec_Time(diet_profile_t* pb ,estVector_t perfValues )
{
  t = atoi(time_str);
  if ( t == 0 )
    t = 10;
  diet_estimate_comptime(perfValues, t*1000);
  diet_est_set(perfValues,0, eft_eval(pb, t*1000));
}

void
set_up_scheduler(diet_profile_desc_t* profile){
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_minuser(agg,0);
}

void check_data(const char* ID) {
  char *value;
  dagda_get_string(ID, &value);
  fprintf(stderr,"CHECK DATA %s : len=%d value=%s\n", ID, strlen(value), value);
}

/**
 * The processor service
 */
void
process_container(short depth, const char *outputstr, const char *parentID) {
  int i;
  char *eltStr[CONTAINER_ELT_NB], *eltID[CONTAINER_ELT_NB];
  char buffer[10];
  if (depth < 0) {
    fprintf(stderr, "Error in process_container: depth=%d\n", depth);
    exit(0);
  }
  for (i=0; i<CONTAINER_ELT_NB; i++) {
    eltStr[i] = (char*) calloc(strlen(outputstr)+2, sizeof(char));
    strcpy(eltStr[i], outputstr);
    sprintf(buffer,"_%d",i);
    strncat(eltStr[i],buffer,2);
    if (depth == 1) {
      fprintf(stderr, "(%d) storing element %d (value=%s)\n", depth, i, eltStr[i]);
      dagda_put_string(eltStr[i], DIET_PERSISTENT, &eltID[i]);
      fprintf(stderr, "(%d) adding element %d : ID=%s\n", depth, i, eltID[i]);
      check_data(eltID[i]);
      if (dagda_add_container_element(parentID,eltID[i],i)) {
        fprintf(stderr, "ERROR : cannot add element to container\n");
        break;
      }
      fprintf(stderr, "(%d) element %d completed\n", depth,i);
    } else {
      dagda_create_container(&eltID[i]);
      fprintf(stderr, "(%d) adding element %d : (CONTAINER) ID=%s\n", depth, i, eltID[i]);
      if (dagda_add_container_element(parentID,eltID[i],i)) {
        fprintf(stderr, "ERROR : cannot add element to container\n");
        break;
      }
      process_container(depth-1, eltStr[i], eltID[i]);
    }
    /* free(eltID); */
  }
}

int
processor(diet_profile_t* pb)
{
  int i, inlength;
  const char *inputstr;
  char * outputstr, *Noutputstr, *idx, *outContID;

  fprintf(stderr, "PROCESSOR %s SOLVING (REQUEST %d)\n", name_str, pb->dietReqID);

  /* process length of output string */
  inlength = 0;
  for (i=0; i<nb_in; i++) {
    diet_string_get(diet_parameter(pb, i), &inputstr, NULL);
    inlength += strlen(inputstr);
  }
  inlength += (nb_in - 1) + strlen(name_str) + 7;
  outputstr = (char*) calloc(inlength, sizeof(char));

  /* build output string */
  strcat(outputstr, name_str);
  strcat(outputstr, "_out");
  if (nb_out > 1)
    strcat(outputstr, "$");
  strcat(outputstr, "(");
  for (i=0; i<nb_in; i++) {
    check_data((*diet_parameter(pb, i)).desc.id);
    diet_string_get(diet_parameter(pb, i), &inputstr, NULL);
    fprintf(stderr, "INPUT %d (len=%d): %s\n", i, strlen(inputstr), inputstr);
    strcat(outputstr, inputstr);
    if (i < nb_in-1)
      strcat(outputstr,",");
  }
  strcat(outputstr,")");

  /* store output in profile */
  if (nb_out == 1) {
    if (ports_depth_table[nb_in] == 0) {
      fprintf(stderr, "OUTPUT = %s\n", outputstr);
      diet_string_set(diet_parameter(pb,nb_in), outputstr, DIET_PERSISTENT_RETURN);
    } else {
      outContID = (pb->parameters[nb_in]).desc.id;
      fprintf(stderr, "OUTPUT (CONTAINER) ID = %s\n", outContID);
      dagda_init_container(diet_parameter(pb,nb_in));
      process_container(ports_depth_table[nb_in], outputstr, outContID);
    }
  } else {
    for (i=0; i<nb_out; i++) {
      Noutputstr = (char*) calloc(inlength, sizeof(char));
      /* TODO remplacer $ par l'index du port */
      strcpy(Noutputstr, outputstr);
      if (ports_depth_table[nb_in+i] == 0) {
        diet_string_set(diet_parameter(pb,nb_in+i), Noutputstr, DIET_PERSISTENT_RETURN);
      } else {
        outContID = (pb->parameters[nb_in+i]).desc.id;
        fprintf(stderr, "OUTPUT %d (CONTAINER) ID = %s\n", i, outContID);
        dagda_init_container(diet_parameter(pb,nb_in+i));
        process_container(ports_depth_table[nb_in+i], Noutputstr, outContID);
      }
    }
    free(outputstr);
  }

  usleep(t*100000);

  return 0;
}

/* argv[1]: server config file path
   argv[2]: name of the service
   argv[3]: time of computation (in milliseconds)
   argv[4]: nb of IN ports
   argv[5]: nb of OUT ports
   argv[6+n]: depth of n-th port (default is 0) (>=1 ONLY AVAILABLE FOR 1st OUT PORT)
 */

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> <name> <comptime> <nb_in> <nb_out> [depth_p0] ... [depth_pN]\n", s);
  exit(1);
}

int checkUsage(int argc, char ** argv) {
  if (argc < 6) {
    usage(argv[0]);
  }
  return 0;
}

int main(int argc, char * argv[]) {

  int res,i;
  diet_profile_desc_t* profile = NULL;

  /* service name */
  strcpy (name_str, argv[2]);

  /* computation time */
  strcpy (time_str, argv[3]);

  /* nb of ports */
  nb_in = atoi(argv[4]);
  nb_out = atoi(argv[5]);
  nb_ports = nb_in + nb_out;

  /* ports depth */
  ports_depth_table = (short*) malloc(nb_ports * sizeof(short));
  for (i=0; i<nb_ports; i++) {
    if (i <= (argc-7)) {
      ports_depth_table[i] = atoi(argv[i+6]);
    } else {
      ports_depth_table[i] = 0;
    }
  }

  diet_service_table_init(1);

  /* Add the service */
  profile = diet_profile_desc_alloc(name_str, (nb_in - 1), (nb_in - 1), (nb_ports - 1));
  for (i=0; i<nb_ports; i++) {
    if (ports_depth_table[i] == 0) {
      diet_generic_desc_set(diet_param_desc(profile,i), DIET_STRING, DIET_CHAR);
    } else {
      diet_generic_desc_set(diet_param_desc(profile,i), DIET_CONTAINER, DIET_CHAR);
    }
  }
  set_up_scheduler(profile);
  if (diet_service_table_add(profile, NULL, processor)) return 1;
  diet_profile_desc_free(profile);

  diet_print_service_table();
  res = diet_SeD(argv[1], argc, argv);
  return 0;
}
