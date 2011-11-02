/**
* @file  FloodRequestList.cc
* 
* @brief  Register the state of a request which is send through the MAs graph
* 
* @author - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.6  2011/03/20 14:40:03  bdepardo
 * move nanobased usleep implementation to src/utils/DIET_compat.{hh, cc}
 *
 * Revision 1.5  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.4  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.3  2004/12/15 15:57:08  sdahan
 * rewrite the FloodRequestsList to use a simplest implementation. The previous mutex bugs does not exist anymore.
 *
 * Revision 1.2  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 ****************************************************************************/

#include "FloodRequestsList.hh"

#ifdef HAVE_MULTI_MA

#include <cassert>
#include <set>
#include <unistd.h>
#include "debug.hh"
#include "DIET_compat.hh"

bool
FloodRequestsList::put(FloodRequest& floodRequest) {
  mutex.lock();
  RequestID reqId = floodRequest.getId();
  iterator iter = requestsList.find(reqId);
  bool result = (iter == requestsList.end());
  if (result) {
    requestsList[reqId] = &floodRequest;
  }
  mutex.unlock();
  return result;
}

FloodRequest &
FloodRequestsList::get(const RequestID & reqID) {
  int lp = 0;
  FloodRequest* result = NULL;
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
}

#endif  // HAVE_MULTI_MA
