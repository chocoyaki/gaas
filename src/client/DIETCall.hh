/**
 * @file  DIETCall.hh
 *
 * @brief   DIET call methods
 *
 * @author   Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#ifndef _DIET_CALL_H_
#define _DIET_CALL_H_

#include "DIET_grpc.h"
#include "SeD.hh"
#include "MasterAgent.hh"

diet_error_t
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
