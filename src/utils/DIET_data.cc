/****************************************************************************/
/* $Id$ */
/* DIET data interface for clients as well as servers                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP - ENS Lyon (France)                 */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.3  2002/10/03 17:58:20  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.2  2002/08/30 16:50:16  pcombes
 * This version works as well as the alpha version from the user point of view,
 * but the API is now the one imposed by the latest specifications (GridRPC API
 * in its sequential part, config file for all parts of the platform, agent
 * algorithm, etc.)
 *  - Reduce marshalling by using CORBA types internally
 *  - Creation of a class ServiceTable that is to be replaced
 *    by an LDAP DB for the MA
 *  - No copy for client/SeD data transfers
 *  - ...
 *
 * Revision 1.1  2002/08/28 10:07:18  pcombes
 * This commit sets the frame for version 1.0 - does not work properly yet
 * - Some files were still not processed -
 *
 ****************************************************************************/


#include <iostream.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "DIET_data.h"
#include "DIET_client.h"
#include "DIET_server.h"
#include "types.hh"
//#include "dietTypes.hh"

/**
 * Trace level
 */
// FIXME: should be used for traceLevel >= TRACE_STRUCTURES only ...
static int traceLevel;

void data_set_trace_level(int level)
{
  traceLevel = level;
}


/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <implemented in DIET_data.cc>                                            */
/****************************************************************************/


size_t type_sizeof(const diet_base_type_t type)
{
  switch (type) {
  case DIET_CHAR:
  case DIET_BYTE:
    return 1;
  case DIET_INT:
    return 4;
  case DIET_LONGINT:
    return 8;
  case DIET_FLOAT:
    return sizeof(float);
  case DIET_DOUBLE:
    return sizeof(double);
  case DIET_SCOMPLEX:
    return 2 * sizeof(float);
  case DIET_DCOMPLEX:
    return  2 * sizeof(double);
  default:
    return 0;
  }
}

size_t macro_data_sizeof(const diet_data_desc_t *desc)
{
  switch (desc->generic.type) {
  case DIET_SCALAR:
    return 1;
  case DIET_VECTOR:
    return desc->specific.vect.size;
  case DIET_MATRIX:
    return (desc->specific.mat.nb_c * desc->specific.mat.nb_r);
  case DIET_STRING:
    return desc->specific.str.length;
  case DIET_FILE:
    return desc->specific.file.size;
  default:
    cerr << "macro_data_sizeof: Error in type (cons type)\n";
    return 0;
  }
}

size_t data_sizeof(const diet_data_desc_t *desc)
{
  return (macro_data_sizeof(desc) * type_sizeof(desc->generic.base_type));
}

size_t data_sizeof(const corba_data_desc_t *desc)
{
  size_t base_size = type_sizeof((diet_base_type_t) desc->base_type);
  size_t size;
#if 0
  switch(desc->base_type) {
  case DIET_CHAR:
  case DIET_BYTE:
    base_size = sizeof(CORBA::Char); break;
  case DIET_INT:
    base_size = sizeof(CORBA::Long); break;
  case DIET_LONGINT:
    base_size = sizeof(CORBA::LongLong); break;
  case DIET_FLOAT:
    base_size = sizeof(CORBA::Float); break;
  case DIET_DOUBLE:
    base_size = sizeof(CORBA::Double); break;
  case DIET_SCOMPLEX:
    base_size = 2 * sizeof(CORBA::Float); break;
  case DIET_DCOMPLEX:
    base_size =  2 * sizeof(CORBA::Double); break;
  default:
    base_size = 0;
  }
#endif // 0
  switch (desc->specific._d()) {
  case DIET_SCALAR:
    size = 1; break;
  case DIET_VECTOR:
    size = desc->specific.vect().size; break;
  case DIET_MATRIX:
    size = (desc->specific.mat().nb_c * desc->specific.mat().nb_r); break;
  case DIET_STRING:
    size = desc->specific.str().length; break;
  case DIET_FILE:
    size = desc->specific.file().size; break;
  default:
    cerr << "macro_data_sizeof: Error in type (cons type)\n";
    size = 0;
  }
  return (base_size * size);
}


/****************************************************************************/
/* Useful functions for profile manipulation                                */
/****************************************************************************/


