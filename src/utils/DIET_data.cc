/**
 * @file  DIET_data.cc
 *
 * @brief  DIET data interface for clients as well as servers
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "DIET_server.h"
#include "DIET_data_internal.hh"
#include "DIET_client.h"
#include "common_types.hh"
#include "debug.hh"

#define DATA_INTERNAL_WARNING(formatted_msg)                            \
  INTERNAL_WARNING( \
    __FUNCTION__ << ": " << formatted_msg << "\n"        \
                 << \
    "Your program might have written in a DIET-reserved " \
                 << "space: please make sure\n"                       \
                 << "you are using use the API "                      \
                 << "functions to fill in DIET data structures")



/****************************************************************************/
/* Functions declared in DIET_data_internal.h                               */
/****************************************************************************/

/** Return the size in bytes of a DIET base type. */
size_t
type_sizeof(const diet_base_type_t type) {
  switch (type) {
  case DIET_CHAR:
    return sizeof(char);

  case DIET_SHORT:
    return sizeof(short);

  case DIET_INT:
    return sizeof(long int);

  case DIET_LONGINT:
    return sizeof(long long int);

  case DIET_FLOAT:
    return sizeof(float);

  case DIET_DOUBLE:
    return sizeof(double);

  case DIET_SCOMPLEX:
    return 2 * sizeof(float);

  case DIET_DCOMPLEX:
    return 2 * sizeof(double);

  default:
    return 0;
  } // switch
} // type_sizeof

/** Compute the size in base elements (base type elements) of an argment. */
size_t
macro_data_sizeof(const diet_data_desc_t *desc) {
  switch (desc->generic.type) {
  case DIET_SCALAR:
    return 1;

  case DIET_VECTOR:
    return desc->specific.vect.size;

  case DIET_MATRIX:
    return (desc->specific.mat.nb_c * desc->specific.mat.nb_r);

  case DIET_STRING:
    return desc->specific.str.length;

  case DIET_PARAMSTRING:
    return desc->specific.pstr.length;

  case DIET_FILE:
    return desc->specific.file.size;

  case DIET_CONTAINER:
    // return desc->specific.cont.size;
    return 0;

  default:
    DATA_INTERNAL_WARNING("bad type (cons type)");
    return 0;
  } // switch
} // macro_data_sizeof

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const diet_data_desc_t *desc) {
  return (macro_data_sizeof(desc) * type_sizeof(desc->generic.base_type));
}

/** Compute data size in bytes from their descriptor. */
size_t
data_sizeof(const corba_data_desc_t *desc) {
  size_t base_size(type_sizeof((diet_base_type_t) desc->base_type));
  size_t size(0);
  switch (desc->specific._d()) {
  case DIET_SCALAR:
    size = 1;
    break;
  case DIET_VECTOR:
    size = desc->specific.vect().size;
    break;
  case DIET_MATRIX:
    size = (desc->specific.mat().nb_c * desc->specific.mat().nb_r);
    break;
  case DIET_STRING:
    size = desc->specific.str().length;
    break;
  case DIET_PARAMSTRING:
    size = desc->specific.pstr().length;
    break;
  case DIET_FILE:
    size = desc->specific.file().size;
    break;
  case DIET_CONTAINER:
    // size = desc->specific.cont().size;
    break;
    size = 0;
    break;
  default:
    DATA_INTERNAL_WARNING("bad type (cons type)");
  } // switch
  return (base_size * size);
} // data_sizeof

/** Alter the part of a descriptor that is common to all types. */
inline int
generic_set_desc(diet_data_desc_t *desc, const char *id,
                 const diet_persistence_mode_t mode,
                 const diet_data_type_t type,
                 const diet_base_type_t base_type) {
  int status(0);
  if (!desc) {
    return 1;
  }
  if ((status = diet_generic_desc_set(&(desc->generic), type, base_type))) {
    return status;
  }
  if (id != NULL) {
    desc->id = CORBA::string_dup(id);
  }
  if (mode != DIET_PERSISTENCE_MODE_COUNT) {
    desc->mode = mode;
  }
  return status;
} // generic_set_desc

