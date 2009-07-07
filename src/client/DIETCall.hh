/****************************************************************************/
/* DIET call methods                                                        */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2009/07/07 08:57:21  bisnard
 * Created new file by transferring functions from DIET_client.
 * Added new parameters to functions request_submission, diet_call_common
 * and diet_call_async_common to replace global variables.
 *
 */

#ifndef _DIET_CALL_H_
#define _DIET_CALL_H_

#include "DIET_grpc.h"
#include "SeD.hh"
#include "MasterAgent.hh"

diet_error_t
diet_call_common(MasterAgent_var& MA,
                 diet_profile_t* profile,
                 SeD_var& chosenServer,
                 estVector_t estimVect,
                 unsigned long maxServers);

diet_error_t
diet_call_async_common(MasterAgent_var& MA,
                       diet_profile_t* profile,
                       SeD_var& chosenServer,
                       estVector_t estimVect,
                       unsigned long maxServers,
                       const char *refCallbackServer);

#if HAVE_JUXMEM
void initJuxMem();
void terminateJuxMem();
#endif

#endif // _DIET_CALL_H_
