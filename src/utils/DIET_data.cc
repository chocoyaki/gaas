/****************************************************************************/
/* DIET data interface for clients as well as servers                       */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.19  2003/09/22 21:09:20  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.17  2003/08/01 19:37:47  pcombes
 * Update diet_profile_alloc to the new API (separated from GridRPC)
 *
 * Revision 1.16  2003/07/04 09:48:06  pcombes
 * Use new ERROR and WARNING macros.
 *
 * Revision 1.15  2003/04/10 12:46:03  pcombes
 * Manage data ID (strings).
 *
 * Revision 1.13  2003/02/19 09:03:40  cpera
 * Add headers include for gcc 2.95.3 compatibility under SunOS.
 *
 * Revision 1.12  2003/02/07 17:02:10  pcombes
 * Remove diet_value. Add diet_is_persistent and diet_free_data.
 * Unify diet_scalar_get prototype to the one of the other _get functions.
 ****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;
#include <sys/stat.h>
#include <sys/types.h>
#include "unistd.h"

#include "DIET_data.h"
#include "DIET_client.h"
#include "DIET_server.h"
#include "common_types.hh"
#include "dietTypes.hh"
#include "debug.hh"

#define DATA_INTERNAL_WARNING(formatted_msg)                                \
  INTERNAL_WARNING(__FUNCTION__ << ": " << formatted_msg << endl            \
		   << "Your program might have written in a DIET-reserved " \
		   << "space: please make sure\n you are using use the API "\
		   << "functions to fill in DIET data structures")



/****************************************************************************/
/* Useful functions for data descriptors manipulation                       */
/* <declared in dietTypes.hh>                                               */
/****************************************************************************/


size_t
type_sizeof(const diet_base_type_t type)
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

size_t
macro_data_sizeof(const diet_data_desc_t* desc)
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
    DATA_INTERNAL_WARNING("bad type (cons type)");
    return 0;
  }
}

size_t
data_sizeof(const diet_data_desc_t* desc)
{
  return (macro_data_sizeof(desc) * type_sizeof(desc->generic.base_type));
}

size_t
data_sizeof(const corba_data_desc_t* desc)
{
  size_t base_size(type_sizeof((diet_base_type_t) desc->base_type));
  size_t size(0);
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
    DATA_INTERNAL_WARNING("bad type (cons type)");
  }
  return (base_size * size);
}

size_t
data_sizeof(const  corba_DataMgr_desc_t* desc)
{
  size_t base_size(type_sizeof((diet_base_type_t) desc->base_type));
  size_t size(0);
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
   DATA_INTERNAL_WARNING("bad type (cons type)");
  }
  return (base_size * size);
}


/****************************************************************************/
/* Useful functions for data descriptors                                    */
/* <declared in dietTypes.hh>                                               */
/****************************************************************************/

inline int
generic_set_desc(diet_data_desc_t* desc, char* const id,
		 const diet_persistence_mode_t mode,
		 const diet_data_type_t type, const diet_base_type_t base_type)
{
  int status(0);
  if (!desc)
    return 1;
  if ((status = diet_generic_desc_set(&(desc->generic), type, base_type)))
    return status;
  if (id != NULL)
    desc->id = id;
  if (mode != DIET_PERSISTENCE_MODE_COUNT)
    desc->mode = mode;
  return status;
}

int
scalar_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, void* const value)
{
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_SCALAR, base_type)))
    return status;
  desc->specific.scal.value = value;
  return status;
}

int
vector_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, const size_t size)
{
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_VECTOR, base_type)))
    return status;
  if (size != 0)
    desc->specific.vect.size = size;
  return status;
}

int
matrix_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode,
		const diet_base_type_t base_type, const size_t nb_r,
		const size_t nb_c, const diet_matrix_order_t order)
{
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_MATRIX, base_type)))
    return status;
  if (nb_r != 0)
    desc->specific.mat.nb_r = nb_r;
  if (nb_c != 0)
    desc->specific.mat.nb_c = nb_c;
  if (order != DIET_MATRIX_ORDER_COUNT)
    desc->specific.mat.order = order;
  return status;
}

int
string_set_desc(diet_data_desc_t* desc, char* const id,
		const diet_persistence_mode_t mode, const size_t length)
{
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_STRING, DIET_CHAR)))
    return status;
  if (length != 0)
    desc->specific.str.length = length;
  return status;
}

