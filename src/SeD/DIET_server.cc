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
			   diet_solve_t         solve_func)
{
  corba_profile_desc_t corba_profile;
  mrsh_profile_desc(&corba_profile, profile, service_path);
  return SrvT->addService(&corba_profile, solve_func);
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