int profile_desc_cmp(const corba_profile_desc_t *p1,
		     const corba_profile_desc_t *p2)
{
  int res;
  if ((res = strcmp(p1->path, p2->path)))
    return res;
  if ((res = !((p1->last_in == p2->last_in)
	       && (p1->last_inout == p2->last_inout)
	       && (p1->last_out == p2->last_out)
	       && (p1->param_desc.length() == p2->param_desc.length()))))
    return res;
  res = 0;
  for (size_t i = 0; i < p1->param_desc.length(); i++) {
    if ((res = !((p1->param_desc[i].type
		  == p2->param_desc[i].type)
		 && (p1->param_desc[i].base_type
		     == p2->param_desc[i].base_type))))
      return res;
  }
  return res;
}


int profile_match(const corba_profile_desc_t *sv_profile,
		  const corba_profile_t *pb_profile)
{
  if (strcmp(sv_profile->path, pb_profile->path))
    return 0;
  if ((   (sv_profile->last_in             != pb_profile->last_in)
       || (sv_profile->last_inout          != pb_profile->last_inout)
       || (sv_profile->last_out            != pb_profile->last_out)
       || (sv_profile->param_desc.length() != pb_profile->param_desc.length())))
    return 0;
  for (size_t i = 0; i < sv_profile->param_desc.length(); i++) {
    if ((   (sv_profile->param_desc[i].type
	     != pb_profile->param_desc[i].specific._d())
	 || (sv_profile->param_desc[i].base_type
	     != pb_profile->param_desc[i].base_type)))
      return 0;
  }
  return 1;
}



extern "C" {

/****************************************************************************/
/* Useful functions for data descriptors                                    */
/****************************************************************************/


int scalar_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, void *value)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = generic_desc_set(&(desc->generic), DIET_SCALAR, base_type)))
    return status;
  desc->mode = mode;
  desc->specific.scal.value = value;
  return status;
}

int vector_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t size)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = generic_desc_set(&(desc->generic), DIET_VECTOR, base_type)))
    return status;
  desc->mode = mode;
  desc->specific.vect.size = size;
  return status;
}

int matrix_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t nb_r, size_t nb_c,
		    int istrans)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = generic_desc_set(&(desc->generic), DIET_MATRIX, base_type)))
    return status;
  desc->mode = mode;
  desc->specific.mat.nb_r = nb_r;
  desc->specific.mat.nb_c = nb_c;
  desc->specific.mat.istrans = istrans;
  return status;
 
}

int string_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    size_t length)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = generic_desc_set(&(desc->generic), DIET_STRING, DIET_CHAR)))
    return status;
  desc->mode = mode;
  desc->specific.str.length = length;
  return status;
}

/* Computes the file size */
int file_desc_set(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		  char *path)
{
  int status = 0;
  struct stat buf;
  if (!desc)
    return 1;
  if ((status = generic_desc_set(&(desc->generic), DIET_FILE, DIET_CHAR)))
    return status;
  desc->mode = mode;
  desc->specific.file.path = path;
  if ((status = stat(path, &buf)))
    return status;
  if (buf.st_mode != S_IFREG)
    return 2;
  desc->specific.file.size = (size_t) buf.st_size;
  return status;
}




/****************************************************************************/
/* Useful functions for data                                                */
/****************************************************************************/

int scalar_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type)
{
  int status = 0;
  if (!data)
    return 1;
  if ((status = scalar_desc_set(&(data->desc), mode, base_type, value)))
    return status;
  data->value = value;
  return status;
}

int vector_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t size)
{
  int status = 0;
  if (!data)
    return 1;
  if ((status = vector_desc_set(&(data->desc), mode, base_type, size)))
    return status;
  data->value = value;
  return status;
}

int matrix_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t nb_r, size_t nb_c, int istrans)
{
  int status = 0;
  if (!data)
    return 1;
  if ((status = matrix_desc_set(&(data->desc),mode,base_type,nb_r,nb_c,istrans)))
    return status;
  data->value = value;
  return status;
}

int string_set(diet_data_t *data, char *value, diet_persistence_mode_t mode,
	       size_t length)
{
  int status = 0;
  if (!data)
    return 1;
  if ((status = string_desc_set(&(data->desc), mode, length)))
    return status;
  data->value = value;
  return status;
}

/* Computes the file size */
int file_set(diet_data_t *data, diet_persistence_mode_t mode, char *path)
{
  int status = 0;
  if (!data)
    return 1;
  if ((status = file_desc_set(&(data->desc), mode, path)))
    return status;
  data->value = NULL;
  return status;
}


} // extern "C"


