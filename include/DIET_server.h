/****************************************************************************/
/* $Id$ */
/* DIET server interface                                                    */
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
 * Revision 1.1  2002/08/09 14:30:21  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 ****************************************************************************/


#ifndef _DIET_SERVER_H_
#define _DIET_SERVER_H_
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


#include "DIET_data.h"


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
/* DIET solve function prototype                                            */
/****************************************************************************/

typedef
int (* diet_solve_t)(diet_data_seq_t *, diet_data_seq_t *, diet_data_seq_t *);


/****************************************************************************/
/* DIET service table API                                                   */
/****************************************************************************/
/* No need to reference the service table since it is unique for the SeD.   */

int diet_service_table_init(int max_size);
int diet_service_table_add(char                *service_path,
			   diet_profile_desc_t *profile,
			   diet_solve_t         solve_func);



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