/* Computes the file size */
int
file_set_desc(diet_data_desc_t* desc, char* const id,
	      const diet_persistence_mode_t mode, char* const path)
{
  int status(0);
  struct stat buf;
  if ((status = generic_set_desc(desc, id, mode, DIET_FILE, DIET_CHAR)))
    return status;
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


int
profile_desc_match(const corba_profile_desc_t* p1,
		   const corba_profile_desc_t* p2)
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


int
profile_match(const corba_profile_desc_t* sv_profile,
	      const corba_pb_desc_t*      pb_desc)
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


int
profile_match(const corba_profile_desc_t* sv_profile,
	      const char* path, const corba_profile_t* pb)
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

diet_profile_t*
diet_profile_alloc(char* pb_name, int last_in, int last_inout, int last_out)
{
  diet_profile_t* res(NULL);
  res = new diet_profile_t;
  res->pb_name    = strdup(pb_name);
  res->last_in    = last_in;
  res->last_inout = last_inout;
  res->last_out   = last_out;
  res->parameters = new diet_arg_t[last_out + 1];
  for (int i = 0; i <= last_out; i++)
    res->parameters[i].desc.id = NULL;
  return res;
}

int
diet_profile_free(diet_profile_t* profile)
{
  free(profile->pb_name);
  delete [] profile->parameters;
  delete profile;
  return 0;
}


/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/

#define get_id(id,arg)      \
  char* id;                 \
  if (!arg)		    \
    return 1;		    \
  if (arg->desc.id == NULL) \
    id = strdup(no_id);	    \
  else			    \
    id = arg->desc.id

int
diet_scalar_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type)
{
  int status(0);
  if ((status = scalar_set_desc(&(arg->desc), NULL, mode, base_type, value)))
    return status;
  arg->value = value;
  return status;
}

int
diet_vector_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type, size_t size)
{
  int status(0);
  if ((status = vector_set_desc(&(arg->desc), NULL, mode, base_type, size)))
    return status;
  arg->value = value;
  return status;
}

int
diet_matrix_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type,
		size_t nb_rows, size_t nb_cols, diet_matrix_order_t order)
{
  int status(0);
  if ((status = matrix_set_desc(&(arg->desc), NULL, mode, base_type,
				nb_rows, nb_cols, order)))
    return status;
  arg->value = value;
  return status;
}

int
diet_string_set(diet_arg_t* arg, char* value, diet_persistence_mode_t mode,
		size_t length)
{
  int status(0);
  if ((status = string_set_desc(&(arg->desc), NULL, mode, length)))
    return status;
  arg->value = value;
  return status;
}

