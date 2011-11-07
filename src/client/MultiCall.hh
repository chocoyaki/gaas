/**
 * @file  MultiCall.hh
 *
 * @brief   Used to divide a profile and make several calls with just one SeD
 *
 * @author   Ghislain Charrier (Ghislain.Charrier@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


#ifndef _MULTICALL_HH_
#define _MULTICALL_HH_

#include "response.hh"
#include "DIET_client.h"
#include "SeD.hh"
#include <vector>

class MultiCall {
public:
  static corba_response_t *
  get_response();

  static void
  set_response(corba_response_t *response);

  static bool
  updateCall(diet_profile_t *profile, SeD_var &chosenServer);

private:
  static std::vector<int>
  cerfacsSchedule();

  static corba_response_t *corba_response;
};

#endif /* ifndef _MULTICALL_HH_ */
