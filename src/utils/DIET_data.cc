/****************************************************************************/
/* $Id$ */
/* DIET data interface for clients as well as servers                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES           - LIP ENS-Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.9  2003/01/21 12:17:02  pcombes
 * Update UM to API 0.6.3, and "hide" data structures.
 *
 * Revision 1.8  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.4  2002/10/15 18:41:39  pcombes
 * Implement convertor API.
 *
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
 ****************************************************************************/


#include <iostream.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "DIET_data.h"
#include "DIET_client.h"
#include "DIET_server.h"
#include "types.hh"
#include "dietTypes.hh"
#include "debug.hh"

/**
 * Trace level
 */
static int traceLevel;

void data_set_trace_level(int level)
{
  traceLevel = level;
}


/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <declared in dietTypes.hh>                                               */
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
/* Useful functions for data descriptors                                    */
/* <declared in dietTypes.hh>                                               */
/****************************************************************************/


int scalar_set_desc(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, void *value)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic),
				      DIET_SCALAR, base_type)))
    return status;
  if (mode != -1)
    desc->mode = mode;
  desc->specific.scal.value = value;
  return status;
}

int vector_set_desc(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t size)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic),
				      DIET_VECTOR, base_type)))
    return status;
  if (mode != -1)
    desc->mode = mode;
  if (size != 0)
    desc->specific.vect.size = size;
  return status;
}

int matrix_set_desc(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    diet_base_type_t base_type, size_t nb_r, size_t nb_c,
		    int istrans)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic),
				      DIET_MATRIX, base_type)))
    return status;
  if (mode != 0)
    desc->mode = mode;
  if (nb_r != 0)
    desc->specific.mat.nb_r = nb_r;
  if (nb_c != 0)
    desc->specific.mat.nb_c = nb_c;
  if (istrans != -1)
    desc->specific.mat.istrans = istrans;
  return status;
}

int string_set_desc(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		    size_t length)
{
  int status = 0;
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic),
				      DIET_STRING, DIET_CHAR)))
    return status;
  if (mode != -1)
    desc->mode = mode;
  if (length != 0)
    desc->specific.str.length = length;
  return status;
}

/* Computes the file size */
int file_set_desc(diet_data_desc_t *desc, diet_persistence_mode_t mode,
		  char *path)
{
  int status = 0;
  struct stat buf;
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic),
				      DIET_FILE, DIET_CHAR)))
    return status;
  if (mode != -1)
    desc->mode = mode;
  if (path) {
    desc->specific.file.path = path;
    if ((status = stat(path, &buf)))
      return status;
    if (!(buf.st_mode & S_IFREG))
      return 2;
    desc->specific.file.size = (size_t) buf.st_size;
  }
  return status;
}


/****************************************************************************/
/* Useful functions for profile manipulation                                */
/* <declared in dietTypes.hh>                                               */
/****************************************************************************/


int profile_desc_match(const corba_profile_desc_t *p1,
		       const corba_profile_desc_t *p2)
{
  if (strcmp(p1->path, p2->path))
    return 0;
  if (   (p1->last_in             != p2->last_in)
      || (p1->last_inout          != p2->last_inout)
      || (p1->last_out            != p2->last_out)
      || (p1->param_desc.length() != p2->param_desc.length()))
    return 0;
  for (size_t i = 0; i < p1->param_desc.length(); i++) {
    if (   (p1->param_desc[i].type      != p2->param_desc[i].type)
	|| (p1->param_desc[i].base_type != p2->param_desc[i].base_type))
      return 0;
  }
  return 1;
}


int profile_match(const corba_profile_desc_t *sv_profile,
		  const corba_pb_desc_t      *pb_desc)
{
  if (strcmp(sv_profile->path, pb_desc->path))
    return 0;
  if ((   (sv_profile->last_in             != pb_desc->last_in)
       || (sv_profile->last_inout          != pb_desc->last_inout)
       || (sv_profile->last_out            != pb_desc->last_out)
       || (sv_profile->param_desc.length() != pb_desc->param_desc.length())))
    return 0;
  for (size_t i = 0; i < sv_profile->param_desc.length(); i++) {
    if ((   (sv_profile->param_desc[i].type
	     != pb_desc->param_desc[i].specific._d())
	 || (sv_profile->param_desc[i].base_type
	     != pb_desc->param_desc[i].base_type)))
      return 0;
  }
  return 1;
}


int profile_match(const corba_profile_desc_t *sv_profile,
		  const char *path, const corba_profile_t *pb)
{
  if (strcmp(sv_profile->path, path))
    return 0;
  if ((   (sv_profile->last_in             != pb->last_in)
       || (sv_profile->last_inout          != pb->last_inout)
       || (sv_profile->last_out            != pb->last_out)
       || (sv_profile->param_desc.length() != pb->parameters.length())))
    return 0;
  for (size_t i = 0; i < sv_profile->param_desc.length(); i++) {
    if ((   (sv_profile->param_desc[i].type
	     != pb->parameters[i].desc.specific._d())
	 || (sv_profile->param_desc[i].base_type
	     != pb->parameters[i].desc.base_type)))
      return 0;
  }
  return 1;
}


