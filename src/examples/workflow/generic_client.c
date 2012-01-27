/**
 * @file generic_client.c
 *
 * @brief  Workflow example : Generic client for workflow submission
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */

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

void
usage(char *s) {
  fprintf(stderr, "Usage: %s <file.cfg> -dag <dag_file>\n", s);
  fprintf(
    stderr,
    "       %s <file.cfg> -wf <wf_file> <data_file> [transcript_file] [-name WFNAME]\n",
    s);
  exit(1);
}
int
checkUsage(int argc, char **argv) {
  if ((argc < 4) || (argc > 8)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char *argv[]) {
  diet_wf_desc_t *profile;
  char *wfFileName, *dataFileName, *transcriptFileName, *wfName, *dagFileName;
  wf_level_t wfType;
  char wfTypeName[10];
  struct timeval t1, t2;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  wfFileName = (char *) NULL;
  dataFileName = (char *) NULL;
  transcriptFileName = (char *) NULL;
  wfName = (char *) NULL;
  dagFileName = (char *) NULL;

  if (!strcmp(argv[2], "-dag")) {
    strcpy(wfTypeName, "dag");
    wfType = DIET_WF_DAG;
    if (argc > 4) {
      usage(argv[0]);
    }
  } else if (!strcmp(argv[2], "-wf")) {
    strcpy(wfTypeName, "workflow");
    wfType = DIET_WF_FUNCTIONAL;
    if (argc < 5) {
      usage(argv[0]);
    }
  } else {
    usage(argv[0]);
  }

  if (wfType == DIET_WF_FUNCTIONAL) {
    int curPos = 5;
    wfFileName = argv[3];
    dataFileName = argv[4];
    if ((argc > curPos) && (strcmp(argv[curPos], "-name"))) {
      transcriptFileName = argv[curPos];
      curPos++;
    }
    if ((argc > curPos) && (!strcmp(argv[curPos], "-name"))) {
      curPos++;
      if (argc > curPos) {
        wfName = argv[curPos];
      } else {
        wfName = "";
      }
    } else {
      wfName = basename(wfFileName);
    }
  } else {      /* DIET_WF_DAG */
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
    diet_wf_set_data_file(profile, dataFileName);

    /*
     * For workflow restart
     * Defines which file is used to store the execution transcriptFileName
     * (file will be overwritten if existing)
     */
    diet_wf_set_transcript_file(profile, transcriptFileName);
  } else {      /* DIET_WF_DAG */
    /*
     * Allocate the dag profile
     */
    profile = diet_wf_profile_alloc(dagFileName, "test", wfType);
  }

  printf("Try to execute the %s\n", wfTypeName);
  if (!diet_wf_call(profile)) {
    float time;
    gettimeofday(&t2, NULL);
    time =
      (t2.tv_sec - t1.tv_sec) + ((float) (t2.tv_usec - t1.tv_usec)) / 1000000;
    printf("The %s submission succeed / time= %f s\n", wfTypeName, time);

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
  } else {
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
} /* main */
