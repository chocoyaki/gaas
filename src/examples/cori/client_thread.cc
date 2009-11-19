/****************************************************************************/
/* DIET scalars example: a client for additions of all types of scalars.    */
/* Uses threads to submit requests                                          */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Benjamin Depardon (Benjamin.Depardon@ens-lyon.fr)                   */
/*    - Peter Frauenkron (Peter.Frauenkron@gmail.com)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.2  2009/11/19 07:27:41  ycaniou
 * Remove warnings
 *
 * Revision 1.1  2009/10/16 08:09:14  bdepardo
 * Threaded version of the client.
 *
 *
 ****************************************************************************/

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>


#include "DIET_client.h"

/* argv[1]: client config file path
   argv[2]: one of the strings above 
   argv[3]: number of fibonacci      */

void
usage(char* cmd)
{
  fprintf(stderr, "Usage: %s <file.cfg> <fibonacci number> <nb threads>\n",
	  cmd);
  exit(1);
}

static unsigned int nb = 0;

void *
call(void* par) {
  diet_profile_t* profile = NULL;
  /* Use the long type for all "integer" types. */
  long * pl2 = NULL;
  char * path = strdup("FIBO");
  long l1 = *(long*) par;
  unsigned int id = nb;
  nb ++;

  for (;;) {
    profile = diet_profile_alloc(path, 0, 0, 1);
        
    printf("Before the call (%u): fibo=%ld\n", id, (long)l1);
    diet_scalar_set(diet_parameter(profile,0), &l1,  DIET_VOLATILE, DIET_LONGINT);
    diet_scalar_set(diet_parameter(profile,1), NULL, DIET_VOLATILE, DIET_LONGINT);
    
    if (!diet_call(profile)) {
	diet_scalar_get(diet_parameter(profile,1), &pl2, NULL);
	
	printf("After the call (%u): fibo=%ld\n", id, (long)*pl2);
      } else {
	printf("Pb !!!\n");
      }
    
    diet_free_data(diet_parameter(profile,1));
    diet_profile_free(profile);
  }
}

int
main(int argc, char* argv[])
{
  long     l1 = 0;
  unsigned int i;
  unsigned int nbThreads;

  if (argc != 4) {
    usage(argv[0]);
  }
 
  if (diet_initialize(argv[1], argc, argv)) {
    fprintf(stderr, "DIET initialization failed !\n");
    return 1;
  }
  
  l1 = atol(argv[2]);
  if (l1>46) {
    printf("too big number! %ld\n", (long)l1);
    exit(1);
  }

  nbThreads = atoi(argv[3]);

  pthread_t * thread = (pthread_t*) malloc(nbThreads * sizeof(pthread_t));
  for (i = 0; i < nbThreads; ++ i) {
    pthread_create(&thread[i], NULL, call, (void *) &l1);
//     omni_thread::create(call, (void *) &l1, omni_thread::PRIORITY_NORMAL);
  }

  for (i = 0; i < nbThreads; ++ i)
    pthread_join(thread[i], NULL);

  diet_finalize();

  return 0;
}