extern "C" {

/****************************************************************************/
/* Profile descriptor                                                       */
/****************************************************************************/

diet_profile_t *diet_profile_alloc(int last_in, int last_inout, int last_out)
{
  diet_profile_t *res;
  
  res = new diet_profile_t;
  res->last_in    = last_in;
  res->last_inout = last_inout;
  res->last_out   = last_out;
  res->parameters = new diet_arg_t[last_out + 1];
  return res;
}

int diet_profile_free(diet_profile_t *profile)
{
  delete [] profile->parameters;
  delete profile;
  return 0;
}


/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/

int
diet_scalar_set(diet_arg_t *arg, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type)
{
  int status = 0;
  if (!arg)
    return 1;
  if ((status = scalar_set_desc(&(arg->desc), mode, base_type, value)))
    return status;
  arg->value = value;
  return status;
}

int
diet_vector_set(diet_arg_t *arg, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type, size_t size)
{
  int status = 0;
  if (!arg)
    return 1;
  if ((status = vector_set_desc(&(arg->desc), mode, base_type, size)))
    return status;
  arg->value = value;
  return status;
}

int
diet_matrix_set(diet_arg_t *arg, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type,
		size_t nb_rows, size_t nb_cols, int isTransposed)
{
  int status = 0;
  if (!arg)
    return 1;
  if ((status = matrix_set_desc(&(arg->desc), mode, base_type,
				nb_rows, nb_cols, isTransposed)))
    return status;
  arg->value = value;
  return status;
}

int
diet_string_set(diet_arg_t *arg, char *value, diet_persistence_mode_t mode,
		size_t length)
{
  int status = 0;
  if (!arg)
    return 1;
  if ((status = string_set_desc(&(arg->desc), mode, length)))
    return status;
  arg->value = value;
  return status;
}

/* Computes the file size */
int
diet_file_set(diet_arg_t *arg, diet_persistence_mode_t mode, char *path)
{
  int status = 0;
  if (!arg)
    return 1;
  if ((status = file_set_desc(&(arg->desc), mode, path)))
    return status;
  arg->value = NULL;
  return status;
}

/****************************************************************************/
/* Utils for setting (IN)OUT arguments (solve functions, after computation) */
/****************************************************************************/

/**
 * Since this function should only be used with (IN)OUT arguments,
 * it copies (* value) into the zone allocated by DIET.
 */
int diet_scalar_desc_set(diet_data_t *data, void *value)
{
  if (data->desc.generic.type != DIET_SCALAR) {
    cerr << "DIET error: diet_scalar_desc_set misused (wrong type)\n";
    return 1;
  }   
  if (!data->value) {
    cerr << "DIET error: diet_scalar_desc_set misused (data->value is NULL)\n";
    return 1;
  }
  switch(data->desc.generic.base_type) {
  case DIET_CHAR:     
  case DIET_BYTE:    *((char *)data->value)     = *((char *)value);     break;
  case DIET_INT:     *((int  *)data->value)     = *((int  *)value);     break;
  case DIET_LONGINT: *((long int *)data->value) = *((long int *)value); break;
  case DIET_FLOAT:   *((float *)data->value)    = *((float *)value);    break;
  case DIET_DOUBLE:  *((double *)data->value)   = *((double *)value);   break;
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
    *((complex *)data->value)        = *((complex *)value);        break;
  case DIET_DCOMPLEX:
    *((double complex *)data->value) = *((double complex *)value); break;
#endif // HAVE_COMPLEX
  default: {
    cerr << "DIET error: diet_scalar_desc_set misused (wrong base type)\n";
    return 1;
  }
  }
  data->desc.specific.scal.value = data->value;
  return 0;
}
    
int diet_matrix_desc_set(diet_data_t *data,
			 size_t nb_r, size_t nb_c, int istrans)
{
  if (data->desc.generic.type != DIET_MATRIX) {
    cerr << "DIET error: diet_matrix_desc_set misused (wrong type)\n";
    return 1;
  }   
  if ((nb_r * nb_c) >
      (data->desc.specific.mat.nb_r * data->desc.specific.mat.nb_c)) {
    cerr << "DIET error: diet_matrix_desc_set misused\n"
	 << "(the new size for the matrix cannot exceed the old one)\n";
    return 1;
  }
  if (nb_r    != 0)
    data->desc.specific.mat.nb_r    = nb_r;
  if (nb_c    != 0)
    data->desc.specific.mat.nb_c    = nb_c;
  if (istrans != 0)
    data->desc.specific.mat.istrans = istrans;    
  return 0;
}

int diet_file_desc_set(diet_data_t *data, char *path)
{
  if (data->desc.generic.type != DIET_FILE) {
    cerr << "DIET error: diet_file_desc_set misused (wrong type)\n";
    return 1;
  }   
//   if (!path) {
//     cerr << "DIET error: diet_file_desc_set misused (path is NULL)\n";
//     return 1;
//   }
  if (path != data->desc.specific.file.path)
    CORBA::string_free(data->desc.specific.file.path);
  data->desc.specific.file.path = path;
  return 0;
}


/****************************************************************************/
/* Utils for getting data descriptions and values                           */
/*    (parameter extraction in solve functions)                             */
/****************************************************************************/


int get_value(diet_data_t *data, void **value)
{
  if (!data)
    return 1;
  if (value) {
    switch(data->desc.generic.base_type) {
    case DIET_CHAR:     
    case DIET_BYTE:    *((char **)value)     = (char *)data->value;     break;
    case DIET_INT:     *((int  **)value)     = (int  *)data->value;     break;
    case DIET_LONGINT: *((long int **)value) = (long int *)data->value; break;
    case DIET_FLOAT:   *((float **)value)    = (float *)data->value;    break;
    case DIET_DOUBLE:  *((double **)value)   = (double *)data->value;   break;
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
      *((complex **)value)        = (complex *)data->value;        break;
    case DIET_DCOMPLEX:
      *((double complex **)value) = (double complex *)data->value; break;
#endif // HAVE_COMPLEX
    default:
      return 1;
    }
  }
  return 0;
}

int _scalar_get(diet_arg_t *arg, void *value, diet_persistence_mode_t *mode)
{
  int res;

  if ((arg->desc.generic.type != DIET_SCALAR) || !arg) {
    cerr << "DIET error: diet_scalar_get misused "
	 << "            (wrong type or arg pointer is NULL)\n";
    return 1;
  }
  if (value) {
    switch(arg->desc.generic.base_type) {
    case DIET_CHAR:     
    case DIET_BYTE:    *((char *)value)     = *((char *)arg->value);     break;
    case DIET_INT:     *((int  *)value)     = *((int  *)arg->value);     break;
    case DIET_LONGINT: *((long int *)value) = *((long int *)arg->value); break;
    case DIET_FLOAT:   *((float *)value)    = *((float *)arg->value);    break;
    case DIET_DOUBLE:  *((double *)value)   = *((double *)arg->value);   break;
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
      *((complex *)value)        = *((complex *)arg->value);        break;
    case DIET_DCOMPLEX:
      *((double complex *)value) = *((double complex *)arg->value); break;
#endif // HAVE_COMPLEX
    default: {
      cerr << "DIET error: diet_scalar_get misused (wrong base type)\n";
      return 1;
    }
    }
  }
  if (mode)
    *mode = arg->desc.mode;
  return 0;
}

int _vector_get(diet_arg_t *arg, void **value, diet_persistence_mode_t *mode,
		size_t *size)
{
  int res;

  if (arg->desc.generic.type != DIET_VECTOR) {
    cerr << "DIET error: diet_vector_get misused (wrong type)\n";
    return 1;
  }   
  if ((res = get_value((diet_data_t *)arg, value))) {
    cerr << "DIET error: diet_vector_get misused "
	 << "(wrong base type or arg pointer is NULL)\n";
    return res;
  }
  if (mode)
    *mode = arg->desc.mode;
  if (size)
    *size = arg->desc.specific.vect.size;
  return 0;
}

int _matrix_get(diet_arg_t *arg, void **value, diet_persistence_mode_t *mode,
		size_t *nb_rows, size_t *nb_cols, int *isTransposed)
{
  int res;

  if (arg->desc.generic.type != DIET_MATRIX) {
    cerr << "DIET error: diet_matrix_get misused (wrong type)\n";
    return 1;
  }   
  if ((res = get_value((diet_data_t *)arg, value))) {
    cerr << "DIET error: diet_matrix_get misused "
	 << "(wrong base type or arg pointer is NULL)\n";
    return res;
  }
  if (mode)
    *mode = arg->desc.mode;
  if (nb_rows)
    *nb_rows = arg->desc.specific.mat.nb_r;
  if (nb_cols)
    *nb_cols = arg->desc.specific.mat.nb_c;
  if (isTransposed)
    *isTransposed = arg->desc.specific.mat.istrans;
  return 0;
}

int _string_get(diet_arg_t *arg, char **value, diet_persistence_mode_t *mode,
		size_t *length)
{
  int res;

  if (arg->desc.generic.type != DIET_STRING) {
    cerr << "DIET error: diet_string_get misused (wrong type)\n";
    return 1;
  }   
  if ((res = get_value((diet_data_t *)arg, (void **)value))) {
    cerr << "DIET error: diet_string_get misused "
	 << "(wrong base type or arg pointer is NULL)\n";
    return res;
  }
  if (mode)
    *mode = arg->desc.mode;
  if (length)
    *length = arg->desc.specific.str.length;
  return 0;
}

int _file_get(diet_arg_t *arg, diet_persistence_mode_t *mode,
	      size_t *size, char **path)
{
  if (arg->desc.generic.type != DIET_FILE) {
    cerr << "DIET error: diet_file_get misused (wrong type)\n";
    return 1;
  }
  if (mode)
    *mode = arg->desc.mode;
  if (size)
    *size = arg->desc.specific.file.size;
  if (path)
    *path = arg->desc.specific.file.path;

  return 0;
}




} // extern "C"
