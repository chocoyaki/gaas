#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_server.h"

static int solveFn(diet_profile_t *pb);
static estVector_t performanceFn(diet_profile_t* pb);

static int numResources = 0;
static char **resources = NULL;

int
main(int argc, char **argv)
{
  diet_profile_desc_t *profile = NULL;

  if (argc < 3) {
    fprintf(stderr,
            "usage: %s <SeD config> <resource> [<resource> ...]\n",
            argv[0]);
    exit (1);
  }

  if ((profile = diet_profile_desc_alloc("DBsearch", 0, 0, 0)) == NULL) {
    fprintf(stderr, "%s: unable to allocate DIET profile\n", argv[0]);
    exit (1);
  }

  numResources = argc-2;
  resources = &(argv[2]);

  if (diet_service_table_init(1) != 0) {
    fprintf(stderr, "%s: unable to initialize service table\n", argv[0]);
    exit (1);
  }

  {
    /* new section of the profile: aggregator */
    diet_aggregator_desc_t *agg;
    agg = diet_profile_desc_aggregator(profile);

    /* point the DIET server code to our custom performance function */
    diet_service_use_perfmetric(performanceFn);

    /* for this service, use a priority scheduler */
    diet_aggregator_set_type(agg, DIET_AGG_PRIORITY);
    diet_aggregator_priority_minuser(agg, 0);
    diet_aggregator_priority_max(agg, EST_TIMESINCELASTSOLVE);
  }

  diet_generic_desc_set(diet_param_desc(profile, 0),
                        DIET_PARAMSTRING,
                        DIET_CHAR);
  if (diet_service_table_add(profile, NULL, solveFn) != 0) {
    fprintf(stderr, "%s: unable to add DBsearch to service table\n", argv[0]);
    exit (1);
  }

  diet_profile_desc_free(profile);
  {
    int retval = diet_SeD(argv[1], argc, argv);
    if (retval != 0) {
      fprintf(stderr, "%s: diet_SeD failed (%d)\n", argv[0], retval);
      exit (1);
    }
  }

  exit (0);
}

static int
checkMismatch(const char *str)
{
  int strLen;
  int resIter;

  if (strchr(str, ':') == NULL) {
    strLen = strlen(str);
  }
  else {
    strLen = strchr(str, ':') - str;
  }

  for (resIter = 0 ; resIter < numResources ; resIter++) {
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
}

static int
computeMismatches(const char *scanString)
{
  int numMismatch = 0;
  do {
    numMismatch += checkMismatch(scanString);
    if (strchr(scanString, ':') == NULL) {
      scanString = NULL;
    }
    else {
      scanString = strchr(scanString, ':') + 1;
    }
  } while (scanString);

  return (numMismatch);
}

static int
solveFn(diet_profile_t *pb)
{
  const char *targetString;
  diet_persistence_mode_t strMode;
  int numMismatch;

  diet_paramstring_get(diet_parameter(pb, 0), &targetString, &strMode);
  numMismatch = computeMismatches(targetString);

  printf("%d mismatches on string %s\n", numMismatch, targetString);

  return (0);
}

static estVector_t
performanceFn(diet_profile_t* pb)
{
  const char *targetString;
  int numMismatch;
  estVector_t perfValues = diet_estimate_new_vector();

  /* string value must be fetched from description; value is NULL */
  targetString = (diet_paramstring_get_desc(diet_parameter(pb, 0)))->param;
  numMismatch = computeMismatches(targetString);

  /* store the mismatch value in the user estimate space */
  diet_set_user_estimate(perfValues, 0, numMismatch);

  /* also store the timestamp since last execution */
  diet_estimate_lastexec(perfValues, pb);

  return (perfValues);
}