/**
 * Alter a scalar descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
scalar_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, void *const value) {
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_SCALAR, base_type))) {
    return status;
  }
  desc->specific.scal.value = value;
  return status;
}

/**
 * Alter a vector descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
vector_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type, const size_t size) {
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_VECTOR, base_type))) {
    return status;
  }
  if (size != 0) {
    desc->specific.vect.size = size;
  }
  return status;
}

/**
 * Alter a matrix descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
matrix_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode,
                const diet_base_type_t base_type,
                const size_t nb_r, const size_t nb_c,
                const diet_matrix_order_t order) {
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_MATRIX, base_type))) {
    return status;
  }
  if (nb_r != 0) {
    desc->specific.mat.nb_r = nb_r;
  }
  if (nb_c != 0) {
    desc->specific.mat.nb_c = nb_c;
  }
  if (order != DIET_MATRIX_ORDER_COUNT) {
    desc->specific.mat.order = order;
  }
  return status;
} // matrix_set_desc

/**
 * Alter a string descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
string_set_desc(diet_data_desc_t *desc, char *const id,
                const diet_persistence_mode_t mode, const size_t length) {
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_STRING, DIET_CHAR))) {
    return status;
  }
  // if (length != 0)
  desc->specific.str.length = length;
  return status;
}

int
paramstring_set_desc(diet_data_desc_t *desc, char *const id,
                     const diet_persistence_mode_t mode,
                     const size_t length, const char *const param) {
  int status(0);
  if ((status =
         generic_set_desc(desc, id, mode, DIET_PARAMSTRING, DIET_CHAR))) {
    return status;
  }
  if (length != 0) {
    desc->specific.pstr.length = length;
    desc->specific.pstr.param = strdup(param);
  }
  return status;
} // paramstring_set_desc

/**
 * Alter a file descriptor. Also computes the file size ...
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
file_set_desc(diet_data_desc_t *desc, const char *id,
              const diet_persistence_mode_t mode, const char *path) {
  int status(0);
  struct stat buf;
  if ((status = generic_set_desc(desc, id, mode, DIET_FILE, DIET_CHAR))) {
    return status;
  }
  if (path) {
    desc->specific.file.path = CORBA::string_dup(path);
    if ((status = stat(path, &buf))) {
      return status;
    }
    if (!(buf.st_mode & S_IFREG)) {
      return 2;
    }
    if (buf.st_size == 0) {
      ERROR("One of the file that you want to transfer is of zero size", 1);
    }
    desc->specific.file.size = (size_t) buf.st_size;
  } else {
    desc->specific.file.path = (char *) "";
    // Should be NULL. Needs some verifications...
    desc->specific.file.size = 0;
  }

  return status;
} // file_set_desc

/**
 * Alter a container descriptor.
 * Each -1 (NULL for pointers) argument does not alter the corresponding field.
 */
int
container_set_desc(diet_data_desc_t *desc, char *const id,
                   const diet_persistence_mode_t mode, const size_t size) {
  int status(0);
  if ((status = generic_set_desc(desc, id, mode, DIET_CONTAINER, DIET_CHAR))) {
    return status;
  }
  // desc->specific.cont.size = size;
  desc->specific.cont.size = 0;
  return status;
}


/** Return true if p1 is exactly identical to p2. */
int
profile_desc_match(const corba_profile_desc_t *p1,
                   const corba_profile_desc_t *p2) {
  if (strcmp(p1->path, p2->path)) {
    return 0;
  }
  if ((p1->last_in != p2->last_in) ||
      (p1->last_inout != p2->last_inout) ||
      (p1->last_out != p2->last_out) ||
      (p1->param_desc.length() != p2->param_desc.length())
#if defined HAVE_ALT_BATCH
      || (p1->parallel_flag != p2->parallel_flag)) {
#else
      ) {
#endif
    return 0;
  }
  for (size_t i = 0; i < p1->param_desc.length(); i++) {
    if ((p1->param_desc[i].type != p2->param_desc[i].type) ||
        (p1->param_desc[i].base_type != p2->param_desc[i].base_type)) {
      return 0;
    }
  }
  return 1;
} // profile_desc_match


/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb_desc describes.
 */
