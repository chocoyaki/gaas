/****************************************************************************/
/* MultiCall.                                                               */
/* It is used to devide a profile and make several calls with just one SeD. */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/*   - Ghislain Charrier (Ghislain.Charrier@ens-lyon.fr)                    */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2011/01/13 23:42:40  ecaron
 * Add missing header
 *
 ****************************************************************************/

#ifndef _MULTICALL_HH_
#define _MULTICALL_HH_

#include "response.hh"
#include "DIET_client.h"
#include "SeD.hh"
#include <vector>

class MultiCall {

public:
  static corba_response_t* get_response();
  static void set_response(corba_response_t *response);
  static bool updateCall(diet_profile_t* profile, SeD_var& chosenServer);

private:
  static std::vector<int> cerfacsSchedule();
  static corba_response_t *corba_response;
};

#endif