/* Computes the file size */
int
diet_file_set(diet_arg_t* arg, diet_persistence_mode_t mode, char* path)
{
  int status(0);
  if ((status = file_set_desc(&(arg->desc), NULL, mode, path)))
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
int
diet_scalar_desc_set(diet_data_t* data, void* value)
{
  if (data->desc.generic.type != DIET_SCALAR) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if (!data->value) {
    ERROR(__FUNCTION__ << " misused (data->value is NULL)", 1);
  } 
  switch(data->desc.generic.base_type) {
  case DIET_CHAR:     
  case DIET_BYTE:    *((char*)data->value)     = *((char*)value);     break;
  case DIET_INT:     *((int*)data->value)      = *((int*)value);      break;
  case DIET_LONGINT: *((long int*)data->value) = *((long int*)value); break;
  case DIET_FLOAT:   *((float*)data->value)    = *((float*)value);    break;
  case DIET_DOUBLE:  *((double*)data->value)   = *((double*)value);   break;
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
    *((complex*)data->value)        = *((complex*)value);        break;
  case DIET_DCOMPLEX:
    *((double complex*)data->value) = *((double complex*)value); break;
#endif // HAVE_COMPLEX
  default: {
   ERROR(__FUNCTION__ << " misused (wrong base type)", 1);
  }
  }
  data->desc.specific.scal.value = data->value;
  return 0;
}
    
int
diet_matrix_desc_set(diet_data_t* data,
		     size_t nb_r, size_t nb_c, diet_matrix_order_t order)
{
  if (data->desc.generic.type != DIET_MATRIX) {
   ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if ((nb_r * nb_c) >
      (data->desc.specific.mat.nb_r * data->desc.specific.mat.nb_c)) {
     ERROR(__FUNCTION__
	  << " misused (the new size cannot exceed the old one)", 1);
  }
  if (nb_r    != 0)
    data->desc.specific.mat.nb_r    = nb_r;
  if (nb_c    != 0)
    data->desc.specific.mat.nb_c    = nb_c;
  if (order != DIET_MATRIX_ORDER_COUNT)
    data->desc.specific.mat.order = order;    
  return 0;
}

int
diet_file_desc_set(diet_data_t* data, char* path)
{
  if (data->desc.generic.type != DIET_FILE) {
   ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if (path != data->desc.specific.file.path)
    CORBA::string_free(data->desc.specific.file.path);
  data->desc.specific.file.path = path;
  return 0;
}


/****************************************************************************/
/* Utils for getting data descriptions and values                           */
/*    (parameter extraction in solve functions)                             */
/****************************************************************************/


int
get_value(diet_data_t* data, void** value)
{
  if (!data)
    return 1;
  if (value) {
    switch(data->desc.generic.base_type) {
    case DIET_CHAR:     
    case DIET_BYTE:    *((char**)value)     = (char*)data->value;     break;
    case DIET_INT:     *((int**)value)      = (int*)data->value;      break;
    case DIET_LONGINT: *((long int**)value) = (long int*)data->value; break;
    case DIET_FLOAT:   *((float**)value)    = (float*)data->value;    break;
    case DIET_DOUBLE:  *((double**)value)   = (double*)data->value;   break;
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
      *((complex**)value)        = (complex*)data->value;        break;
    case DIET_DCOMPLEX:
      *((double complex**)value) = (double complex*)data->value; break;
#endif // HAVE_COMPLEX
    default:
      return 1;
    }
  }
  return 0;
}

int
_scalar_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode)
{
  int res;

  if ((arg->desc.generic.type != DIET_SCALAR) || !arg) {
    ERROR(__FUNCTION__ << " misused (wrong type or arg pointer is NULL)", 1);
  }
  if ((res = get_value((diet_data_t*)arg, value))) {
    ERROR(__FUNCTION__
	  << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode)
    *mode = arg->desc.mode;
  return 0;
}

int
_vector_get(diet_arg_t *arg, void** value, diet_persistence_mode_t* mode,
	    size_t* size)
{
  int res;

  if (arg->desc.generic.type != DIET_VECTOR) {
   ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if ((res = get_value((diet_data_t*)arg, value))) {
    ERROR(__FUNCTION__
	  << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode)
    *mode = arg->desc.mode;
  if (size)
    *size = arg->desc.specific.vect.size;
  return 0;
}

int
_matrix_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
	    size_t* nb_rows, size_t* nb_cols, diet_matrix_order_t* order)
{
  int res;

  if (arg->desc.generic.type != DIET_MATRIX) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if ((res = get_value((diet_data_t*)arg, value))) {
    ERROR(__FUNCTION__
	  << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode)
    *mode = arg->desc.mode;
  if (nb_rows)
    *nb_rows = arg->desc.specific.mat.nb_r;
  if (nb_cols)
    *nb_cols = arg->desc.specific.mat.nb_c;
  if (order)
    *order = arg->desc.specific.mat.order;
  return 0;
}

int
_string_get(diet_arg_t* arg, char** value, diet_persistence_mode_t* mode,
	    size_t* length)
{
  int res;

  if (arg->desc.generic.type != DIET_STRING) {
   ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }   
  if ((res = get_value((diet_data_t*)arg, (void**)value))) {
   ERROR(__FUNCTION__
	  << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode)
    *mode = arg->desc.mode;
  if (length)
    *length = arg->desc.specific.str.length;
  return 0;
}

int
_file_get(diet_arg_t* arg, diet_persistence_mode_t* mode,
	  size_t* size, char** path)
{
  if (arg->desc.generic.type != DIET_FILE) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if (mode)
    *mode = arg->desc.mode;
  if (size)
    *size = arg->desc.specific.file.size;
  if (path)
    *path = arg->desc.specific.file.path;

  return 0;
}


/****************************************************************************/
/* Free the amount of data pointed at by the value field of an argument.    */
/* This should be used ONLY for VOLATILE data,                              */
/*    - on the server for IN arguments that will no longer be used          */
/*    - on the client for OUT arguments, after the problem has been solved, */
/*      when they will no longer be used.                                   */
/* NB: for files, this function removes the file and free the path (since   */
/*     it has been dynamically allocated by DIET in both cases)             */
/****************************************************************************/

int
diet_free_data(diet_arg_t* arg)
{
  if (diet_is_persistent(*arg)) {
    WARNING(" attempt to use " << __FUNCTION__
	    << " on persistent data - IGNORED");
    return 3;
  }
  switch(arg->desc.generic.type) {
  case DIET_FILE:
    if (arg->desc.specific.file.path) {
      if (unlink(arg->desc.specific.file.path)) {
	perror(arg->desc.specific.file.path);
	return 2;
      }
      free(arg->desc.specific.file.path);
      arg->desc.specific.file.path = NULL;
    } else {
      return 1;
    }
    break;

  default:
    if (arg->value)
      free(arg->value);
    else
      return 1;
    arg->value = NULL;
  }
  return 0;
}


} // extern "C"
