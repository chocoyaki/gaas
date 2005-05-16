#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_client.h"

static char *constructTargetString(int num, char **strings);

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

static char *
constructTargetString(int num, char **strings)
{
  char *s;
  int stringLengthCount = 0;
  int strIter;

  assert(num > 0);
  assert(strings != NULL);

  for (strIter = 0 ; strIter < num ; strIter++) {
    stringLengthCount += strlen(strings[strIter]);
  }
  s = calloc(num + stringLengthCount, sizeof (char));
  assert(s != NULL);

  for (strIter = 0 ; strIter < num-1 ; strIter++) {
    sprintf(s+strlen(s), "%s:", strings[strIter]);
  }
  sprintf(s+strlen(s), "%s", strings[num-1]);

  return (s);
}
