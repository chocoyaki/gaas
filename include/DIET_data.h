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
 * Revision 1.7  2003/01/17 18:08:43  pcombes
 * New API (0.6.3): structures are not hidden, but the user can ignore them.
 *
 * Revision 1.3  2002/10/15 18:36:03  pcombes
 * Remove the descriptors set functions.
 * Add convertors in API :
 *     Convertors let DIET build the sequence of arguments, that the
 *     correspunding solver needs, from the client sequence of arguments
 *     (which might match another service declared with the same solver).
 * For compatibility with old API, just add NULL for the convertor argument in
 * diet_service_table_add. But all the solvers needed before in programs using
 * DIET can be transformed into convertors.
 *
 ****************************************************************************/


#ifndef _DIET_DATA_H_
#define _DIET_DATA_H_

#include <sys/types.h>
#include "DIET_config.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus



/****************************************************************************/
/* Base and data types                                                      */
/****************************************************************************/

typedef enum {
  DIET_CHAR = 0,
  DIET_BYTE,
  DIET_INT,
  DIET_LONGINT,
  DIET_FLOAT,
  DIET_DOUBLE,
  DIET_SCOMPLEX,
  DIET_DCOMPLEX,
  DIET_BASE_TYPE_COUNT
} diet_base_type_t;

typedef enum {
  DIET_SCALAR = 0,
  DIET_VECTOR,
  DIET_MATRIX,
  DIET_STRING,
  DIET_FILE,
  DIET_DATA_TYPE_COUNT
} diet_data_type_t;


/****************************************************************************/
/* Base and data types                                                      */
/****************************************************************************/

typedef enum {
  DIET_VOLATILE = 0,
  DIET_PERSISTENT,
  DIET_STICKY
} diet_persistence_mode_t;



/****************************************************************************/
/* Data descriptions                                                        */
/****************************************************************************/


/*----[ scalar - specific ]-------------------------------------------------*/
struct diet_scalar_specific {
  void *value;
};

/*----[ vector - specific ]-------------------------------------------------*/
struct diet_vector_specific {
  size_t size;
};

/*----[ matrix - specific ]-------------------------------------------------*/
struct diet_matrix_specific {
  size_t nb_r;
  size_t nb_c;
  int    istrans; /* whether the matrix is transposed or not */
};

/*----[ string - specific ]-------------------------------------------------*/
struct diet_string_specific {
  size_t length;
};

/*----[ file - specific ]---------------------------------------------------*/
struct diet_file_specific {
  int   size;
  char *path;
};

/*----[ data - generic ]----------------------------------------------------*/
struct diet_data_generic {
  diet_data_type_t type;
  diet_base_type_t base_type;
};

/*----[ data description ]--------------------------------------------------*/
typedef struct {
  diet_persistence_mode_t  mode;
  struct diet_data_generic generic;
  union {
    struct diet_scalar_specific scal;
    struct diet_vector_specific vect;
    struct diet_matrix_specific mat;
    struct diet_string_specific str;
    struct diet_file_specific   file;
  } specific;
} diet_data_desc_t;


/****************************************************************************/
/* Data type                                                                */
/****************************************************************************/

typedef struct {
  diet_data_desc_t desc;
  void            *value;
} diet_data_t;

/**
 * Type: (type *) diet_value ( (C type name), (diet_data_t *) )
 * Casted pointer to the data value.
 */
#define diet_value(type, data) (type *)((data)->value)


typedef struct {
  size_t length;
  diet_data_t *seq;
} diet_data_seq_t;



/****************************************************************************/
/* Profile descriptor                                                       */
/****************************************************************************/

typedef diet_data_t diet_arg_t;
typedef struct {
  int         last_in, last_inout, last_out;
  diet_arg_t *parameters;
} diet_profile_t;

/* Allocate a DIET profile with memory space for its arguments.
   If no IN argument, please give -1 for last_in.
   If no INOUT argument, please give last_in for last_inout.
   If no OUT argument, please give last_inout for last_out.
   Once the profile is allocated, please use set functions on each parameter.
   For example, the nth argument is a matrix:
   diet_matrix_set(diet_parameter(profile,n),
                   mode, value, btype, nb_r, nb_c, istrans);
   NB: mode is the persistence mode of the parameter.
   Since a profile will not be freed until profile_free is called, it is
   possible to refer to each parameter for data handles (cf. below)         */
