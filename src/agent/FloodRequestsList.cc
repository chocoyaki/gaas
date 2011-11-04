/**
 * @file  FloodRequestList.cc
 *
 * @brief  Register the state of a request which is send through the MAs graph
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "FloodRequestsList.hh"

#ifdef HAVE_MULTI_MA

#include <cassert>
#include <set>
#include <unistd.h>
#include "debug.hh"
#include "DIET_compat.hh"

bool
FloodRequestsList::put(FloodRequest &floodRequest) {
  mutex.lock();
  RequestID reqId = floodRequest.getId();
  iterator iter = requestsList.find(reqId);
  bool result = (iter == requestsList.end());
  if (result) {
    requestsList[reqId] = &floodRequest;
  }
  mutex.unlock();
  return result;
} // put

FloodRequest &
FloodRequestsList::get(const RequestID &reqID) {
  int lp = 0;
  FloodRequest *result = NULL;
  bool find = false;
  // waits a maximum of one second
  while (!find && lp < 100) {
    mutex.lock();
    iterator iter = requestsList.find(reqID);
    find = (iter != requestsList.end());
    if (find) {
      result = &(*iter->second);
      requestsList.erase(iter);
      mutex.unlock();
    } else {
      // if the request is not found, wait 10 ms that a thread free
      // the resource access
      mutex.unlock();
      TRACE_TEXT(20, "FloodRequestsLists sleep 10ms\n");
      diet::usleep(10000);
    }
  }
  if (!find) {
    throw FloodRequestNotFoundException(reqID);
  }
  return *result;
} // get

#endif  // HAVE_MULTI_MA
