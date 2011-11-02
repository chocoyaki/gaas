/**
* @file client.c
* 
* @brief  DIET scalars example: a client for additions of all types of scalars 
* 
* @author  - Peter Frauenkron (Peter.Frauenkron@gmail.com) 
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.3  2009/08/06 14:00:52  bdepardo
 * Linear version of Fibonacci
 *
 * Revision 1.2  2009/08/04 12:11:35  bdepardo
 * Corrected data management
 *
 * Revision 1.1  2006/01/25 21:39:06  pfrauenk
 * CoRI Example dmatmips replaced by a simple fibonacci example
 *
 ****************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "DIET_client.h"

#define NB_PB 1
static const char* PB[NB_PB] =
  {"FIBO"};


/* argv[1]: client config file path
   argv[2]: one of the strings above 
   argv[3]: number of fibonacci      */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> [%s] <fibonacci number> \n",
          cmd, PB[0]);
  exit(1);
}

int
main(int argc, char* argv[])
{
  char * path;
  diet_profile_t* profile = NULL;
  /* Use the long type for all "integer" types. */
  long     l1 = 0;
  long*   pl2 = NULL;
  if (argc != 4) {
    usage(argv[0]);
  }
  path = argv[2];
 
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }

  profile = diet_profile_alloc(path, 0, 0, 1);
  
  l1 = atol(argv[3]);
  if (l1>46) {
    printf("too big number! %ld\n", (long)l1);
    exit(1);
  }

  printf("Before the call: fibo=%ld\n", (long)l1);
  diet_scalar_set(diet_parameter(profile, 0), &l1,  DIET_VOLATILE, DIET_LONGINT);
  diet_scalar_set(diet_parameter(profile, 1), NULL, DIET_VOLATILE, DIET_LONGINT);
 
  if (!diet_call(profile)) {
   
    diet_scalar_get(diet_parameter(profile, 1), &pl2, NULL);
     
    printf("After the call: fibo=%ld\n",(long)*pl2);

  } else {
    fprintf(stderr, "diet_call has returned with an error code !!!\n");
    return 1;
  }
  
  diet_free_data(diet_parameter(profile, 1));
  diet_profile_free(profile);
  diet_finalize();

  return 0;
}
