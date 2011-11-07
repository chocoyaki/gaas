/**
 * @file  FloodRequestList.hh
 *
 * @brief  Register the state of a request which is send through the MAs graph
 *
 * @author  Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */

#ifndef _FLOOD_REQUESTS_LIST_HH_
#define _FLOOD_REQUESTS_LIST_HH_

#include "FloodRequest.hh"

#ifdef HAVE_MULTI_MA

#include <map>
#include "omnithread.h"
#include "RequestID.hh"


class FloodRequestNotFoundException {
public:
  /** constructor, reqID is the request identifier which was not
      found.*/
  explicit
  FloodRequestNotFoundException(const RequestID &reqId)
    : reqId(reqId) {
  }

  /** the id of the request which throw the exception */
  CORBA::Long reqId;

  friend std::ostream &
  operator << (std::ostream & output, const FloodRequestNotFoundException &e);
};

/**
 * Writes an ascii description of a exception.
 */
inline std::ostream &
operator << (std::ostream & output, const FloodRequestNotFoundException &e) {
  return output << "FloodRequestNotFoundException: " << e.reqId;
}


/**
 * Stores the FloodRequests. It have a read and a write access to it.
 */

class FloodRequestsList {
public:
  /**
   * adds a new request in the list. If there is another request with
   * the same ID, it does nothing.
   *
   * @param floodRequest the request added to the list.
   *
   * @return true if the floodRequest is added to the list. false if
   * another request with the same id was added.
   */
  bool
  put(FloodRequest &floodRequest);

  /**
   * get and remove from the list the flood request with the ID
   * reqID. if the flood request does not exists, an
   * FloodRequestNotFoundException is send.
   *
   * @param reqID the id of the searched request.
   *
   * @exception FloodRequestNotFoundException when the flood request
   * with the id reqID does not exist.
   *
   * @return a reference on the flood request.
   */
  FloodRequest &
  get(const RequestID &reqID);

private:
  /** The lock which control the access to the list */
  omni_mutex mutex;

  /** a map to store the FloodRequest. */
  typedef std::map<RequestID, FloodRequest *> RequestsList;

  /** the iterator of RequestsList */
  typedef RequestsList::iterator iterator;

  /** the list of all the request */
  RequestsList requestsList;
};  // FloodRequestsList

#endif  // HAVE_MULTI_MA

#endif  // _FLOOD_REQUESTS_LIST_HH_
