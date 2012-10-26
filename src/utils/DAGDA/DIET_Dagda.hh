/**
 * @file DIET_Dagda.hh
 *
 * @brief  Dagda specific functions
 *
 * @author Gael Le Mahec (lemahec@clermont.in2p3.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef __DIET_DAGDA_HH__
#define __DIET_DAGDA_HH__

#include "DIET_data.h"
#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "DIET_grpc.h"
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif


/* Client side. */
extern "C" DIET_API_LIB void
dagda_mrsh_profile(corba_profile_t *, diet_profile_t *, MasterAgent_var &);

extern "C" DIET_API_LIB void
dagda_download_SeD_data(diet_profile_t *, corba_profile_t *);

extern "C" DIET_API_LIB diet_error_t
dagda_get_data_desc(corba_pb_desc_t &, MasterAgent_var &);

/* Server side. */
extern "C" DIET_API_LIB void
dagda_download_data(diet_profile_t &, corba_profile_t &);

extern "C" DIET_API_LIB void
dagda_upload_data(diet_profile_t &profile, corba_profile_t &pb);
#endif /* ifndef __DIET_DAGDA_HH__ */
