/****************************************************************************/
/* plugin_example, server side                                              */
/*                                                                          */
/*   This example simulates a database search application, in which         */
/*   servers have access to a fixed set of databases, and clients           */
/*   submit queries against these databases.  The servers in this           */
/*   example make use of DIET's plugin scheduler facility to perform        */
/*   client request scheduling based on optimization of the                 */
/*   availability of the requested database(s) at the targeted              */
/*   server nodes.                                                          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Alan SU (Alan.Su@ens-lyon.fr)                                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2005/06/13 11:35:36  alsu
 * a few more comments to make the example easier to understand
 *
 ****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_client.h"

/* forward declarations of private functions */
static char *constructTargetString(int num, char **strings);

/*
** MAIN
*/
int
main(int argc, char **argv)
{
  int retval;
  diet_profile_t *profile = NULL;
  char *targetString;

  if (argc < 3) {
    fprintf(stderr,
            "usage: %s <client config> <resource> [<resource> ...]\n",
            argv[0]);
    exit (1);
  }

  if ((retval = diet_initialize(argv[1], argc, argv)) != 0) {
    fprintf(stderr, "%s: DIET initialization failed (%d)\n", argv[0], retval);
    exit (1);
  }

  if ((profile = diet_profile_alloc("DBsearch", 0, 0, 0)) == NULL) {
    fprintf(stderr, "%s: unable to allocate DIET profile\n", argv[0]);
    exit (1);
  }

  /*
  ** create a colon-separated string representing the databases
  ** that the user has requested be queried
  */
  targetString = constructTargetString(argc - 2, &(argv[2]));
  if (diet_paramstring_set(diet_parameter(profile, 0),
                           targetString,
                           DIET_VOLATILE) != 0) {
    fprintf(stderr, "%s: unable to set target string\n", argv[0]);
    exit (1);
  }

  if ((retval = diet_call(profile)) != 0) {
    fprintf(stderr, "%s: DIET call failed (%d)\n", argv[0], retval);
    exit (1);
  }

  free(targetString);
  diet_profile_free(profile);
  diet_finalize();

  exit (0);
}

/*
** constructTargetString: create colon-separated string from an
**   argv-style series of elements
*/
static char *
constructTargetString(int num, char **strings)
{
  char *s;
  int stringLengthCount = 0;
  int strIter;

  assert(num > 0);
  assert(strings != NULL);

  for (strIter = 0; strIter < num; strIter++) {
    stringLengthCount += strlen(strings[strIter]);
  }
  s = calloc(num + stringLengthCount, sizeof (char));
  assert(s != NULL);

  for (strIter = 0; strIter < num-1; strIter++) {
    sprintf(s+strlen(s), "%s:", strings[strIter]);
  }
  sprintf(s+strlen(s), "%s", strings[num-1]);

  return (s);
}
