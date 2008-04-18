/***********************************************************/
/* Dagda specific functions.                               */
/*                                                         */
/*  Author(s):                                             */
/*    - Gael Le Mahec (lemahec@in2p3.fr)                   */
/*                                                         */
/* $LICENSE$                                               */
/***********************************************************/
/* $Id$
/* $Log
/*
************************************************************/

#ifndef __DIET_DAGDA_HH__
#define __DIET_DAGDA_HH__

#include "DIET_data.h"
#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "DIET_grpc.h"


/* Client side. */
void dagda_mrsh_profile(corba_profile_t*, diet_profile_t*, MasterAgent_var&);
void dagda_download_SeD_data(diet_profile_t*, corba_profile_t*);
diet_error_t dagda_get_data_desc(corba_pb_desc_t&, MasterAgent_var&);

/* Server side. */
void dagda_download_data(diet_profile_t&, corba_profile_t&);
void dagda_upload_data(diet_profile_t& profile, corba_profile_t& pb);
#endif