int
profile_match(const corba_profile_desc_t *sv_profile,
              const corba_pb_desc_t *pb_desc) {
  if (strcmp(sv_profile->path, pb_desc->path)) {
    return 0;
  }
  if ((sv_profile->last_in != pb_desc->last_in) ||
      (sv_profile->last_inout != pb_desc->last_inout) ||
      (sv_profile->last_out != pb_desc->last_out) ||
      (sv_profile->param_desc.length() != pb_desc->param_desc.length())) {
    return 0;
  }
#if defined HAVE_ALT_BATCH
  /*
  **  - if parallel or sequential is asked, strict check
  **  - if nothing specified, both // and non-// must be considered
  */
  if ((pb_desc->parallel_flag != 0) &&
      (sv_profile->parallel_flag != pb_desc->parallel_flag)) {
    return 0;
  }
#endif
  for (size_t i = 0; i < sv_profile->param_desc.length(); i++) {
    if (((sv_profile->param_desc[i].type !=
          pb_desc->param_desc[i].specific._d()) ||
         (sv_profile->param_desc[i].base_type !=
          pb_desc->param_desc[i].base_type))) {
      return 0;
    }
  }
  return 1;
} // profile_match


/**
 * Return true if sv_profile describes a service that matches the problem that
 * pb and path describe.
 */
int
profile_match(const corba_profile_desc_t *sv_profile, const char *path,
              const corba_profile_t *pb) {
  if (strcmp(sv_profile->path, path)) {
    return 0;
  }
  if ((sv_profile->last_in != pb->last_in) ||
      (sv_profile->last_inout != pb->last_inout) ||
      (sv_profile->last_out != pb->last_out) ||
      (sv_profile->param_desc.length() != pb->parameters.length())) {
    return 0;
  }

#if defined HAVE_ALT_BATCH
  /* As the client request this server, and until here, then SeD propose
     the service. We assume that SeD that was seq or // is still seq or //
   */
#endif

  for (size_t i = 0; i < sv_profile->param_desc.length(); i++) {
    if (((sv_profile->param_desc[i].type !=
          pb->parameters[i].desc.specific._d()) ||
         (sv_profile->param_desc[i].base_type !=
          pb->parameters[i].desc.base_type))) {
      return 0;
    }
  }
  return 1;
} // profile_match



/****************************************************************************/
/* Functions declared in DIET_data.h                                        */
/****************************************************************************/


