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
 * Revision 1.13  2010/07/20 09:08:47  bisnard
 * Added name parameter to wf client (used by gui)
 *
 * Revision 1.12  2009/09/25 12:54:02  bisnard
 * fixed bug for dag submission
 * make command-line parameters standard
 *
 * Revision 1.11  2009/08/26 10:35:40  bisnard
 * use new workflow API
 *
 * Revision 1.10  2009/07/30 09:37:23  bisnard
 * added diet_wf_free after workflow completion
 *
 * Revision 1.9  2009/07/23 12:26:06  bisnard
 * new API method to get functional wf results as a container
 *
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
#include <sys/time.h>
#include <libgen.h> /* basename() */
#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the worflow description file */

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> -dag <dag_file>\n", s);
  fprintf(stderr, "       %s <file.cfg> -wf <wf_file> <data_file> [transcript_file] [-name WFNAME]\n", s);
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc < 4) || (argc > 8)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
  diet_wf_desc_t * profile;
  char *wfFileName, *dataFileName, *transcriptFileName, *wfName, *dagFileName;
  int curPos = 0;
  wf_level_t wfType;
  char wfTypeName[10];
  struct timeval t1, t2;
  float time;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  wfFileName = (char*) NULL;
  dataFileName = (char*) NULL;
  transcriptFileName = (char*) NULL;
  wfName = (char*) NULL;
  dagFileName = (char*) NULL;

  if (!strcmp(argv[2],"-dag")) {
    strcpy(wfTypeName, "dag");
    wfType = DIET_WF_DAG;
    if (argc > 4) usage(argv[0]);
  } else if (!strcmp(argv[2],"-wf")) {
    strcpy(wfTypeName, "workflow");
    wfType = DIET_WF_FUNCTIONAL;
    if (argc < 5) usage(argv[0]);
  } else {
    usage(argv[0]);
  }

  if (wfType == DIET_WF_FUNCTIONAL) {
    wfFileName = argv[3];
    dataFileName = argv[4];
    curPos = 5;
    if ((argc > curPos) && (strcmp(argv[curPos], "-name"))) {
	transcriptFileName = argv[curPos];
	curPos++;
    }
    if ((argc > curPos) && (!strcmp(argv[curPos], "-name"))) {
      curPos++;
      if (argc > curPos)
	wfName = argv[curPos];
      else
	wfName = "";
    } else {
      wfName = basename(wfFileName);
    }
  } else {	/* DIET_WF_DAG */
    dagFileName = argv[3];
  }

  gettimeofday(&t1, NULL);


  if (wfType == DIET_WF_FUNCTIONAL) {
    
    /*
     * Allocate the workflow profile
     */
    profile = diet_wf_profile_alloc(wfFileName, wfName, wfType);
    /*
    * For functional workflows ONLY
    * Defines which file is used to provide the data to instanciate the wf
    */
    diet_wf_set_data_file(profile,dataFileName);

    /*
    * For workflow restart
    * Defines which file is used to store the execution transcriptFileName
    * (file will be overwritten if existing)
    */
    diet_wf_set_transcript_file(profile, transcriptFileName);
  } else {	/* DIET_WF_DAG */
    /*
     * Allocate the dag profile
     */
    profile = diet_wf_profile_alloc(dagFileName,"test", wfType);
  }

  printf("Try to execute the %s\n", wfTypeName);
  if (! diet_wf_call(profile)) {
    gettimeofday(&t2, NULL);
    time = (t2.tv_sec - t1.tv_sec) + ((float)(t2.tv_usec - t1.tv_usec))/1000000;
    printf("The %s submission succeed / time= %f s\n" ,wfTypeName, time);

    if (wfType == DIET_WF_FUNCTIONAL) {
      printf("Save data in data_out.xml\n");
      if (diet_wf_save_data_file(profile, "data_out.xml")) {
        printf("Could not save data file\n");
      }
    }

    printf("Display results:\n");
    if (diet_wf_print_results(profile) != 0) {
      printf("Could not display results\n");
    }

  }
  else {
    printf("The %s submission failed\n", wfTypeName);
  }

  if (transcriptFileName == NULL) {
    diet_wf_free(profile);
  } else {
    printf("Save transcript of workflow execution\n");
    diet_wf_save_transcript_file(profile, transcriptFileName);
  }
  diet_finalize();
  fflush(stdout);
  return 0;
}
