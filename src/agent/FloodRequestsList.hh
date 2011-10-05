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
 * Revision 1.5  2006/11/16 09:55:54  eboix
 *   DIET_config.h is no longer used. --- Injay2461
 *
 * Revision 1.4  2004/12/15 15:57:08  sdahan
 * rewrite the FloodRequestsList to use a simplest implementation. The previous mutex bugs does not exist anymore.
 *
 * Revision 1.3  2004/10/21 18:14:55  rbolze
 * correct a comment line which wasn't compatible with distrib script :)
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

#ifndef _FLOOD_REQUESTS_LIST_HH_
#define _FLOOD_REQUESTS_LIST_HH_

#include "FloodRequest.hh"

#ifdef HAVE_MULTI_MA

#include <map>
#include "omnithread.h"
#include "RequestID.hh"

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
  omni_mutex mutex ;

  /** a map to store the FloodRequest. */
  typedef map<RequestID, FloodRequest*> RequestsList ;

  /** the iterator of RequestsList */
  typedef RequestsList::iterator iterator ;

  /** the list of all the request */
  RequestsList requestsList ;

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
  bool put(FloodRequest& floodRequest) ;

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
  FloodRequest & get(const RequestID & reqID) ;

} ; // FloodRequestsList

#endif // HAVE_MULTI_MA

#endif // _FLOOD_REQUESTS_LIST_HH_
