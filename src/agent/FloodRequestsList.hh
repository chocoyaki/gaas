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
 ****************************************************************************///

#ifndef _FLOOD_REQUESTS_LIST_HH_
#define _FLOOD_REQUESTS_LIST_HH_

#include "DIET_config.h"

#ifdef HAVE_MULTI_MA

#include <map>
#include "RequestID.hh"
#include "FloodRequest.hh"
#include "ReadersWriterLock.hh"

/**
 * exception throw when we try to get an FloodRequest that does not
 * existe.
 */
class FloodRequestNotFoundException {
public :
  /** constructor, reqID is the request identifier which was not
      found.*/
  inline FloodRequestNotFoundException(const RequestID & reqId) : reqId(reqId)
  { /* does nothing */ }

  /** the id of the request which throw the exception */
  CORBA::Long reqId ;

  friend std::ostream& operator<<(std::ostream& output,
			     const FloodRequestNotFoundException & e);
} ;

/**
 * Writes an ascii description of a exception.
 */
inline std::ostream& operator<<(std::ostream& output,
			   const FloodRequestNotFoundException & e) {
  return output << "FloodRequestNotFoundException: " << e.reqId ;
}


/**
 * Stores the FloodRequests. It have a read and a write access to it.
 */

class FloodRequestsList {

private :
  /** The lock which control the access to the list */
  DietReadersWriterLock lock ;

  /** a counter to know when the garbage collector is called */
  unsigned int garbageCounter ;

  /** indicates after how many period call to the method add the
      garbage collector must be executed.*/
  const static unsigned int period = 258 ;

  /** a map to store the FloodRequest. */
  typedef map<RequestID, FloodRequest*> RequestsList ;

  /** the iterator of RequestsList */
  typedef RequestsList::iterator iterator ;

  /** the list of all the request */
  RequestsList requestsList ;

  /** the garbage collector */
  void garbageCollector() ;

public :
  /**
   * adds a new request in the list. If there is another request with
   * the same ID, it does nothing.
   *
   * @param floodRequest the request added to the list.
   *
   * @return true if the floodRequest is added to the list. false if
   * another request with the same id was added.
   */
  bool add(FloodRequest& floodRequest) ;

  /**
   * get the flood request with the ID reqID. if the flood request
   * does not exists, an FloodRequestNotFoundException is send.
   *
   * @param reqID the id of the searched request.
   *
   * @exception FloodRequestNotFoundException when the flood request
   * with the id reqID does not exist.
   *
   * @return a reference on the flood request.
   */
  FloodRequest & get(const RequestID & reqID) ;

} ; // FloodRequestsList

#endif // HAVE_MULTI_MA

#endif // _FLOOD_REQUESTS_LIST_HH_