diet_profile_t *diet_profile_alloc(int last_in, int last_inout, int last_out);
int diet_profile_free(diet_profile_t *profile);


/**
 * Type: (diet_arg_t *) diet_parameter( (diet_profile_t *), (int) )
 * Pointer to the nth parameter of a profile
 */
#define diet_parameter(pt_profile, n) &((pt_profile)->parameters[(n)])


/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/
/**
 * If mode, base_type or istrans is -1,
 *    size, nb_r, nb_c or length is 0,
 *    path                       is NULL,
 * the correspunding field is not modified.
 */

/* should not be used on server with (IN)OUT arguments */
int
diet_scalar_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type);
/* should not be used on server with (IN)OUT arguments */
int
diet_vector_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type, size_t size);
/* should not be used on server with (IN)OUT arguments */
int
diet_matrix_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
		diet_base_type_t base_type,
		size_t nb_r, size_t nb_c, int istrans);

/* should not be used on server with (IN)OUT arguments */
int
diet_string_set(diet_data_t *data, char *value, diet_persistence_mode_t mode,
		size_t length);
/* Computes the file size
   ! Warning ! The path is not duplicated !!! */
int
diet_file_set(diet_data_t *data, diet_persistence_mode_t mode, char *path);


/****************************************************************************/
/* Utils for getting data descriptions and values                           */
/*    (parameter extraction on server in solve functions)                   */
/*    (parameter extraction on client at results return)                    */
/****************************************************************************/
/**
 * A NULL pointer is not an error (except for data argument): it is simply
 * IGNORED. For instance,
 *   diet_scalar_get(arg, &value, NULL),
 * will only set the value to the value field of the (*arg) structure.
 * 
 * NB: these are macros that let the user not worry about casting its (int **)
 * or (double **) etc. into (void **).
 */

/**
 * Type: int diet_scalar_get((diet_data_t *), (void *),
 *                           (diet_persistence_mode_t *))
 * (void *) means (int *), (double *), (float *), etc., depending on the
 *          base C type of users's data.
 */
#define diet_scalar_get(data, value, mode) \
        _scalar_get((data), (void *)(value), (mode))
/**
 * Type: int diet_vector_get((diet_data_t *), (void **),
 *                           (diet_persistence_mode_t *), (size_t *))
 * (void **) means (int **), (double **), (float **), etc., depending on the
 *           base C type of users's data.
 */
#define diet_vector_get(data, value, mode, size) \
        _vector_get((data), (void **)(value), (mode), (size))
/**
 * Type: int diet_matrix_get((diet_data_t *), (void **),
 *                           (diet_persistence_mode_t *),
 *                           (size_t *), (size_t *), (int *))
 * (void **) means (int **), (double **), (float **), etc., depending on the
 *           base C type of users's data.
 */
#define diet_matrix_get(data, value, mode, nb_r, nb_c, istrans) \
        _matrix_get((data), (void **)(value), (mode), (nb_r), (nb_c), (istrans))
/**
 * Type: int diet_string_get((diet_data_t *), (char **),
 *                           (diet_persistence_mode_t *), (size_t *))
 */
#define diet_string_get(data, value, mode, length) \
        _string_get((data), (char **)(value), (mode), (length))
/**
 * Type: int diet_file_get((diet_data_t *),
 *                         (diet_persistence_mode_t *), (size_t *), (char **))
 */
#define diet_file_get(data, mode, size, path) \
        _file_get((data), (mode), (size), (char **)(path))



// These are the effective get functions 
int _scalar_get(diet_data_t *data, void *value, diet_persistence_mode_t *mode);
int _vector_get(diet_data_t *data, void **value, diet_persistence_mode_t *mode,
		size_t *size);
int _matrix_get(diet_data_t *data, void **value, diet_persistence_mode_t *mode,
		size_t *nb_r, size_t *nb_c, int *istrans);
int _string_get(diet_data_t *data,
		char **value, diet_persistence_mode_t *mode, size_t *length);
int _file_get(diet_data_t *data, diet_persistence_mode_t *mode,
	      size_t *size, char **path);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_DATA_H_

