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
 * Revision 1.5  2003/07/04 09:48:06  pcombes
 * Use new ERROR and WARNING macros.
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
  COMMUNICATION_DIRECTORY = getenv("COMMUNICATION_DIRECTORY");
  if (COMMUNICATION_DIRECTORY == NULL) {
    ERROR(" COMMUNICATION_DIRECTORY is not set", );
  }
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
      WARNING(" could not create file " << file_name);
    }
  
    NB_EMITTED_COM++;
  }
}


#endif // HAVE_CICHLID