extern "C" {
/****************************************************************************/
/* Profile descriptor                                                       */
/****************************************************************************/

/* Called from client */
diet_profile_t *
diet_profile_alloc(const char *pb_name, int last_in,
                   int last_inout, int last_out) {
  diet_profile_t *res(NULL);
  res = new diet_profile_t;
  res->pb_name = strdup(pb_name);
  res->last_in = last_in;
  res->last_inout = last_inout;
  res->last_out = last_out;
  res->parameters = new diet_arg_t[last_out + 1];
  for (int i = 0; i <= last_out; i++) {
    res->parameters[i].desc.id = NULL;
  }
  res->dietReqID = 0;
#if defined HAVE_ALT_BATCH
  /* By default, ask for sequential and parallel task (cf DIET_data.h) */
  res->parallel_flag = 0;
  res->nbprocs = 0;
  res->nbprocess = 0;
  res->walltime = 0;
#endif
  return res;
} // diet_profile_alloc

int
diet_profile_free(diet_profile_t *profile) {
  free(profile->pb_name);
  delete [] profile->parameters;
  delete profile;
  return 0;
}

#if defined HAVE_ALT_BATCH
/* Functions for client profile request */
int
diet_profile_set_parallel(diet_profile_t *profile) {
  profile->parallel_flag = 2;
  return 0;
}

int
diet_profile_set_sequential(diet_profile_t *profile) {
  profile->parallel_flag = 1;
  return 0;
}

int
diet_profile_set_nbprocs(diet_profile_t *profile, int nbprocs) {
  if (nbprocs <= 0) {
    ERROR("the Number of procs must be greater than 0", 1);
  }
  profile->nbprocs = nbprocs;
  return 0;
}

/* Functions for server profile registration */
int
diet_profile_desc_set_sequential(diet_profile_desc_t *profile) {
  profile->parallel_flag = 1;
  return 0;
}

int
diet_profile_desc_set_parallel(diet_profile_desc_t *profile) {
  profile->parallel_flag = 2;
  return 0;
}
#endif // if defined HAVE_ALT_BATCH

/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/

#define get_id(id, arg)                         \
  char *id;                                     \
  if (!arg) {                                   \
    return 1;                                   \
  }                                             \
  if (arg->desc.id == NULL) {                   \
    id = strdup(no_id);                         \
  } else {                                       \
    id = arg->desc.id;                           \
  }                                              \


int
diet_scalar_set(diet_arg_t *arg, void *value, diet_persistence_mode_t mode,
                diet_base_type_t base_type) {
  int status(0);
  if ((status =
         scalar_set_desc(&(arg->desc), NULL, mode, base_type, value))) {
    return status;
  }
  arg->value = value;
  return status;
}

int
diet_vector_set(diet_arg_t *arg, void *value,
                diet_persistence_mode_t mode, diet_base_type_t base_type,
                size_t size) {
  int status(0);
  if ((status =
         vector_set_desc(&(arg->desc), NULL, mode, base_type, size))) {
    return status;
  }
  arg->value = value;
  return status;
}

int
diet_matrix_set(diet_arg_t *arg, void *value,
                diet_persistence_mode_t mode, diet_base_type_t base_type,
                size_t nb_rows, size_t nb_cols,
                diet_matrix_order_t order) {
  int status(0);
  if ((status = matrix_set_desc(&(arg->desc), NULL, mode, base_type,
                                nb_rows, nb_cols, order))) {
    return status;
  }
  arg->value = value;
  return status;
}

int
diet_string_set(diet_arg_t *arg, char *value, diet_persistence_mode_t mode) {
  int status(0);
  size_t len;

  if (value) {
    len = strlen((char *) value) + 1;
  } else {
    len = 0;
  }

  if ((status = string_set_desc(&(arg->desc), NULL, mode, len))) {
    return status;
  }
  arg->value = value;
  return status;
} // diet_string_set

int
diet_paramstring_set(diet_arg_t *arg, char *value,
                     diet_persistence_mode_t mode) {
  int status(0);
  if (!value) {
    ERROR(
      __FUNCTION__ << " misused (value is NULL).\n"
                   << "Make sure you allocated the paramstring pointer.\n"
                   <<
      "Note that paramstrings cannot be used as OUT parameters", 1);
  }
  if ((status = paramstring_set_desc(&(arg->desc), NULL, mode,
                                     strlen((char *) value) + 1,
                                     value))) {
    return status;
  }

  arg->value = value;
  return status;
} // diet_paramstring_set

int
diet_container_set(diet_arg_t *arg, diet_persistence_mode_t mode) {
  int status(0);
  if ((status = container_set_desc(&(arg->desc), NULL, mode, 0))) {
    return status;
  }
  return status;
}

void
diet_use_data(diet_arg_t *arg, const char *id) {
  arg->value = NULL;
  arg->desc.id = CORBA::string_dup(id);
}

/* Computes the file size (stocked in a field of arg) */
int
diet_file_set(diet_arg_t *arg, const char *path,
              diet_persistence_mode_t mode) {
  int status(0);
  if ((status = file_set_desc(&(arg->desc), NULL, mode, path))) {
    return status;
  }
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
diet_scalar_desc_set(diet_data_t *data, void *value) {
  return diet_scalar_set(data, value,
                         data->desc.mode, data->desc.generic.base_type);

  if (data->desc.generic.type != DIET_SCALAR) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if (!data->value) {
    ERROR(__FUNCTION__ << " misused (data->value is NULL)", 1);
  }
  switch (data->desc.generic.base_type) {
  case DIET_CHAR:
    *((char *) data->value) = *((char *) value);
    break;
  case DIET_SHORT:
    *((short *) data->value) = *((short *) value);
    break;
  case DIET_INT:
    *((int *) data->value) = *((int *) value);
    break;
  case DIET_LONGINT:
    *((long int *) data->value) = *((long int *) value);
    break;
  case DIET_FLOAT:
    *((float *) data->value) = *((float *) value);
    break;
  case DIET_DOUBLE:
    *((double *) data->value) = *((double *) value);
    break;
#if HAVE_COMPLEX
  case DIET_SCOMPLEX:
    *((complex *) data->value) = *((complex *) value);
    break;
  case DIET_DCOMPLEX:
    *((double complex *) data->value) = *((double complex *) value);
    break;
#endif  // HAVE_COMPLEX
  default: {
    ERROR(__FUNCTION__ << " misused (wrong base type)", 1);
  }
  } // switch
  data->desc.specific.scal.value = data->value;
  return 0;
} // diet_scalar_desc_set

int
diet_matrix_desc_set(diet_data_t *data, size_t nb_r, size_t nb_c,
                     diet_matrix_order_t order) {
  if (data->desc.generic.type != DIET_MATRIX) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if ((nb_r * nb_c) >
      (data->desc.specific.mat.nb_r * data->desc.specific.mat.nb_c)) {
    ERROR(__FUNCTION__
          << " misused (the new size cannot exceed the old one)", 1);
  }
  if (nb_r != 0) {
    data->desc.specific.mat.nb_r = nb_r;
  }
  if (nb_c != 0) {
    data->desc.specific.mat.nb_c = nb_c;
  }
  if (order != DIET_MATRIX_ORDER_COUNT) {
    data->desc.specific.mat.order = order;
  }
  return 0;
} // diet_matrix_desc_set

int
diet_file_desc_set(diet_data_t *data, char *path) {
  if ((data->desc.generic.type != DIET_FILE)) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if (path != data->desc.specific.file.path) {
    /* FIXME: erase me if ok:
       was created with new char[25] in unmarsh_data
       and released with free((char*)data->desc.specific.file.path);
       Now, it is CORBA stuff
     */
    CORBA::string_free(data->desc.specific.file.path);
  }
  data->desc.specific.file.path = path;
  return 0;
} // diet_file_desc_set


/****************************************************************************/
/* Utils for getting data descriptions and values                           */
/*    (parameter extraction in solve functions)                             */
/****************************************************************************/


int
get_value(diet_data_t *data, void **value) {
  if (!data) {
    return 1;
  }
  if (value) {
    switch (data->desc.generic.base_type) {
    case DIET_CHAR:
    case DIET_SHORT:
    case DIET_INT:
    case DIET_LONGINT:
    case DIET_FLOAT:
    case DIET_DOUBLE:
#if HAVE_COMPLEX
    case DIET_SCOMPLEX:
    case DIET_DCOMPLEX:
#endif /* HAVE_COMPLEX */
      *(value) = data->value;
      break;
    default:
      return (1);
    } // switch
  }
  return 0;
} // get_value

int
_scalar_get(diet_arg_t *arg, void **value, diet_persistence_mode_t *mode) {
  int res;

  if ((arg->desc.generic.type != DIET_SCALAR) || !arg) {
    ERROR(__FUNCTION__ << " misused (wrong type or arg pointer is NULL)", 1);
  }
  if ((res = get_value((diet_data_t *) arg, value))) {
    ERROR(__FUNCTION__
          << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode) {
    *mode = arg->desc.mode;
  }
  return 0;
} // _scalar_get

int
_vector_get(diet_arg_t *arg, void **value,
            diet_persistence_mode_t *mode, size_t *size) {
  int res;

  if (arg->desc.generic.type != DIET_VECTOR) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if ((res = get_value((diet_data_t *) arg, value))) {
    ERROR(__FUNCTION__
          << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode) {
    *mode = arg->desc.mode;
  }
  if (size) {
    *size = arg->desc.specific.vect.size;
  }
  return 0;
} // _vector_get

int
_matrix_get(diet_arg_t *arg, void **value, diet_persistence_mode_t *mode,
            size_t *nb_rows, size_t *nb_cols, diet_matrix_order_t *order) {
  int res;

  if (arg->desc.generic.type != DIET_MATRIX) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }

  if ((res = get_value((diet_data_t *) arg, value))) {
    ERROR(__FUNCTION__
          << " misused (wrong base type or arg pointer is NULL)", res);
  }

  if (mode) {
    *mode = arg->desc.mode;
  }
  if (nb_rows) {
    *nb_rows = arg->desc.specific.mat.nb_r;
  }
  if (nb_cols) {
    *nb_cols = arg->desc.specific.mat.nb_c;
  }
  if (order) {
    *order = arg->desc.specific.mat.order;
  }
  return 0;
} // _matrix_get

int
_string_get(diet_arg_t *arg, char **value, diet_persistence_mode_t *mode) {
  int res;

  if (arg->desc.generic.type != DIET_STRING) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if ((res = get_value((diet_data_t *) arg, (void **) value))) {
    ERROR(__FUNCTION__
          << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode) {
    *mode = arg->desc.mode;
  }

  return 0;
} // _string_get

int
_paramstring_get(diet_arg_t *arg, char **value,
                 diet_persistence_mode_t *mode) {
  int res;

  if (arg->desc.generic.type != DIET_PARAMSTRING) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if ((res = get_value((diet_data_t *) arg, (void **) value))) {
    ERROR(__FUNCTION__
          << " misused (wrong base type or arg pointer is NULL)", res);
  }
  if (mode) {
    *mode = arg->desc.mode;
  }

  return 0;
} // _paramstring_get

int
_file_get(diet_arg_t *arg, char **path, diet_persistence_mode_t *mode,
          size_t *size) {
  if (arg->desc.generic.type != DIET_FILE) {
    ERROR(__FUNCTION__ << " misused (wrong type)", 1);
  }
  if (mode) {
    *mode = arg->desc.mode;
  }
  if (size) {
    *size = arg->desc.specific.file.size;
  }
  if (path) {
    /* Assume that path contains NULL. If not, it's SeD programmer's error */
    *path = arg->desc.specific.file.path;
  }

  return 0;
} // _file_get

diet_scalar_desc_t
diet_scalar_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_SCALAR) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.scal));
}

