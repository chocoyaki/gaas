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
 * Revision 1.14  2003/04/10 11:27:14  pcombes
 * Add the mode DIET_PERSISTENT_RETURN.
 *
 * Revision 1.13  2003/02/07 17:02:10  pcombes
 * Remove diet_value. Add diet_is_persistent and diet_free_data.
 * Unify diet_scalar_get prototype to the one of the other _get functions.
 *
 * Revision 1.12  2003/02/04 10:08:22  pcombes
 * Apply Coding Standards
 *
 * Revision 1.10  2003/01/23 18:37:30  pcombes
 * API 0.6.4: change _set "dummy" arguments
 *
 * Revision 1.9  2003/01/22 17:14:09  pcombes
 * API 0.6.4 : istrans -> order (row- or column-major)
 *
 * Revision 1.8  2003/01/21 12:17:02  pcombes
 * Update UM to API 0.6.3, and "hide" data structures.
 *
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
 ****************************************************************************/

#ifndef _DIET_DATA_H_
#define _DIET_DATA_H_

#include <sys/types.h>

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
/* Various persistence modes for data :                                     */
/*   - DIET_VOLATILE:                                                       */
/*        No persistency at all.                                            */
/*   - DIET_PERSISTENT_RETURN: (valid for INOUT and OUT arguments only)     */
/*        Data are saved on the server and a copy is sent back to the       */
/*        client after the computation is complete.                         */
/*   - DIET_PERSISTENT:                                                     */
/*        Data are saved on the server and nothing is brought back to the   */
/*        client.                                                           */
/*   - DIET_STICKY:                                                         */
/*        Data are saved on the server, they cannot been moved from there   */
/*        to another server, and thus cannot be sent back to the client.    */
/****************************************************************************/

typedef enum {
  DIET_VOLATILE = 0,
  DIET_PERSISTENT_RETURN, // Data are saved on the server, but must be brought back
  DIET_PERSISTENT,
  DIET_STICKY,
  DIET_PERSISTENCE_MODE_COUNT
} diet_persistence_mode_t;


/****************************************************************************/
/* Argument type - a structure with two fields:                             */
/*  desc  : a descriptor of the argument                                    */
/*  value : a (void*) pointer to the memory zone where the data are stored */
/****************************************************************************/

typedef struct diet_arg_s diet_arg_t;

/**
 * Type: int diet_is_persistent ((diet_arg_t))
 * Return true if arg persistence mode is sticky or persistent.
 */
#define diet_is_persistent(arg) \
  (((arg).desc.mode > DIET_VOLATILE) && ((arg).desc.mode <= DIET_STICKY))


/****************************************************************************/
/* Profile descriptor                                                       */
/****************************************************************************/

typedef struct {
  int         last_in, last_inout, last_out;
  diet_arg_t* parameters;
} diet_profile_t;

/* Allocate a DIET profile with memory space for its arguments.
   If no IN argument, please give -1 for last_in.
   If no INOUT argument, please give last_in for last_inout.
   If no OUT argument, please give last_inout for last_out.
   Once the profile is allocated, please use set functions on each parameter.
   For example, the nth argument is a matrix:
   diet_matrix_set(diet_parameter(profile,n),
                   mode, value, btype, nb_r, nb_c, order);
   NB: mode is the persistence mode of the parameter.
   Since a profile will not be freed until profile_free is called, it is
   possible to refer to each parameter for data handles (cf. below)         */
diet_profile_t*
diet_profile_alloc(int last_in, int last_inout, int last_out);
int
diet_profile_free(diet_profile_t* profile);


/**
 * Type: (diet_arg_t*) diet_parameter( (diet_profile_t*), (int) )
 * Pointer to the nth parameter of a profile
 */
#define diet_parameter(pt_profile, n) &((pt_profile)->parameters[(n)])


/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/
/**
 * If mode                             is DIET_PERSISTENCE_MODE_COUNT,
 * if base_type                        is DIET_BASE_TYPE_COUNT,
 * if order                            is DIET_MATRIX_ORDER_COUNT,
 * if size, nb_rows, nb_cols or length is 0,
 * if path                             is NULL,
 * then the correspunding field is not modified.
 */

/* should not be used on server with (IN)OUT arguments */
int
diet_scalar_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type);
/* should not be used on server with (IN)OUT arguments */
int
diet_vector_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type, size_t size);

/* Matrices can be stored by rows or by columns */
typedef enum {
  DIET_COL_MAJOR = 0,
  DIET_ROW_MAJOR,
  DIET_MATRIX_ORDER_COUNT
} diet_matrix_order_t;

/* should not be used on server with (IN)OUT arguments */
int
diet_matrix_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type,
		size_t nb_rows, size_t nb_cols, diet_matrix_order_t order);
