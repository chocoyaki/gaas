/****************************************************************************/
/* DIET communication tools source code                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2003/04/10 12:45:25  pcombes
 * Remove duplicated functions.
 *
 * Revision 1.3  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.2  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include "com_tools.hh"
#include "dietTypes.hh"

#if HAVE_CICHLID

static char* COMMUNICATION_DIRECTORY = NULL;
static int NB_EMITTED_COM = 0;


long
profile_size(corba_pb_desc_t* p) 
{
  int i;
  long sum;

  sum = sizeof(corba_profile_t);
  for (i = 0; i < p->last_inout; i++) {
    sum += sizeof(corba_profile_t) + data_sizeof(&(p->param_desc[i]));
  }
  return sum;
}

void
init_communications() 
{
  char* com_dir = getenv("COMMUNICATION_DIRECTORY");

  if (com_dir == NULL) {
    fprintf(stderr,
	    "The COMMUNICATION_DIRECTORY environment variable is not defined !\n");
  }

  COMMUNICATION_DIRECTORY = com_dir;
}

void
add_communication(char* node0, char* node1, long size) 
{
  FILE* f;
  char file_name[1000];

  if (COMMUNICATION_DIRECTORY) {
    sprintf(file_name, 
	    "%s/com_%d_%d.data", 
	    COMMUNICATION_DIRECTORY, 
	    getpid(),
	    NB_EMITTED_COM);
    
    f = fopen(file_name, "w");
    if (f != NULL) {
      fprintf(f, "%s %s %ld", node0, node1, size);
      fclose(f);
    } else {
      fprintf(stderr, "Could not create file %s!\n", file_name);
    }
  
    NB_EMITTED_COM++;
  }
}


#endif // HAVE_CICHLID
