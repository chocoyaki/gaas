/****************************************************************************/
/* Register the state of a request which is send through the MAs graph      */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 ****************************************************************************/

#include "FloodRequestsList.hh"

#ifdef HAVE_MULTI_MA

#include <assert.h>
#include <set.h>

void FloodRequestsList::garbageCollector() {
  garbageCounter = 0 ;
  set<RequestID> garbage ;
  for(iterator iter = requestsList.begin() ;
      iter != requestsList.end() ; iter++) {
    if(iter->second->accessCpt == -1) {
      delete iter->second ;
      garbage.insert(iter->first) ;
    }
  }
  for(set<RequestID>::iterator iter = garbage.begin() ;
      iter == garbage.end() ; iter++)
    requestsList.erase(*iter) ;
}

bool FloodRequestsList::add(FloodRequest& floodRequest) {
  lock.writeLock() ;
  if (garbageCounter > period)
    garbageCollector() ;
  floodRequest.getAccess() ;
  RequestID reqId = floodRequest.getId() ;
  iterator iter = requestsList.find(reqId) ;
  bool result = (iter == requestsList.end()) ;
  if(result) {
    requestsList[reqId] = &floodRequest ;
  }
  lock.unlock() ;
  return result ;
}

FloodRequest & FloodRequestsList::get(const RequestID & reqID) {
  lock.readLock() ;
  iterator iter = requestsList.find(reqID) ;
  if (iter == requestsList.end())
    throw FloodRequestNotFoundException(reqID) ;
  FloodRequest & result = *iter->second ;
  result.getAccess() ;
  lock.unlock() ;
  return result ;
}

#endif // HAVE_MULTI_MA
