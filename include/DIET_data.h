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
 * Revision 1.34  2005/10/04 12:05:40  alsu
 * minor changes to pacify gcc/g++ 4.0
 *
 * Revision 1.33  2005/09/05 15:53:39  hdail
 * Addition of estimation tags EST_COMMPROXIMITY and EST_TRANSFEREFFORT.
 *
 * Revision 1.32  2005/08/31 14:34:30  alsu
 * New plugin scheduling interface: estVector_t is no longer a structure
 * in and of itself; but rather a pointer to a corba_estimation_t
 * object.
 *
 * Revision 1.31  2005/08/30 12:49:57  ycaniou
 * Added the  diet_submit_call_t in DIET_server.h and the dietJobID field in
 *   the diet_profile_t
 * Added some comments in batch.tex
 *
 * Revision 1.30  2005/06/27 09:53:10  alsu
 * corrected comments for the diet_string_get and diet_paramstring_get macros
 *
 * Revision 1.29  2005/05/27 08:18:16  mjan
 * Moving JuxMem in a more appropriate place (src/utils)
 * Added log messages for VizDIET
 * Added use of JuxMem in the client side
 *
 * Revision 1.28  2005/05/18 14:18:09  mjan
 * Initial adding of JuxMem support inside DIET. dmat_manips examples tested without JuxMem and with JuxMem
 *
 * Revision 1.27  2005/05/15 15:29:33  alsu
 * - adding new aggregator structure
 * - removing obsolete estimation tags
 *
 * Revision 1.26  2005/05/10 11:55:05  alsu
 * optimized and retested estimation vector
 *
 * Revision 1.25  2005/05/02 14:51:53  ycaniou
 * The client API has now the functions diet_call_batch() and diet_call_async_batch(). The client has also the possibility to modify the profile so that it is a batch, parallel or if he wants a special number of procs.
 * Changes in diet_profile_t and diet_profile_desc_t structures
 * Functions to register a parallel or batch problem.
 * The SeD developper must end its profile solve function by a call to diet_submit_batch().
 *
 * Revision 1.24  2004/12/15 18:09:58  alsu
 * cleaner, easier to document interface: changing diet_perfmetric_t back
 * to the simpler one-argument (of type diet_profile_t) version, and
 * hiding a copy of the pointer back to the SeD in the profile.
 *
 * Revision 1.23  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.22.2.4  2004/11/26 16:54:30  alsu
 * string/paramstring functions calculate length on their own (so that
 * they can be null-terminated automatically)
 *
 * Revision 1.22.2.3  2004/11/24 09:30:15  alsu
 * - adding new datatype DIET_PARAMSTRING, which allows users to define
 *   strings for which the value is important for performance evaluation
 *   (and which is consequently stored in the argument description, much
 *   like what is done for DIET_SCALAR arguments)
 * - adding functions to access the type-specific data structures stored
 *   in the diet_data_desc_t.specific union (for use in custom
 *   performance metrics to access data such as those that are described
 *   above)
 *
 * Revision 1.22.2.2  2004/10/27 22:35:50  alsu
 * include
 *
 * Revision 1.22.2.1  2004/10/26 14:12:52  alsu
 * (Tag: AS-plugin-sched)
 *  - branch created to avoid conflicting with release 1.2 (imminent)
 *  - initial commit on branch, new dynamic performance info structure in
 *    the profile
 *
 * Revision 1.22  2004/02/27 10:31:47  bdelfabr
 * modifications applied to corresponding header files
 *
 * Revision 1.21  2003/10/21 13:30:08  bdelfabr
 * removing diet_matrix_set_with_id method
 *
 * Revision 1.20  2003/10/10 14:57:22  bdelfabr
 * adding diet_matrix_set_with_id
 *
 * Revision 1.19  2003/09/27 07:51:06  pcombes
 * Replace silly base type DIET_BYTE by DIET_SHORT.
 *
 * Revision 1.17  2003/07/25 20:37:36  pcombes
 * Separate the DIET API (slightly modified) from the GridRPC API (version of
 * the draft dated to 07/21/2003)
 *
 * Revision 1.14  2003/04/10 11:27:14  pcombes
 * Add the mode DIET_PERSISTENT_RETURN.
 *
 * Revision 1.13  2003/02/07 17:02:10  pcombes
 * Remove diet_value. Add diet_is_persistent and diet_free_data.
 * Unify diet_scalar_get prototype to the one of the other _get functions.
 *
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
/* Aggregator types                                                         */
/****************************************************************************/

