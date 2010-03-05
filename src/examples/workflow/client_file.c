/****************************************************************************/
/* Workflow example : Client for file workflow submission                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.8  2010/03/05 15:52:09  ycaniou
 * Ordered things in CMakelist.txt and update Display (Batch, build_version...)
 * Fix version guess of compiler (was gcc only)
 * Use option to avoid %zd warning
 * Undo previous cast of size_t into int
 *
 * Revision 1.7  2008/12/09 09:01:06  bisnard
 * added new param to diet_wf_profile_alloc to select btw dag or functional wf
 *
 * Revision 1.6  2008/07/07 14:52:30  bisnard
 * bug fix
 *
 * Revision 1.5  2008/07/07 09:50:48  bisnard
 * Added EFT calculation required by MaDag scheduler
 *
 * Revision 1.4  2008/04/14 09:10:39  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.3  2007/05/30 11:16:36  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.2  2006/11/28 15:09:33  aamar
 * Correct headers
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
  char * path1 = NULL, * path2 = NULL;
  char command1[255], command2[255];
  size_t out_size1 = 0 , out_size2 = 0;

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
    diet_wf_file_get(profile, "n3#out1", &out_size1, &path1);
    if (path1 && (*path1 != '\0')) {
      printf("Location of returned file is %s, its size is %zd.\n",
	     path1, out_size1);
      /* diet_free_data(diet_parameter(profile,4)); */
    }
    diet_wf_file_get(profile, "n3#out2", &out_size2, &path2);
    if (path2 && (*path2 != '\0')) {
      printf("Location of returned file is %s, its size is %zd.\n",
	     path2, out_size2);
      /* diet_free_data(diet_parameter(profile,4)); */
    }

  }
  else {
    printf("The workflow submission failed\n");
  }

  diet_wf_free(profile);

  return 0;
}
