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
 * Revision 1.6  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
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

typedef struct {
  size_t length;
  diet_data_t *seq;
} diet_data_seq_t;


/****************************************************************************/
/* Useful functions for data                                                */
/****************************************************************************/

/* There should be no use of allocating functions */

/*----[ Data altering ]-----------------------------------------------------*/
/* Each -1 (or NULL for pointers) argument does not alter the corresponding
   field. */

int scalar_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type);
int vector_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t size);
int matrix_set(diet_data_t *data, void *value, diet_persistence_mode_t mode,
	       diet_base_type_t base_type, size_t nb_r, size_t nb_c, int istrans);
int string_set(diet_data_t *data, char *value, diet_persistence_mode_t mode,
	       size_t length);
/* Computes the file size
   ! Warning ! The path is not duplicated !!! */
int file_set(diet_data_t *data, diet_persistence_mode_t mode, char *path);


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
   matrix_set(&(profile->parameters[n]), mode, value, btype, nb_r, nb_c, istrans);
   NB: mode is the persistence mode of the parameter.
   Since a profile will not be freed until profile_free is called, it is
   possible to refer to each parameter for data handles (cf. below)         */
diet_profile_t *profile_alloc(int last_in, int last_inout, int last_out);
int profile_free(diet_profile_t *profile);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_DATA_H_