diet_vector_desc_t
diet_vector_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_VECTOR) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.vect));
}

diet_matrix_desc_t
diet_matrix_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_MATRIX) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.mat));
}

diet_string_desc_t
diet_string_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_STRING) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.str));
}

diet_paramstring_desc_t
diet_paramstring_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_PARAMSTRING) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.pstr));
}

diet_file_desc_t
diet_file_get_desc(diet_arg_t *arg) {
  if (arg->desc.generic.type != DIET_FILE) {
    ERROR(__FUNCTION__ << " misused (wrong type)", NULL);
  }
  return (&((arg->desc).specific.file));
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
diet_free_data(diet_arg_t *arg) {
  return 0;
}

#ifdef HAVE_WORKFLOW

#define LINE_LENGTH 1024
/****************************************************************************/
/* Workflow profile descriptor                                              */
/****************************************************************************/

/**
 * Workflow profile allocation method *
 * (common for dags and functional workflows)
 */
diet_wf_desc_t *
diet_wf_profile_alloc(const char *wf_file_name, const char *name,
                      wf_level_t wf_level) {
  diet_wf_desc_t *profile = new diet_wf_desc_t;
  profile->level = wf_level;
  profile->name = strdup(name);
  profile->dataFile = NULL;
  profile->wfReqID = -1;
  struct stat stat_p;
  FILE *file;
  char line[LINE_LENGTH];

  stat(wf_file_name, &stat_p);
  profile->abstract_wf = (char *) malloc(stat_p.st_size + 1);
  strcpy(profile->abstract_wf, "");
  file = fopen(wf_file_name, "r");
  if (file != NULL) {
    while (fgets(line, LINE_LENGTH, file) != NULL) {
      strcat(profile->abstract_wf, line);
    }
    fclose(file);
  } else {
    WARNING("Initializing workflow profile with invalid file : "
            << wf_file_name);
  }

  return profile;
} // diet_wf_profile_alloc

/**
 * Workflow profile method to set data file name
 * (for functional wf only)
 */
void
diet_wf_set_data_file(diet_wf_desc_t *profile, const char *data_file_name) {
  if (data_file_name) {
    profile->dataFile = strdup(data_file_name);
  }
}

/**
 * Workflow profile method to set transcript file name
 * (for DAG or FUNCTIONAL wf)
 */
void
diet_wf_set_transcript_file(diet_wf_desc_t *profile,
                            const char *transcript_file_name) {
  if (transcript_file_name) {
    profile->transcriptFile = strdup(transcript_file_name);
  }
}

/**
 * Free a workflow profile *
 */
void
diet_wf_profile_free(diet_wf_desc_t *profile) {
  if (profile->abstract_wf) {
    free(profile->abstract_wf);
  }
  if (profile->name) {
    free(profile->name);
  }
  if (profile->dataFile) {
    free(profile->dataFile);
  }
  if (profile->transcriptFile) {
    free(profile->transcriptFile);
  }
  free(profile);
} // diet_wf_profile_free
#endif  // endif HAVE_WORKFLOW
}  // extern "C"