/* should not be used on server with (IN)OUT arguments */
int
diet_string_set(diet_arg_t* arg, char* value, diet_persistence_mode_t mode,
		size_t length);
/* Computes the file size
   ! Warning ! The path is not duplicated !!! */
int
diet_file_set(diet_arg_t* arg, diet_persistence_mode_t mode, char* path);


/****************************************************************************/
/* Utils for getting argument descriptions and values                       */
/*    (parameter extraction on server in solve functions)                   */
/*    (parameter extraction on client at results return)                    */
/****************************************************************************/
/**
 * A NULL pointer is not an error (except for arg): it is simply IGNORED.
 * For instance,
 *   diet_scalar_get(arg, &value, NULL),
 * will only set the value to the value field of the (*arg) structure.
 * 
 * NB: these are macros that let the user not worry about casting its (int**)
 * or (double**) etc. into (void**).
 */

/**
 * Type: int diet_scalar_get((diet_arg_t*), (void**),
 *                           (diet_persistence_mode_t*))
 * (void**) means (int**), (double**), (float**), etc., depending on the
 *          base C type of users's data.
 */
#define diet_scalar_get(arg, value, mode) \
  _scalar_get(arg, (void**)value, mode)

/**
 * Type: int diet_vector_get((diet_arg_t*), (void**),
 *                           (diet_persistence_mode_t*), (size_t*))
 * (void**) means (int**), (double**), (float**), etc., depending on the
 *           base C type of users's data.
 */
#define diet_vector_get(arg, value, mode, size) \
  _vector_get(arg, (void**)value, mode, size)

/**
 * Type: int diet_matrix_get((diet_arg_t*), (void**),
 *                           (diet_persistence_mode_t*),
 *                           (size_t*), (size_t*), (diet_matrix_order_t*))
 * (void**) means (int**), (double**), (float**), etc., depending on the
 *           base C type of users's data.
 */
#define diet_matrix_get(arg, value, mode, nb_rows, nb_cols, order) \
  _matrix_get(arg, (void**)value, mode, nb_rows, nb_cols, order)

/**
 * Type: int diet_string_get((diet_arg_t*), (char**),
 *                           (diet_persistence_mode_t*), (size_t*))
 */
#define diet_string_get(arg, value, mode, length) \
  _string_get(arg, (char**)value, mode, length)

/**
 * Type: int diet_file_get((diet_arg_t*),
 *                         (diet_persistence_mode_t*), (size_t*), (char**))
 */
#define diet_file_get(arg, mode, size, path) \
  _file_get(arg, mode, size, (char**)path)




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
diet_free_data(diet_arg_t* arg);






/****************************************************************************/
/* This part is useless for users, but may be read for more information     */
/* on data structures                                                       */
/****************************************************************************/


/****************************************************************************
 * Utils for getting argument descriptions and values
 *    (the effective get function)
 */

int
_scalar_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode);
int
_vector_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
	    size_t* size);
int
_matrix_get(diet_arg_t* arg, void** value, diet_persistence_mode_t* mode,
	    size_t* nb_rows, size_t *nb_cols, diet_matrix_order_t* order);
int
_string_get(diet_arg_t* arg, char** value, diet_persistence_mode_t* mode,
	    size_t* length);
int
_file_get(diet_arg_t* arg, diet_persistence_mode_t* mode,
	  size_t* size, char** path);




/****************************************************************************
 * Data descriptions
 * The user can read this to understand how data structures are built
 * internally, but all functions and macros defined above should be
 * sufficient.
 */

/*----[ scalar - specific ]-------------------------------------------------*/
struct diet_scalar_specific {
  void* value;
};

/*----[ vector - specific ]-------------------------------------------------*/
struct diet_vector_specific {
  size_t size;
};

/*----[ matrix - specific ]-------------------------------------------------*/
struct diet_matrix_specific {
  size_t              nb_r;
  size_t              nb_c;
  diet_matrix_order_t order;
};

/*----[ string - specific ]-------------------------------------------------*/
struct diet_string_specific {
  size_t length;
};

/*----[ file - specific ]---------------------------------------------------*/
struct diet_file_specific {
  int   size;
  char* path;
};

/*----[ data - generic ]----------------------------------------------------*/
struct diet_data_generic {
  diet_data_type_t type;
  diet_base_type_t base_type;
};

/*----[ data description ]--------------------------------------------------*/
typedef struct {
  char* id;     // allocated at the creation of the encapsulating data_handle
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

struct diet_arg_s {
  diet_data_desc_t desc;
  void*            value;
};

/* diet_data_t is the same as diet_arg_t, so far ... */
typedef struct diet_arg_s diet_data_t;



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_DATA_H_

