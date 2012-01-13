/**
 * @file  DIETCall.hh
 *
 * @brief   DIET call methods
 *
 * @author   Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _DIET_CALL_H_
#define _DIET_CALL_H_
#ifdef WIN32
   #define DIET_API_LIB __declspec(dllexport)
#else
   #define DIET_API_LIB
#endif
#include "DIET_grpc.h"
#include "SeD.hh"
#include "MasterAgent.hh"

DIET_API_LIB diet_error_t
diet_call_common(MasterAgent_var &MA,
                 diet_profile_t *profile,
                 SeD_var &chosenServer,
                 estVector_t estimVect,
                 unsigned long maxServers);

diet_error_t
diet_call_async_common(MasterAgent_var &MA,
                       diet_profile_t *profile,
                       SeD_var &chosenServer,
                       estVector_t estimVect,
                       unsigned long maxServers,
                       const char *refCallbackServer);

#endif  // _DIET_CALL_H_
