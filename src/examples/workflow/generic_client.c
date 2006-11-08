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
  fprintf(stderr, "Usage: %s <file.cfg> <wf_file> [option]\n", s);
  fprintf(stderr, "option = -madag_sched | -notmadag_sched\n");
  exit(1);
}
int checkUsage(int argc, char ** argv) {
  if ((argc != 3) && (argc != 4)) {
    usage(argv[0]);
  }
  if (argc == 4) {
    if (strcmp(argv[3], "-madag_sched") && 
	strcmp(argv[3], "-notmadag_sched")) {
      usage(argv[0]);
    }
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

  if (argc == 4)
    set_madag_sched(!strcmp(argv[3], "-madag_sched"));

  profile = diet_wf_profile_alloc(fileName);

  /*  set_heft_sched(); */

  enable_reordering("generic-client", 0);
  /*  set_reordering_delta(10, 1); */
  
  printf("Try to execute the workflow\n");
  if (! diet_wf_call(profile)) {
    printf("The workflow submission succeed\n");
  }
  else {
    printf("The workflow submission failed\n");
  }

  get_all_results();
  
  diet_wf_free(profile);

  return 0;
}
