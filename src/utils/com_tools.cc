/****************************************************************************/
/* $Id$ */
/* DIET communication tools source code                                     */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Ludovic BERTSCH (Ludovic.Bertsch@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
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

#if HAVE_CICHLID

static char* COMMUNICATION_DIRECTORY = NULL;
static int NB_EMITTED_COM = 0;

int
base_type_size(diet_base_type_t base_type)
{
  switch (base_type) {
  case DIET_CHAR:
    return sizeof(char);
  case DIET_BYTE:                 // I presume this is a 'byte'
    return sizeof(unsigned char);
  case DIET_INT:
    return sizeof(int);
  case DIET_LONGINT:
    return sizeof(long);
  case DIET_FLOAT:
    return sizeof(float);
  case DIET_DOUBLE:
    return sizeof(double);
    /*  case DIET_SCOMPLEX:
    return sizeof();
  case DIET_DCOMPLEX:
  return sizeof();*/
  default:
    fprintf(stderr, "Warning!! base_type_size(): base_type is unknown\n");
    return 0;
  }
}

long
parameter_size(diet_arg_t* p) 
{
  diet_data_desc_t* d;
  long size;
  int size_of_base_type;

  d = &p->desc;
  size = sizeof(diet_data_desc_t);
  size_of_base_type = base_type_size(d->generic.base_type);

  switch (d->generic.type) {
  case DIET_SCALAR:
    size += size_of_base_type;
    break;
  case DIET_VECTOR:
    size += d->specific.vect.size*size_of_base_type;
    break;
  case DIET_MATRIX:
    size += d->specific.mat.nb_r*d->specific.mat.nb_c*size_of_base_type;
    break;
  case DIET_STRING:
    size += d->specific.str.length + 1;
    break;
  case DIET_FILE:
    size += d->specific.file.size;
    break;
  default:
    size = 0;
    fprintf(stderr, "Warning! parameter_size(): Parameter type is unknown\n");
  }

  return size;
}

long
profile_size(diet_profile_t* p) 
{
  int i;
  long sum;

  sum = 0;
  for (i = 0; i < p->last_inout; i++) {
    sum += parameter_size(&p->parameters[i]);
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
    exit(1);
  }

  COMMUNICATION_DIRECTORY = com_dir;
}

void
add_communication(char* node0, char* node1, long size) 
{
  FILE* f;
  char file_name[1000];

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


#endif // HAVE_CICHLID
