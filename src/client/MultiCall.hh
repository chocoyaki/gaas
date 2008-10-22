#ifndef _MULTICALL_HH_
#define _MULTICALL_HH_

#include "response.hh"
#include <vector>

class MultiCall {

public:
  static void set_response(corba_response_t *response);
  static corba_response_t* get_response();
  static std::vector<int> cerfacsSchedule();

private:
  static corba_response_t *corba_response;
};

#endif