typedef enum {
  DIET_AGG_DEFAULT = 0,
  DIET_AGG_PRIORITY
} diet_aggregator_type_t;

/****************************************************************************/
/* Base and data types                                                      */
/****************************************************************************/

typedef enum {
  DIET_CHAR = 0,
  DIET_SHORT,
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
  DIET_PARAMSTRING,
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
  DIET_PERSISTENT_RETURN, // Saved on the server, and a copy brought back
  DIET_PERSISTENT,
  DIET_STICKY,
  DIET_PERSISTENCE_MODE_COUNT
} diet_persistence_mode_t;


/****************************************************************************/
/* Argument type - a structure with two fields:                             */
/*  desc  : a descriptor of the argument                                    */
/*  value : a (void*) pointer to the memory zone where the data are stored. */
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
  char*       pb_name;
  int         last_in, last_inout, last_out;
  diet_arg_t* parameters;

  const void* SeDPtr; /* pointer to SeD object, to be used in
                      ** performance estimation
		      ** YC
		      ** And for batch submission
		      ** FYC
                      */
#ifdef HAVE_BATCH
  unsigned short int batch_flag ;
  int nbprocs ;
  unsigned long walltime ;
  // Used for correspondance batch job ID / DIET job ID
  int dietJobID ;
#endif

} diet_profile_t;

/* Allocate a DIET profile with memory space for its arguments.
   pb_name is deep-copied.
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
diet_profile_alloc(char* pb_name, int last_in, int last_inout, int last_out);
int
diet_profile_free(diet_profile_t* profile);

#ifdef HAVE_BATCH
int
diet_profile_set_batch(diet_profile_t* profile) ;
int
diet_profile_set_parallel(diet_profile_t* profile) ;
int
diet_profile_set_nbprocs(diet_profile_t* profile, int nbprocs) ;
#endif

/**
 * Type: (diet_arg_t*) diet_parameter( (diet_profile_t*), (int) )
 * Pointer to the nth parameter of a profile
 */
#define diet_parameter(pt_profile, n) &((pt_profile)->parameters[(n)])


/****************************************************************************/
/* Utils functions for setting parameters of a problem description          */
/****************************************************************************/
/**
 * On the server side, these functions should not be used on arguments, but only
 * on convertors.
 * If mode                             is DIET_PERSISTENCE_MODE_COUNT,
 * if base_type                        is DIET_BASE_TYPE_COUNT,
 * if order                            is DIET_MATRIX_ORDER_COUNT,
 * if size, nb_rows, nb_cols or length is 0,
 * if path                             is NULL,
 * then the correspunding field is not modified.
 */

int
diet_scalar_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type);
int
diet_vector_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type, size_t size);

/* Matrices can be stored by rows or by columns */
typedef enum {
  DIET_COL_MAJOR = 0,
  DIET_ROW_MAJOR,
  DIET_MATRIX_ORDER_COUNT
} diet_matrix_order_t;

int
diet_matrix_set(diet_arg_t* arg, void* value, diet_persistence_mode_t mode,
		diet_base_type_t base_type,
		size_t nb_rows, size_t nb_cols, diet_matrix_order_t order);



int
diet_string_set(diet_arg_t* arg, char* value, diet_persistence_mode_t mode);

int
diet_paramstring_set(diet_arg_t* arg,
                     char* value,
                     diet_persistence_mode_t mode);

/* Computes the file size
   ! Warning ! The path is not duplicated !!! */
int
diet_file_set(diet_arg_t* arg, diet_persistence_mode_t mode, char* path);

  /** sets only identifier : data is present inside the platform */
