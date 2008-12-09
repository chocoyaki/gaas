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

#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: path of the worflow description file */

void usage(char * s) {
  fprintf(stderr, "Usage: %s <file.cfg> <wf_file>\n", s);
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc != 3) && (argc != 4)) {
    usage(argv[0]);
  }
  return 0;
}

int
main(int argc, char* argv[])
{
  diet_wf_desc_t * profile;
  char * fileName;

  checkUsage(argc, argv);

  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  fileName = argv[2];

  profile = diet_wf_profile_alloc(fileName,"test",DIET_WF_DAG);

  printf("Try to execute the workflow\n");
  if (! diet_wf_call(profile)) {
    printf("The workflow submission succeed\n");
  }
  else {
    printf("The workflow submission failed\n");
  }

  get_all_results(profile);

  diet_wf_free(profile);

  return 0;
}
