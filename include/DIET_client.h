/****************************************************************************/
/* $Id$ */
/* DIET client interface                                                    */
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
 * Revision 1.4  2002/12/03 19:08:24  pcombes
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


#ifndef _DIET_CLIENT_H_
#define _DIET_CLIENT_H_
#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include "DIET_data.h"


/****************************************************************************/
/* A session in GridRPC terminolgy is a DIET client request.                */
/* A DIET client can submit several request in one session.                 */
/****************************************************************************/

typedef long int diet_reqID_t;

/*----[ Initialize and Finalize session ]-----------------------------------*/
long int diet_initialize(int argc, char **argv, char *config_file);
long int diet_finalize();


/****************************************************************************/
/* Data handles                                                             */
/****************************************************************************/

typedef diet_data_t diet_data_handle_t;

/* The grpc_malloc function is useless, since it data descriptor allocation is
   useless and set functions aim at filling the structure.                  */
/* Once the data handle is allocated, please use the set functions to fill in
   the structure. */
diet_data_handle_t *diet_data_handle_malloc();
void *diet_get(diet_data_handle_t *handle);
/* Free data on server, and free handle if and if only allocated by
   diet_data_handle_malloc. */
int diet_free(diet_data_handle_t *handle);

 

/****************************************************************************/
/* Argument Stack                                                           */
/****************************************************************************/

typedef enum {IN, INOUT, OUT} diet_arg_mode_t;
typedef struct diet_argStack_elt_s diet_argStack_elt_t;
typedef struct diet_argStack_s     diet_argStack_t;

diet_arg_mode_t diet_argStack_elt_mode(diet_argStack_elt_t *argStack_elt);
diet_arg_t     *diet_argStack_elt_arg(diet_argStack_elt_t *argStack_elt);

/* !!! allocate a new diet_argStack_elt_t => to be freed !!! */
diet_argStack_elt_t *newArgStack_elt(diet_arg_mode_t mode, diet_arg_t *arg);
diet_argStack_t     *newArgStack(size_t maxsize);
/* !!! copy arg fields !!! */
int                  pushArg(diet_argStack_t *stack, diet_argStack_elt_t *arg);
/* !!! allocate a new diet_argStack_elt_t => to be freed !!! */
diet_argStack_elt_t *popArg(diet_argStack_t *stack);
int                  destructArgStack(diet_argStack_t *stack);



/****************************************************************************/
/* Function handles                                                         */
/****************************************************************************/

typedef struct diet_function_handle_s diet_function_handle_t;

/* Allocate a function handle and set its server to DIET_DEFAULT_SERVER */
diet_function_handle_t *diet_function_handle_default(char *pb_name);

/* Allocate a function handle and set its server */
/* This function is only added for GridRPC compatibility.
   Please, avoid using it !                              */
diet_function_handle_t *diet_function_handle_init(char *server_name);

/* Free a function handle */
long int diet_function_handle_destruct(diet_function_handle_t *handle);

/* Get the function handle linked to reqID */
diet_function_handle_t *diet_get_function_handle(diet_reqID_t reqID);



/****************************************************************************/
/* GridRPC call functions                                                   */
/****************************************************************************/

int diet_call(diet_function_handle_t *handle, diet_profile_t *profile);
diet_reqID_t diet_call_async(diet_function_handle_t *handle,
			     diet_profile_t *profile);
int diet_call_argstack(diet_function_handle_t *handle,
		       diet_argStack_t *args);
diet_reqID_t diet_call_argstack_async(diet_function_handle_t *handle,
				      diet_argStack_t *args);



/****************************************************************************/
/* GridRPC control and wait functions                                       */
/****************************************************************************/

int diet_probe(diet_reqID_t reqID);
/* This function erases all persistent data that are manipulated by the reqID
   request. Do not forget to call diet_get_data_handle on data you would like
   to save.                                                                 */
int diet_cancel(diet_reqID_t reqID);

int diet_wait(diet_reqID_t reqID);
int diet_wait_and(diet_reqID_t *IDs, size_t length);
int diet_wait_or(diet_reqID_t *IDs, size_t length, diet_reqID_t *IDptr);
int diet_wait_all();
int diet_wait_any(diet_reqID_t *IDptr);






#ifdef __cplusplus
}
#endif // __cplusplus

#endif // _DIET_CLIENT_H_

