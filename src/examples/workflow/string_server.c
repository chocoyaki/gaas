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
 * Revision 1.8  2010/09/06 07:41:13  bdepardo
 * Fixed warnings
 *
 * Revision 1.7  2009/07/10 12:02:39  bisnard
 * fixed string allocation bug
 *
 * Revision 1.6  2009/06/23 09:26:56  bisnard
 * new API method for EFT estimation
 *
 * Revision 1.5  2009/05/28 14:07:25  bisnard
 * removed constant value for nb of elements of input
 *
 * Revision 1.4  2009/05/15 10:58:47  bisnard
 * minor changes due to conditional nodes test
 *
 * Revision 1.3  2009/01/22 14:19:42  bisnard
 * new example for dag using containers
 *
 * Revision 1.2  2009/01/22 10:16:04  bisnard
 * new example for container usage in workflows
 *
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
char *parLeft_c = "[";
char *parRight_c = "]";
char *separator_c = ",";

void
performance_Exec_Time(diet_profile_t* pb ,estVector_t perfValues )
{
  double eft, tcomp;
  t = atoi(time_str);
  if ( t == 0 )
    t = 10;
  /* Set the job duration and compute SeD's EFT (results stored in EV) */
  diet_estimate_comptime(perfValues, t*1000);
  diet_estimate_eft(perfValues, t*1000, pb);
  /* Get the values from EV */
  tcomp = diet_est_get_system(perfValues, EST_TCOMP, 10000000);
  eft = diet_est_get_system(perfValues, EST_EFT, 10000000);
  printf("TCOMP=%f / EFT=%f\n", tcomp, eft);
}

void
set_up_scheduler(diet_profile_desc_t* profile){
  diet_aggregator_desc_t *agg = NULL;
  agg = diet_profile_desc_aggregator(profile);
  diet_service_use_perfmetric(performance_Exec_Time);
  diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
  diet_aggregator_priority_min(agg, EST_EFT);
}

void check_data(const char* ID) {
  char *value;
  dagda_get_string(ID, &value);
  fprintf(stderr,"CHECK DATA %s : len=%d value=%s\n", ID, (int)strlen(value), value);
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
    eltStr[i] = (char*) calloc(strlen(outputstr)+3, sizeof(char));
    strcpy(eltStr[i], outputstr);
    sprintf(buffer,"_%d",i);
    strncat(eltStr[i],buffer,2);
    if (depth == 1) {
      fprintf(stderr, "(%d) storing element %d (value=%s)\n", depth, i, eltStr[i]);
      dagda_put_string(eltStr[i], DIET_PERSISTENT, &eltID[i]);
      fprintf(stderr, "(%d) adding element %d : ID=%s\n", depth, i, eltID[i]);
      /* check_data(eltID[i]); */
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

int container_string_length(const char* contID, short depth, char getContainer) {
  unsigned int i;
  short length = 0;
  diet_container_t content;
  char *eltStr;
  if (depth < 0) {
    fprintf(stderr, "Error in container_string_length: depth=%d\n", depth);
    exit(0);
  }
  /* container download not necessary for root input container but required
     for sub-containers */
  if (getContainer) {
    fprintf(stderr, "(%d) getting container ID = %s\n", depth, contID);
    if (dagda_get_container(contID)) {
      fprintf(stderr, "ERROR: could not get container ID = %s\n", contID);
      return 0;
    }
  }

  fprintf(stderr, "(%d) getting element ids (container ID = %s)\n", depth, contID);
  if (!dagda_get_container_elements(contID, &content)) {
    for (i=0; i<content.size; i++) {
      if (content.elt_ids[i] == NULL)
          length += 6;
      else if (depth == 1) {
        if (!dagda_get_string(content.elt_ids[i], &eltStr))
          length += strlen(eltStr);
        else
          fprintf(stderr, "ERROR: cannot get container element %s\n", content.elt_ids[i]);
      } else {
        length += container_string_length(content.elt_ids[i], depth-1, 1);
      }
      if (i < content.size-1)
        length += 1;
    }
    length += 2;
  } else fprintf(stderr, "ERROR: cannot get container (%s) element IDs\n", contID);
  return length;
}

void container_string_get(const char* contID, short depth, char *contStr) {
  unsigned int i;
  diet_container_t content;
  char *eltStr;
  if (depth < 0) {
    fprintf(stderr, "Error in container_string_get: depth=%d\n", depth);
    exit(0);
  }
  if (!dagda_get_container_elements(contID, &content)) {
    strcat(contStr,parLeft_c);
    for (i=0; i<content.size; i++) {
      if (content.elt_ids[i] == NULL) {
        strcat(contStr, "[VOID]");
      } else if (depth == 1) {
        if (!dagda_get_string(content.elt_ids[i], &eltStr))
          strcat(contStr, eltStr);
        else
          fprintf(stderr, "ERROR: cannot get container element %s\n", content.elt_ids[i]);
      } else {
        container_string_get(content.elt_ids[i], depth-1, contStr);
      }
      if (i < content.size-1)
        strcat(contStr,separator_c);
    }
    strcat(contStr,parRight_c);
  } else fprintf(stderr, "ERROR: cannot get container (%s) element IDs\n", contID);
}

int
processor(diet_profile_t* pb)
{
  int i, inlength;
  const char *inputstr;
  char *outputstr, *Noutputstr, *outContID;
  char buffer[10];

  fprintf(stderr, "PROCESSOR %s SOLVING (REQUEST %d)\n", name_str, pb->dietReqID);

  /* process length of output string */
  inlength = 0;
  for (i=0; i<nb_in; i++) {
    if (ports_depth_table[i] == 0) {
      diet_string_get(diet_parameter(pb, i), &inputstr, NULL);
      inlength += strlen(inputstr);
    } else {
      inlength += container_string_length(pb->parameters[i].desc.id, ports_depth_table[i], 0);
    }
  }
  inlength += (nb_in - 1) + strlen(name_str) + 7;
  outputstr = (char*) calloc(inlength+1, sizeof(char));

  /* build output string */
  strcat(outputstr, name_str);
  strcat(outputstr, "_out");
  if (nb_out > 1)
    strcat(outputstr, "$");
  strcat(outputstr, parLeft_c);
  for (i=0; i<nb_in; i++) {
    if (ports_depth_table[i] == 0) {
      /* check_data((*diet_parameter(pb, i)).desc.id); */
      diet_string_get(diet_parameter(pb, i), &inputstr, NULL);
      fprintf(stderr, "INPUT %d (len=%d): %s (%p)\n", i, (int)strlen(inputstr), inputstr, inputstr);
      strcat(outputstr, inputstr);
    } else {
      container_string_get(pb->parameters[i].desc.id, ports_depth_table[i], outputstr);
    }
    if (i < nb_in-1)
      strcat(outputstr,separator_c);
  }
  strcat(outputstr,parRight_c);
  fprintf(stderr, "INPUT STRING = %s (len=%d)\n", outputstr, (int)strlen(outputstr));

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
      Noutputstr = (char*) calloc(inlength+1, sizeof(char));
      /* remplacer $ par l'index du port */
      sprintf(buffer,"%d",i);
      strcpy(Noutputstr, outputstr);
      Noutputstr[strstr(Noutputstr, "$") - Noutputstr] = buffer[0];

      if (ports_depth_table[nb_in+i] == 0) {
        fprintf(stderr, "OUTPUT %d = %s\n", i, Noutputstr);
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
   argv[5]: nb of OUT ports (must be <10)
   argv[6+n]: depth of n-th port (default is 0)
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
