/****************************************************************************/
/* DIET scalars example: a client for additions of all types of scalars.    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2003/09/26 14:07:33  pcombes
 * Add an example devoted to the management of scalars (and their encoding length).
 *
 ****************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"


#define print_condition 1


#define NB_PB 6
static const char* PB[NB_PB] =
  {"CADD", "BADD", "IADD", "LADD", "FADD", "DADD"};


/* argv[1]: client config file path
   argv[2]: one of the strings above */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> [%s|%s|%s|%s|%s|%s]\n",
	  cmd, PB[0], PB[1], PB[2], PB[3], PB[4], PB[5]);
  exit(1);
}

int
main(int argc, char* argv[])
{
  int i;
  /* Use the long type for all "integer" types, and double for floating types,
     so that no overflow can occur. */
  long     l1 = 0;
  long     l2 = 0;
  long*   pl3 = NULL;
  double   d1 = 0.0;
  double   d2 = 0.0;
  double* pd3 = NULL;
  char*  path = NULL;
  diet_profile_t* profile = NULL;

  int   pb[NB_PB] = {0, 0, 0, 0, 0, 0};

  if (argc != 3) {
    usage(argv[0]);
  }
  path = argv[2];

  
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  for (i = 0; i < NB_PB; i++) {
    if ((pb[i] = !strcmp(path, PB[i]))) break;
  }

  profile = diet_profile_alloc(path, 0, 1, 2);

  if (pb[0]) {
    (char)l1 = 0x1;
    (char)l2 = 0x2;
    printf("Before the call: l1=0x%hhX, l2=0x%hhX\n", (char)l1, (char)l2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_CHAR);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_CHAR);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_CHAR);
  } else if (pb[1]) {
    (short)l1 = 0x11;
    (short)l2 = 0x22;
    printf("Before the call: l1=0x%hX, l2=0x%hX\n", (short)l1, (short)l2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_BYTE);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_BYTE);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_BYTE);
  } else if (pb[2]) {
    (int)l1 = 0x1111;
    (int)l2 = 0x2222;
    printf("Before the call: l1=0x%X, l2=0x%X\n", (int)l1, (int)l2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_INT);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_INT);
  } else if (pb[3]) {
    l1 = 0x11111111;
    l2 = 0x22222222;
    printf("Before the call: l1=0x%lX, l2=0x%lX\n", (long)l1, (long)l2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_LONGINT);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_LONGINT);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_LONGINT);
  } else if (pb[4]) {
    (float)d1 = HUGE_VAL;
    (float)d2 = HUGE_VAL;
    printf("Before the call: d1=%f, d2=%f\n", (float)d1, (float)d2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_FLOAT);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_FLOAT);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_FLOAT);
  } else if (pb[5]) {
    (double)d1 = HUGE_VAL;
    (double)d2 = HUGE_VAL;
    printf("Before the call: d1=%lf, d2=%lf\n", (double)d1, (double)d2);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_DOUBLE);
    diet_scalar_set(diet_parameter(profile,1), &l2,  DIET_VOLATILE, DIET_DOUBLE);
    diet_scalar_set(diet_parameter(profile,2), NULL, DIET_VOLATILE, DIET_DOUBLE);    
  } else {
    usage(argv[0]);
  } 
    
  if (!diet_call(profile)) {
    if (pb[4] || pb[5]) {
      diet_scalar_get(diet_parameter(profile,2), &pd3, NULL);
      if (pb[4]) {
	printf("After the call: d1=%f, d2=%f, d3=%f\n",
	       (float)d1, (float)d2, (float)*pd3);
      } else if (pb[5]) {
	printf("After the call: d1=%lf, d2=%lf, d3=%lf\n",
	       (double)d1, (double)d2, (double)*pd3);
      }
    } else {
      diet_scalar_get(diet_parameter(profile,2), &pl3, NULL);

      /* We decide to print all results as longs, which lets the user see how
	 the types are managed. */
#define PRETTY_PRINT 0
#if PRETTY_PRINT
      if (pb[0]) {
	printf("After the call: l1=0x%hhX, l2=0x%hhX, l3=0x%hhX\n",
	       (char)l1, (char)l2, (char)(*pl3));
      } else if (pb[1]) {
	printf("After the call: l1=0x%hX, l2=0x%hX, l3=0x%hX\n",
	       (short)l1, (short)l2, (short)*pl3);
      } else if (pb[2]) {
	printf("After the call: l1=0x%X, l2=0x%X, l3=0x%X\n",
	       (int)l1, (int)l2, (int)*pl3);
      } else if (pb[3]) {
#endif // PRETTY_PRINT
	printf("After the call: l1=0x%lX, l2=0x%lX, l3=0x%lX\n",
	       (long)l1, (long)l2, (long)*pl3);
#if PRETTY_PRINT
      }
#endif // PRETTY_PRINT
    }
  }
  
  diet_profile_free(profile);
  diet_finalize();

  return 0;
}