void
diet_use_data(diet_arg_t* arg, char* id);

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

#define diet_matrix_get_with_id(arg, id, value, mode, nb_rows, nb_cols, order) \
  _matrix_get_with_id(arg, id, (void**)value, mode, nb_rows, nb_cols, order)
/**
 * Type: int diet_string_get((diet_arg_t*), (char**),
 *                           (diet_persistence_mode_t*))
 */
#define diet_string_get(arg, value, mode) \
  _string_get(arg, (char**)value, mode)

/**
 * Type: int diet_paramstring_get((diet_arg_t*),
 *                                (char**),
 *                                (diet_persistence_mode_t*))
 */
#define diet_paramstring_get(arg, value, mode) \
  _paramstring_get(arg, (char**) value, mode)

/**
 * Type: int diet_file_get((diet_arg_t*),
 *                         (diet_persistence_mode_t*), (size_t*), (char**))
 */
#define diet_file_get(arg, mode, size, path) \
  _file_get(arg, mode, size, (char**)path)



/*
** type-specific data descriptor access functions
*/
typedef const struct diet_scalar_specific* diet_scalar_desc_t;
typedef const struct diet_vector_specific* diet_vector_desc_t;
typedef const struct diet_matrix_specific* diet_matrix_desc_t;
typedef const struct diet_string_specific* diet_string_desc_t;
typedef const struct diet_paramstring_specific* diet_paramstring_desc_t;
typedef const struct diet_file_specific* diet_file_desc_t;
diet_scalar_desc_t diet_scalar_get_desc(diet_arg_t* arg);
diet_vector_desc_t diet_vector_get_desc(diet_arg_t* arg);
diet_matrix_desc_t diet_matrix_get_desc(diet_arg_t* arg);
diet_string_desc_t diet_string_get_desc(diet_arg_t* arg);
diet_paramstring_desc_t diet_paramstring_get_desc(diet_arg_t* arg);
diet_file_desc_t diet_file_get_desc(diet_arg_t* arg);

/****************************************************************************/
/* Free the amount of data pointed at by the value field of an argument.    */
/* This should be used ONLY for VOLATILE data,                              */
/*    - on the server for IN arguments that will no longer be used          */
/*    - on the client for OUT arguments, after the problem has been solved, */
/*      when they will no longer be used.                                   */
/* NB: for files, this function removes the file and frees the path (since  */
/*     it has been dynamically allocated by DIET in both cases)             */
/****************************************************************************/

int
diet_free_data(diet_arg_t* arg);






/****************************************************************************/
/* The following part is useless for users, but may be read for more        */
/* information on data structures.                                          */
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
_string_get(diet_arg_t* arg, char** value, diet_persistence_mode_t* mode);

int
_paramstring_get(diet_arg_t* arg, char** value, diet_persistence_mode_t* mode);

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

struct diet_paramstring_specific {
  size_t length;
  char*  param;
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
    struct diet_scalar_specific      scal;
    struct diet_vector_specific      vect;
    struct diet_matrix_specific      mat;
    struct diet_string_specific      str;
    struct diet_paramstring_specific pstr;
    struct diet_file_specific        file;
  } specific;
} diet_data_desc_t;

struct diet_arg_s {
  diet_data_desc_t desc;
  void*            value;
};

/* diet_data_t is the same as diet_arg_t, so far ... */
typedef struct diet_arg_s diet_data_t;

/**
 * estimation tags
 */
typedef enum {
  EST_INVALID = -1,
  EST_TOTALTIME = 1,
  EST_COMMTIME,
  EST_TCOMP,
  EST_FREECPU,
  EST_FREEMEM,
  EST_NBCPU,
  EST_CPUSPEED,
  EST_TOTALMEM,
  EST_TIMESINCELASTSOLVE,
  EST_COMMPROXIMITY,      
  EST_TRANSFEREFFORT,     
  EST_USERDEFINED
} diet_est_tag_t;


/**
 * estimation vector
 */
typedef struct corba_estimation_t *estVector_t;
typedef const struct corba_estimation_t *estVectorConst_t;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_DATA_H_

