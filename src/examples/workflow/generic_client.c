/****************************************************************************/
/* Workflow example : Generic client for workflow submission                */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2009/05/15 10:58:47  bisnard
 * minor changes due to conditional nodes test
 *
 * Revision 1.7  2009/01/19 09:37:17  bisnard
 * new example for functional wf instanciation
 *
 * Revision 1.6  2009/01/16 16:34:05  bisnard
 * updated command line options to handle functional wf
 *
 * Revision 1.5  2008/12/09 09:01:06  bisnard
 * added new param to diet_wf_profile_alloc to select btw dag or functional wf
 *
 * Revision 1.4  2008/04/14 09:10:39  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.3  2007/05/30 11:16:36  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.2  2006/11/08 17:51:37  aamar
 * Use the get_all_results in the generic workflow client.
 *
 * Revision 1.1  2006/10/26 15:22:45  aamar
 * Replace generic-client.c by generic_client.c and scalar-server.c by
 * scalar_server.c
 *
 * Revision 1.3  2006/10/20 09:28:26  aamar
 * *** empty log message ***
 *
 ****************************************************************************/
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the worflow description file */

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> dag|dagNR|wf <wf_file> [data_file]\n", s);
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc != 4) && (argc != 5)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
  diet_wf_desc_t * profile;
  char *wfFileName, *dataFileName;
  wf_level_t wfType;
  struct timeval t1, t2;
  float time;
  short noRes = 0;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  wfFileName = argv[3];

  if (!strcmp(argv[2],"dag")) {
    wfType = DIET_WF_DAG;
  } else if (!strcmp(argv[2],"wf")) {
    wfType = DIET_WF_FUNCTIONAL;
  } else if (!strcmp(argv[2],"dagNR")) {
    wfType = DIET_WF_DAG;
    noRes = 1;
  } else {
    usage(argv[0]);
  }

  if (argc >= 5) {
    dataFileName = argv[4];
  } else {
    dataFileName = (char*) NULL;
  }

  gettimeofday(&t1, NULL);

  /*
   * Allocate a DIET workflow profile
   * The same method is used for both dags and functional workflows
   */
  profile = diet_wf_profile_alloc(wfFileName,"test", wfType);

  /*
   * For functional workflows ONLY
   * Defines which file is used to provide the data to instanciate the wf
   */
  diet_wf_profile_set_data_file(profile,dataFileName);

  printf("Try to execute the workflow\n");
  if (! diet_wf_call(profile)) {
    gettimeofday(&t2, NULL);
    time = (t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000;
    printf("The workflow submission succeed / time= %f s\n",time);
    if (!noRes) {
      printf("Display results:\n");
      if (!get_all_results(profile)) {
        printf("Could not display results\n");
      }
    }
  }
  else {
    printf("The workflow submission failed\n");
  }

//   diet_wf_free(profile);
  diet_finalize();
  fflush(stdout);
  return 0;
}
