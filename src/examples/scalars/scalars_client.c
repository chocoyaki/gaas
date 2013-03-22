/**
 * @file scalars_client.c
 *
 * @brief  DIET scalars example: a client for additions of all types of scalars.
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef __WIN32__
#include <unistd.h>
#endif
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"


#define print_condition 1


#define NB_PB 6
static const char *PB[NB_PB] =
{"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char *cmd) {
  fprintf(stderr, "Usage: %s <file.cfg> [%s|%s|%s|%s|%s|%s]\n",
          cmd, PB[0], PB[1], PB[2], PB[3], PB[4], PB[5]);
  exit(1);
}

int
main(int argc, char *argv[]) {
  int i;
  /* Use the long type for all "integer" types. */
  long l1 = 0;
  long l2 = 0;
  long *pl3 = NULL;
  /* It is not possible to apply this rule for floating types ... */
  float f1 = 0.0;
  float f2 = 0.0;
  float *pf3 = NULL;
  double d1 = 0.0;
  double d2 = 0.0;
  double *pd3 = NULL;
  char *path = NULL;
  diet_profile_t *profile = NULL;

  int pb[NB_PB] = {0, 0, 0, 0, 0, 0};

  if (argc != 3) {
    usage(argv[0]);
  }
  path = argv[2];


  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) {
      break;
    }
  }

  profile = diet_profile_alloc(path, 0, 1, 2);

  if (pb[0]) {
    l1 = (char) 0x1;
    l2 = (char) 0x2;
    printf("Before the call: l1 = 0x%hhX, l2 = 0x%hhX\n", (char) l1, (char) l2);
    diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_CHAR);
    diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_CHAR);
    diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_CHAR);
  } else if (pb[1]) {
    l1 = (short) 0x11;
    l2 = (short) 0x22;
    printf("Before the call: l1 = 0x%hX, l2 = 0x%hX\n", (short) l1, (short) l2);
    diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_SHORT);
    diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_SHORT);
    diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_SHORT);
  } else if (pb[2]) {
    l1 = (int) 0x1111;
    l2 = (int) 0x2222;
    printf("Before the call: l1 = 0x%X, l2 = 0x%X\n", (int) l1, (int) l2);
    diet_scalar_set(diet_parameter(profile, 0), &l1, DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 1), &l2, DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_INT);
  } else if (pb[3]) {
    l1 = 0x11111111;
    l2 = 0x22222222;
    printf("Before the call: l1 = 0x%lX, l2 = 0x%lX\n", (long) l1, (long) l2);
    diet_scalar_set(diet_parameter(profile,
                                   0), &l1, DIET_VOLATILE, DIET_LONGINT);
    diet_scalar_set(diet_parameter(profile,
                                   1), &l2, DIET_VOLATILE, DIET_LONGINT);
    diet_scalar_set(diet_parameter(profile,
                                   2), NULL, DIET_VOLATILE, DIET_LONGINT);
  } else if (pb[4]) {
    f1 = 1.1e38;
    f2 = 2.2e38;
    printf("Before the call: f1=%f, f2=%f\n", f1, f2);
    diet_scalar_set(diet_parameter(profile, 0), &f1, DIET_VOLATILE, DIET_FLOAT);
    diet_scalar_set(diet_parameter(profile, 1), &f2, DIET_VOLATILE, DIET_FLOAT);
    diet_scalar_set(diet_parameter(profile, 2), NULL, DIET_VOLATILE, DIET_FLOAT);
  } else if (pb[5]) {
    d1 = 1.1e307;
    d2 = 2.2e307;
    printf("Before the call: d1=%lg, d2=%lg\n", d1, d2);
    diet_scalar_set(diet_parameter(profile, 0), &d1, DIET_VOLATILE, DIET_DOUBLE);
    diet_scalar_set(diet_parameter(profile, 1), &d2, DIET_VOLATILE, DIET_DOUBLE);
    diet_scalar_set(diet_parameter(profile,
                                   2), NULL, DIET_VOLATILE, DIET_DOUBLE);
  } else {
    usage(argv[0]);
  }

#define PRETTY_PRINT 0

  if (!diet_call(profile)) {
    if (pb[0] || pb[1] || pb[2] || pb[3]) {
      diet_scalar_get(diet_parameter(profile, 2), &pl3, NULL);
#if PRETTY_PRINT
      if (pb[0]) {
        printf("After the call: l1 = 0x%hhX, l2 = 0x%hhX, l3 = 0x%hhX\n",
               (char) l1, (char) l2, (char) (*pl3));
      } else if (pb[1]) {
        printf("After the call: l1 = 0x%hX, l2 = 0x%hX, l3 = 0x%hX\n",
               (short) l1, (short) l2, (short) *pl3);
      } else if (pb[2]) {
        printf("After the call: l1 = 0x%X, l2 = 0x%X, l3 = 0x%X\n",
               (int) l1, (int) l2, (int) *pl3);
      } else if (pb[3])
#endif /* PRETTY_PRINT */
       /* When PRETTY_PRINT is 0, we print all results as longs, which lets the
          user see how the types are managed. */
      printf("After the call: l1 = 0x%lX, l2 = 0x%lX, l3 = 0x%lX\n",
             (long) l1, (long) l2, (long) *pl3);
    } else if (pb[4]) {
      diet_scalar_get(diet_parameter(profile, 2), &pf3, NULL);
      printf("After the call: f1=%g, f2=%g, f3=%g\n",
             (float) f1, (float) f2, (float) *pf3);
    } else if (pb[5]) {
      diet_scalar_get(diet_parameter(profile, 2), &pd3, NULL);
      printf("After the call: d1=%lg, d2=%lg, d3=%lg\n",
             (double) d1, (double) d2, (double) *pd3);
    }
  } else {
    fprintf(stderr, "diet_call has returned with an error code !!!\n");
    return 1;
  }

  diet_profile_free(profile);
  diet_finalize();

  return 0;
} /* main */
