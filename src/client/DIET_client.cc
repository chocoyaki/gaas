/****************************************************************************/
/* $Id$ */
/* DIET client interface                                                    */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Frederic LOMBARD          - LIFC Besançon (France)                  */
/*    - Philippe COMBES           - LIP ENS Lyon (France)                   */
/*                                                                          */
/*  This is part of DIET software.                                          */
/*  Copyright (C) 2002 ReMaP/INRIA                                          */
/*                                                                          */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.8  2002/11/05 18:32:20  pcombes
 * Fix bugs in config file parsing.
 *
 * Revision 1.7  2002/10/25 10:50:18  pcombes
 * FAST support: convertors implemented and compatible to --without-fast
 *               configure option, but still not tested with FAST !
 *
 * Revision 1.6  2002/10/15 18:46:09  pcombes
 * Some impacts of convertor API.
 *
 * Revision 1.5  2002/10/03 17:58:16  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.4  2002/10/02 17:06:15  pcombes
 * Complete ArgStack API
 *
 * Revision 1.3  2002/09/17 15:23:14  pcombes
 * Bug fixes on inout arguments and examples
 * Add support for omniORB 4.0.0
 *
 * Revision 1.2  2002/08/30 16:50:15  pcombes
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
 * Revision 1.1  2002/08/09 14:30:31  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 ****************************************************************************/


#include "DIET_client.h"

#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "dietTypes.hh"
#include "agent.hh"
#include "SeD.hh"
#include "types.hh"
#include "marshalling.hh"
#include "omniorb.hh"
#include "debug.hh"

