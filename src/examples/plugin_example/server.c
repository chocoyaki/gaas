/**
 * @file server.c
 *
 * @brief  plugin_example, server side
 *
 * @author  Alan SU (Alan.Su@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */
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
/****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_server.h"

/* forward declarations of private functions */
static int
solveFn(diet_profile_t *pb);
static void
performanceFn(diet_profile_t *pb, estVector_t perfValues);

/* server-level global variables */
static int numResources = 0;
static char **resources = NULL;

/*
** MAIN
*/
int
main(int argc, char **argv) {
  diet_profile_desc_t *profile = NULL;

  if (argc < 3) {
    fprintf(stderr,
            "usage: %s <SeD config> <resource> [<resource> ...]\n",
            argv[0]);
    exit(1);
  }

  if ((profile = diet_profile_desc_alloc("DBsearch", 0, 0, 0)) == NULL) {
    fprintf(stderr, "%s: unable to allocate DIET profile\n", argv[0]);
    exit(1);
  }

  /*
  ** the arguments from the second onward represent logical
  ** databases to which this server has access
  */
  numResources = argc - 2;
  resources = &(argv[2]);

  if (diet_service_table_init(1) != 0) {
    fprintf(stderr, "%s: unable to initialize service table\n", argv[0]);
    exit(1);
  }

  {
    /* new section of the profile: aggregator */
    diet_aggregator_desc_t *agg;
    agg = diet_profile_desc_aggregator(profile);

    /* install our custom performance function */
    diet_service_use_perfmetric(performanceFn);

    /* for this service, use a priority scheduler */
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_minuser(agg, 0);
    diet_aggregator_priority_max(agg, EST_TIMESINCELASTSOLVE);
  }

  /*
  ** the name of the requested database is passed as a
  ** DIET_PARAMSTRING, indicating that the contents of
  ** the string represent service parameters.  Thus, the
  ** contents need to be passed in the DIET profile, in
  ** order that the server take the value into account
  ** during the scheduling process.
  */
  diet_generic_desc_set(diet_param_desc(profile, 0),
                        DIET_PARAMSTRING,
                        DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solveFn) != 0) {
    fprintf(stderr, "%s: unable to add DBsearch to service table\n", argv[0]);
    exit(1);
  }

  diet_profile_desc_free(profile);
  {
    int retval = diet_SeD(argv[1], argc, argv);
    if (retval != 0) {
      fprintf(stderr, "%s: diet_SeD failed (%d)\n", argv[0], retval);
      exit(1);
    }
  }

  exit(0);
} /* main */

/*
** checkMismatch: determine if the server has access to the next
**   requested database in a colon-separated list
*/
static int
checkMismatch(const char *str) {
  int strLen;
  int resIter;

  if (strchr(str, ':') == NULL) {
    strLen = strlen(str);
  } else {
    strLen = strchr(str, ':') - str;
  }

  for (resIter = 0; resIter < numResources; resIter++) {
    if (strlen(resources[resIter]) > strLen) {
      continue;
    }
    if (strncmp(resources[resIter], str, strLen) != 0) {
      continue;
    }
    return (0);
  }

  printf(" mismatch: %.*s\n", strLen, str);
  return (1);
} /* checkMismatch */

/*
** computeMismatch: utility function to calculate the number
**   of client-requested databases that are *not* available
**   to this server (based on the list of databases that
**   are declared available to the server at executino-time).
*/
static int
computeMismatches(const char *scanString) {
  int numMismatch = 0;
  do {
    numMismatch += checkMismatch(scanString);
    if (strchr(scanString, ':') == NULL) {
      scanString = NULL;
    } else {
      scanString = strchr(scanString, ':') + 1;
    }
  } while (scanString);

  return (numMismatch);
} /* computeMismatches */

/*
** solveFn: the main DIET solve function
*/
static int
solveFn(diet_profile_t *pb) {
  const char *target;
  diet_persistence_mode_t strMode;
  int numMismatch;

  diet_paramstring_get(diet_parameter(pb, 0), &target, &strMode);
  numMismatch = computeMismatches(target);

  printf("%d mismatches on string %s\n", numMismatch, target);

  return (0);
} /* solveFn */

/*
** performanceFn: the performance function to use in the DIET
**   plugin scheduling facility
*/
static void
performanceFn(diet_profile_t *pb, estVector_t perfValues) {
  const char *target;
  int numMismatch;
  /* string value must be fetched from description; value is NULL */
  target = (diet_paramstring_get_desc(diet_parameter(pb, 0)))->param;
  numMismatch = computeMismatches(target);

  /*
  ** store the mismatch value in the user estimate space,
  ** using tag value 0
  */
  diet_est_set(perfValues, 0, numMismatch);

  /* also store the timestamp since last execution */
  diet_estimate_lastexec(perfValues, pb);
} /* performanceFn */
