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
 * Revision 1.4  2002/10/15 18:45:00  pcombes
 * Implement convertor API and file transfer.
 *
 * Revision 1.3  2002/10/03 17:58:14  pcombes
 * Add trace levels (for Bert): traceLevel = n can be added in cfg files.
 * An agent son can now be killed (^C) without crashing this agent.
 * DIET with FAST: compilation is OK, but run time is still to be fixed.
 *
 * Revision 1.2  2002/08/30 16:50:14  pcombes
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
 * Revision 1.1  2002/08/09 14:30:29  pcombes
 * This is commit set the frame for version 1.0 - does not work properly yet
 *
 ****************************************************************************/


#include <iostream.h>
#include <unistd.h>
#include <stdlib.h>

#include "DIET_server.h"
#include "omniorb.hh"
#include "marshalling.hh"
#include "ServiceTable.hh"
//#include "dietTypes.hh"
#include "SeD_impl.hh"
//#include "libSeD.hh"
//#include "agent.hh"
//#include "SeD.hh"
#include "debug.hh"
#include "types.hh"


extern "C" {


/****************************************************************************/
/* DIET service table                                                       */
/****************************************************************************/

static ServiceTable *SrvT;

int diet_service_table_init(int maxsize)
{
  SrvT = new ServiceTable(maxsize);
  return 0;
}

int diet_service_table_add(char                *service_path,
			   diet_profile_desc_t *profile,
			   diet_convertor_t    *cvt,
			   diet_solve_t         solve_func)
{
  corba_profile_desc_t corba_profile;
  diet_convertor_t    *actual_cvt;
  mrsh_profile_desc(&corba_profile, profile, service_path);
  if (cvt) {
    actual_cvt = cvt;
  } else {
    actual_cvt = convertor_alloc(service_path, profile->last_in,
				 profile->last_inout, profile->last_out);
    for (int i = 0; i <= profile->last_out; i++)
      diet_arg_cvt_set(&(actual_cvt->arg_convs[i]), DIET_CVT_IDENTITY, i, NULL);
  }
  return SrvT->addService(&corba_profile, actual_cvt, solve_func, NULL);
}

void print_table()
{
  SrvT->dump(stdout);
}


/****************************************************************************/
/* DIET service profile descriptor                                          */
/****************************************************************************/

diet_profile_desc_t *profile_desc_alloc(int last_in, int last_inout, int last_out)
{
  diet_profile_desc_t *desc = NULL;
  diet_arg_desc_t *param_desc;
  
  if ((last_in < -1) || (last_inout < -1) || (last_out < -1))
    return NULL;
  if (last_out == -1)
    param_desc = NULL;
  else {
    param_desc = (diet_arg_desc_t *) calloc(last_out + 1,
					    sizeof(diet_arg_desc_t));
    if (!param_desc)
      return NULL;  
  }
  desc = (diet_profile_desc_t *) malloc(sizeof(diet_profile_desc_t));
  if (!desc)
    return NULL;
  desc->last_in    = last_in;
  desc->last_inout = last_inout;
  desc->last_out   = last_out;
  desc->param_desc = param_desc;
  return desc;
}

int profile_desc_free(diet_profile_desc_t *desc)
{
  if (!desc)
    return 1;
  if ((desc->last_out > -1) && desc->param_desc) {
    free(desc->param_desc);
    free(desc);
    return 0;
  } else {
    free(desc);
    return 1;
  }
}
  

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


int diet_arg_cvt_set(diet_arg_convertor_t *arg_cvt,
		     diet_convertor_function_t f, int arg_idx, diet_arg_t *arg)
{
  if (!arg_cvt)
    return 1;
  arg_cvt->f       = f;
  arg_cvt->arg_idx = arg_idx;
  arg_cvt->arg     = arg;
  return 0;
}


diet_convertor_t *convertor_alloc(char *path,
				  int last_in, int last_inout, int last_out)
{
  diet_convertor_t *res = new diet_convertor_t;
  res->path       = strdup(path);
  res->last_in    = last_in;
  res->last_inout = last_inout;
  res->last_out   = last_out;
  res->arg_convs  = new diet_arg_convertor_t[last_out + 1];
  for (int i = 0; i < last_out; i++) {
    res->arg_convs[i].f   = DIET_CVT_COUNT;
    res->arg_convs[i].arg = NULL;
  }
  return res;
}


int convertor_free(diet_convertor_t *cvt)
{
  free(cvt->path);
  for (int i = 0; i < cvt->last_out; i++) {
    if (cvt->arg_convs[i].arg)
      free(cvt->arg_convs[i].arg);
  }
  free(cvt->arg_convs);
  return 0;
}


/****************************************************************************/
/* DIET server call                                                         */
/****************************************************************************/

int DIET_SeD(char *config_file_name, int argc, char **argv)
{
  SeD_impl *SeD;
  
  /* ORB initialization */
  omniORB_init(argc, argv, true);

  /* SeD creation */
  SeD = new SeD_impl(2000);

  /* Activate SeD */
  omniORB_activate(SeD);

  /* Launch SeD */
  if (SeD->run(config_file_name, SrvT)) {
    cerr << "Unable to launch the SeD.\n";
    return 1;
  }


  /* Wait for RPCs : */

  /* FIXME: This is ugly.                 */
  /* A smart signal handling should be    */
  /* implemented anyway. This would       */
  /* allow to kill a server/agent without */
  /* crashing the diet system.            */

  while(1)
    {
      sleep(10000);
    }
 
  return 0;
}


} // extern "C"