extern "C" {


/****************************************************************************/
/* Global variables                                                         */
/****************************************************************************/

/* The Master Agent */
static Agent_var MA;

/* Trace Level */
static int traceLevel;


/****************************************************************************/
/* GridRPC API                                                              */
/****************************************************************************/



/****************************************************************************/
/* Initialize and Finalize session                                          */

int parseConfigFile(char *config_file_name, char *MA_name)
{
  FILE *file; 
  
  file = fopen(config_file_name, "r");

  if (!file) {
    perror("Parsing client configuration file");
    return 1;
  }

  if (fscanf(file, "%s ", MA_name) != 1) {
    cerr << "Parsing client configuration file: Failed to read agent name.\n";
    return 1;
  }
  
  {
    int fscanf_res, level = 1;
    fscanf_res = fscanf(file, "traceLevel = %d ", &level);
    if (fscanf_res == 0 || fscanf_res == 1) {
      traceLevel = level;
    } else {
      cerr << "Parsing client configuration file: Failed to read trace level.\n";
      return 1;
    }
  }

  // Here must be inserted the services parsing

  if (fclose(file))
    cerr << "Warning: cannot close configuration file.\n";
  
  return 0;
}

long int diet_initialize(int argc, char **argv, char *config_file)
{
  char MA_name[257];
  
  if (parseConfigFile(config_file, MA_name))
    return 1;
  
  /* Initialize ORB */
  if (omniORB_init(argc, argv, false)) {
    cerr << "ORB initialization failed\n";
    return 1;
  }
  /* Find Master Agent */
  MA = Agent::_narrow(getAgentReference(MA_name));
  if (CORBA::is_nil(MA))
    return 1;
  
  return 0;
}
  
long int diet_finalize()
{
  return 0;
}


/****************************************************************************/
/* Data handles                                                             */

diet_data_handle_t *diet_data_handle_malloc()
{
  return new diet_data_handle_t;
}

void *diet_get(diet_data_handle_t *handle)
{
  return handle->value;
}

int diet_free(diet_data_handle_t *handle)
{
  delete handle;
  return 0;
}


/****************************************************************************/
/* Argument Stack                                                           */

struct diet_argStack_elt_s {
  diet_arg_mode_t mode;
  diet_arg_t     *arg;
};
struct diet_argStack_s {
  size_t               maxsize, size;
  int                  first;
  diet_argStack_elt_t *stack;
};

diet_argStack_elt_t *newArgStack_elt(diet_arg_mode_t mode, diet_arg_t *arg)
{
  diet_argStack_elt_t *res = new diet_argStack_elt_t;
  res->mode = mode;
  res->arg  = arg;
  return res;
}

diet_arg_mode_t diet_argStack_elt_mode(diet_argStack_elt_t *argStack_elt)
{
  return argStack_elt->mode;
}

diet_arg_t *diet_argStack_elt_arg(diet_argStack_elt_t *argStack_elt)
{
  return argStack_elt->arg;
}

diet_argStack_t *newArgStack(size_t maxsize)
{
  diet_argStack_t *res;
  res = new diet_argStack_t;
  res->maxsize = maxsize;
  res->size = 0;
  res->first = -1;
  res->stack = new diet_argStack_elt_t[maxsize];
  return res;
}

int pushArg(diet_argStack_t *stack, diet_argStack_elt_t *arg)
{
  if (stack->size == stack->maxsize)
    return 1;
  stack->first++;
  stack->stack[stack->first] = *arg;
  return 0;
}

/* !!! allocate a new diet_argStack_elt_t => to be freed !!! */
// FIXME: take care that argument copy is partial
diet_argStack_elt_t *popArg(diet_argStack_t *stack)
{
  diet_argStack_elt_t *arg;
  if (stack->size == 0)
    return NULL;
  arg = (diet_argStack_elt_t *)malloc(sizeof(diet_argStack_elt_t));
  *arg = stack->stack[stack->first];
  stack->first--;
  stack->size--;
  return arg;
}

int destructArgStack(diet_argStack_t *stack)
{
  delete [] stack->stack;
  delete stack;
  return 0;
}



/****************************************************************************/
/* Function handles                                                         */

struct diet_function_handle_s {
  char *pb_name;
  SeD_var server;
};
#define DIET_DEFAULT_SERVER NULL


/* Allocate a function handle and set its server to DIET_DEFAULT_SERVER */
diet_function_handle_t *diet_function_handle_default(char *pb_name)
{
  diet_function_handle_t *res = new diet_function_handle_t;
  res->pb_name = strdup(pb_name);
  res->server = DIET_DEFAULT_SERVER;
  return res;
}

/* Allocate a function handle and set its server */
/* This function is only added for GridRPC compatibility.
   Please, avoid using it !                              */
diet_function_handle_t *diet_function_handle_init(char *server_name)
{
  /* Cannot be implemented until servers register in Naming Service */
  return NULL;
}

/* Free a function handle */
long int diet_function_handle_destruct(diet_function_handle_t *handle)
{
  free(handle->pb_name);
  delete handle;
  return 0;
}

/* Get the function handle linked to reqID */
diet_function_handle_t *diet_get_function_handle(diet_reqID_t reqID)
{
  return NULL;
}



/****************************************************************************/
/* GridRPC call functions                                                   */


/* Submit a request: the profile descriptor.
   Return the index of decision that can be used, -1 if none can be used.
   NB: (*decision)->length() == 0 if no service was found. */

int submission(corba_pb_desc_t *pb, SeqCorbaDecision_t **decision)
{
  int server_OK;
  
  (*decision) = MA->submit(*pb);
  
  if ((*decision)->length() == 0) {
    cerr << "No server found for problem " << pb->path << ".\n";
    server_OK = -1;

  } else {

    if (traceLevel >= TRACE_MAIN_STEPS) {
      cout << "The Master Agent found the following server(s):\n";
      for (size_t i = 0; i < (*decision)->length(); i++) {
	cout << "    " << (**decision)[i].chosenServerName << ":"
	     << (**decision)[i].chosenServerPort << "\n";
      }
    }
    server_OK = 0;
    while ((size_t) server_OK < (*decision)->length()) {
      try {
	// FIXME: this should be contract checking ...
	(**decision)[server_OK].chosenServer->ping();
	break;
      } catch (...) {
	continue;
      }
    }
    if ((size_t) server_OK == (*decision)->length())
      server_OK = -1;
  }
  return server_OK;
}


int diet_call(diet_function_handle_t *handle, diet_profile_t *profile)
{
  corba_pb_desc_t     corba_pb;
  corba_profile_t     corba_profile;
  SeqCorbaDecision_t *decision;
  int subm_count, server_OK, solve_res;
  static int nb_tries = 3;
    
  /* Request submission : try nb_tries times */

  if (mrsh_pb_desc(&corba_pb, profile, handle->pb_name))
    return 1;
  subm_count = 0;
  do {
    server_OK = submission(&corba_pb, &decision);
  }  while ((decision->length() > 0)
	    && (server_OK == -1) && (++subm_count < 3));
  if (decision->length() == 0) {
    cerr << "Unable to find a server.\n";
    return 1;
  }
  if (server_OK == -1) {
    cerr << "Unable to find a server after " << nb_tries << " tries.\n";
    return 1;
  }

#if 0
  if (traceLevel >= TRACE_ALL_STEPS) {
    cout << "Parsing parameters locations:" << endl;
  
    for (int i = 0; i <= pb->last_inout; i++) {
      cout << " Datum " << i << ": ";
      
      if (CORBA::is_nil(decision->decisions[0].dataLocs[i].localization)) {
	cout << "  unknown location" << endl;
      } else {
	cout << "  located on " << decision->decisions[0].dataLocs[i].hostname << ", port "
	     << decision->decisions[0].dataLocs[i].port
	     << "  (pinging server (" << i << ") ... ";
	decision->decisions[0].dataLocs[i].localization->ping();
	cout << "done" << endl;
      }
    }
  }
#endif
  if (mrsh_profile_to_in_args(&corba_profile, profile))
    return 1;
  solve_res =
    (*decision)[server_OK].chosenServer->solve(handle->pb_name, corba_profile);
  if (unmrsh_out_args_to_profile(profile, &corba_profile))
    return 1;
  
  return solve_res;
}


diet_reqID_t diet_call_async(diet_function_handle_t *handle,
			     diet_profile_t *profile)
{
  if (diet_call(handle, profile))
    return 0;
  else
    return  -1;
}



// FIXME: take care that argument copies are partial
int argStack2profile(diet_profile_t *profile, diet_argStack_t *args)
{
  int tmp;
  
  profile->last_in    = 0;
  profile->last_inout = 0;
  profile->last_out = args->size - 1;
  profile->parameters = new diet_arg_t[profile->last_out + 1];
  tmp = 0;
  for (int mode = (int) IN; mode <= (int) OUT; mode++) {
    for (int i = args->first; i >= 0; i--) {
      if (args->stack[i].mode == (diet_arg_mode_t) mode) {
	profile->parameters[tmp] = (*args->stack[i].arg);
	tmp++;
      }
    }
    switch (mode) {
    case IN:    profile->last_in    = tmp - 1; break;
    case INOUT: profile->last_inout = tmp - 1; break;
    case OUT:   {
      if (profile->last_out != tmp - 1) {
	cerr << "DIET internal error: conversion argStack to profile.\n";
	return 1;
      }
    }
    }
  }
  return 0;
}

int diet_call_argstack(diet_function_handle_t *handle,
		       diet_argStack_t *args)
{
  int res;
  diet_profile_t profile;

  if (argStack2profile(&profile, args))
    return 1;
  
  res = diet_call(handle, &profile);

  delete [] profile.parameters;
  return res;
}


diet_reqID_t diet_call_argstack_async(diet_function_handle_t *handle,
				      diet_argStack_t *args)
{
  diet_reqID_t reqID;
  diet_profile_t profile;

  if (argStack2profile(&profile, args))
    return 1;
  
  reqID = diet_call_async(handle, &profile);

  delete [] profile.parameters;
  return reqID;
}


/****************************************************************************/
/* GridRPC control and wait functions                                       */

int diet_probe(diet_reqID_t reqID)
{
  return 1;
}
  
/* This function erases all persistent data that are manipulated by the reqID
   request. Do not forget to call diet_get_data_handle on data you would like
   to save.                                                                 */
int diet_cancel(diet_reqID_t reqID)
{
  return 1;
}

int diet_wait(diet_reqID_t reqID)
{
  return 0;
}

int diet_wait_and(diet_reqID_t *IDs, size_t length)
{
  return 0;
}

int diet_wait_or(diet_reqID_t *IDs, size_t length, diet_reqID_t *IDptr)
{
  return 0;
}

int diet_wait_all()
{
  return 0;
}

int diet_wait_any(diet_reqID_t *IDptr)
{
  return 0;
}



} // extern "C"




