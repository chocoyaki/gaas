/****************************************************************************/
/* $Id$ */
/* DIET server interface                                                    */
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
 * Revision 1.5  2002/12/03 19:08:24  pcombes
 * Update configure, update to FAST 0.3.15, clean CVS logs in files.
 * Put main Makefile in root directory.
 *
 * Revision 1.3  2002/10/15 18:36:04  pcombes
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


#ifndef _DIET_SERVER_H_
#define _DIET_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "DIET_data.h"


/****************************************************************************/
/* Useful functions for data descriptors                                    */
/****************************************************************************/

inline int generic_desc_set(struct diet_data_generic *desc,
			    diet_data_type_t type, diet_base_type_t base_type)
{
  if (!desc)
    return 1;
  desc->type      = type;
  desc->base_type = base_type;
  return 0;
}


/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

typedef struct diet_data_generic diet_arg_desc_t;

typedef struct {
  int              last_in, last_inout, last_out;
  diet_arg_desc_t *param_desc;
} diet_profile_desc_t;

/* Allocate a DIET profile descriptors with memory space for its argument
   descriptors.
   If no IN argument, please give -1 for last_in.
   If no INOUT argument, please give last_in for last_inout.
   If no OUT argument, please give last_inout for last_out.
   Once allocation is performed, please use set functions for each descriptor.
   For example, the nth argument is a matrix of doubles:
    generic_desc_set(&(profile->param_desc[n]), DIET_MATRIX; DIET_DOUBLE);   */
diet_profile_desc_t *profile_desc_alloc(int last_in, int last_inout, int last_out);
int profile_desc_free(diet_profile_desc_t *desc);


/****************************************************************************/
/* DIET problem evaluation                                                  */
/****************************************************************************/

/* The server may declare several services for only one underlying routine.
   Thus, diet_convertors are useful to translate the various declared profiles
   into the actual profile of the underlying routine, ie the profile that is
   used for the FAST benches.
   Internally, when a client requests for a declared service, the correspunding
   convertor is used to generate the actual profile : this allows evaluation
   (cf. below)
*/

typedef enum {
  DIET_CVT_IDENTITY = 0,
  DIET_CVT_VECT_SIZE,
  DIET_CVT_MAT_NB_ROW,
  DIET_CVT_MAT_NB_COL,
  DIET_CVT_MAT_ISTRANS,
  DIET_CVT_STR_LEN,
  DIET_CVT_FILE_SIZE,
  DIET_CVT_COUNT,
} diet_convertor_function_t;

/**
 * If arg_idx is between last_in and last_out fields of the profile, then the
 * function f is applied to the diet_arg_t which is at arg_idx place in the
 * profile parameters array. Else, it is applied to arg.
 */
typedef struct {
  diet_convertor_function_t f;
  int                 arg_idx;
  diet_arg_t             *arg;
} diet_arg_convertor_t;

int diet_arg_cvt_set(diet_arg_convertor_t *arg_cvt,
		     diet_convertor_function_t f, int arg_idx, diet_arg_t *arg);

typedef struct {
  char *path;
  int last_in, last_inout, last_out;
  diet_arg_convertor_t *arg_convs;
} diet_convertor_t;

diet_convertor_t *convertor_alloc(char *path,
				  int last_in, int last_inout, int last_out);
// Free also arg field (if not NULL) of each diet_arg_convertor_t in arg_convs
// array. Be careful to the coherence between arg_idx and arg fields !!!
int convertor_free(diet_convertor_t *cvt);


/****************************************************************************/
/* DIET evaluation function prototype                                       */
/****************************************************************************/


/* If FAST cannot evaluate the computation time of a request, for instance
   because the FAST benches have not been performed, then the service programmer
   should provide an evaluation function that uses the actual profile fo the
   service to forecast the computation time.
   NB: The profile internally given to the eval function has not the value
   fields set, since evaluation takes place before data are transfered.
 */

typedef
int (* diet_eval_t)(diet_profile_t *, double *);


/****************************************************************************/
/* DIET solve function prototype                                            */
/****************************************************************************/

typedef
int (* diet_solve_t)(diet_profile_t *);


/****************************************************************************/
/* DIET service table API                                                   */
/****************************************************************************/
/* No need to reference the service table since it is unique for the SeD.   */

int diet_service_table_init(int max_size);
/* (cvt = NULL) is equivalent to "no conversion needed" */
int diet_service_table_add(char                *service_path,
			   diet_profile_desc_t *profile,
			   diet_convertor_t    *cvt,
			   diet_solve_t         solve_func);
void print_table();


/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

/* Most users should set argc to 0 and argv to NULL.
   Advanced omniORB users can set these arguments that are transmitted to
   the ORB initialization. */
int DIET_SeD(char *config_file_name, int argc, char **argv);



#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_SERVER_H_
