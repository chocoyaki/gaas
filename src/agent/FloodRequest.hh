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
 ****************************************************************************///

#ifndef _FLOOD_REQUEST_HH_
#define _FLOOD_REQUEST_HH_

#include "MasterAgentImpl.hh"
#include "DIET_config.h"

#ifdef HAVE_MULTI_MA

#include "omnithread.h"
#include <map>


class FloodRequestsList ;

/**
 * This class manages the search of a SeD by flooding the MA
 * graph. Each MA can have some FloodRequest object that find a SeD
 * for a problem by contacting the neighborhood MA.
 *
 * @author Sylvain DAHAN - LIFC Besancon (France)
 */

class FloodRequest {

  friend class FloodRequestsList ;

private :
  /**
   * control the access of the access compter
   */
  mutable omni_mutex accessMutex ;

  /**
   * count the number of simultaneous access to the Request. a
   * negative value means that the Request can be removed.
   */
  mutable int accessCpt ;

  /**
   * the mutex which is used to make the FloodRequest thread safe
   */
  mutable omni_mutex locker ;

  /**
   * true if the FloodRequest is defined
   */
  bool _defined ;

  /**
   * the description of the predecessor, it is not defined if there is
   * no predecessor.
   */
  MasterAgentImpl::MADescription predecessor ;

  /**
   * This is the description of the owner of the FloodRequest. The
   * owner is the MasterAgent_impl which are created the instance of
   * the current object.
   */
  MasterAgentImpl::MADescription owner ;

  /**
   * This structure is used to have the list of the Neighboring MA
   * and there state for the current flooding.
   */
  typedef struct {
    /** The description of a neighboring MA */
    MasterAgentImpl::MADescription maDescription ;
    /** its state */
    enum state_t {
      /** it is not contacted */
      nmaStateNotContacted,
      /** it is already contacted by another MA. */
      nmaStateAlreadyContacted,
      /** it is contacted and completely flooded. */
      nmaStateFlooded,
      /** it is contacted and not completely flooded. */
      nmaStateFlooding
    } state ;
  } NeighbouringMA ;

  /**
   * This is a list of neighboring MAs indexed by there
   * KeyString.
   */
  typedef std::map<KeyString, NeighbouringMA> NeighbouringMAs ;

  /**
   * List of the neighboring MAs and there states.
   */ 
  NeighbouringMAs neighbouringMAs ;

  /**
   * The description of a problem. The flood request searches a SeD
   * which can resolve this problem.
   */
  corba_request_t request ;

  /**
   * The union of the decisions taken by the graph of MA
   */
  corba_response_t decisions ;
  

  /**
   * Number of responses waited by the current flood.
   */
  int nbOfWaitingResponse ;
  
  /**
   * an condition to used to wait all the responses of the last
   * flooding.
   */
  omni_condition* allResponseReceived ;

  /**
   * Must be called when a new response is added. This methods checks
   * if all responses arrived and wake up the method floodNextStep.
   */
  void addNewResponse() ;

  /**
   * gets an access right to the request
   */
  void getAccess() const ;

public :

  /**
   * creates an undefined FloodRequest.
   */
  FloodRequest() ;

  /**
   * creates a new FloodRequest. It is a copy of the flood request
   * given in argument.
   *
   * @param request the flood request which is cloned.
   */
  FloodRequest(const FloodRequest & request) ;

  /**
   * creates a new defined FloodRequest.
   *
   * @param predecessor the ior of the master agent which are the
   * predecessor of this agent in the flooding graph.
   *
   * @param owner The owner is the MasterAgent_impl which are created
   * the instance of the current object.
   *
   * @param request is the description of the problem. The flood
   * request searches a SeD which can resolve this problem.
   *
   * @param knownMAs a list of all neighboring MAs.
   */
  FloodRequest(const MasterAgentImpl::MADescription & predecessor,
	       const MasterAgentImpl::MADescription & owner,
	       const corba_request_t& request,
	       MasterAgentImpl::MAList& knownMAs) ;

  /**
   * destroys the current flood request. (thread safe)
   */
  ~FloodRequest() ;

  /**
   * release the access right to the request
   */
  void releaseAccess() const ;

  /**
   * returns true if the FloodRequest is defined, false if not.
   */
  inline bool defined() { return _defined ; }

  /**
   * The current flood request become a copy of request. (thread safe)
   *
   * @param request the flood request which is copied.
   */
  FloodRequest & operator=(const FloodRequest & request) ;

  /**
   * Returns true if all the area is flooded. In this case no other MA
   * can be contacted. The FloodRequest must be defined. (thread safe)
   */
  bool flooded() ;

  /**
   * Flood all the neighborhood node of the MA graph. Wait until all
   * responses arrived. After, it sends the union of all the decision
   * taken and returnes it to its predecessor if it exists. The
   * FloodRequest must be defined. (thread safe)
   */
  void floodNextStep() ;

  /**
   * Return the union of all the decision token by the new flooded
   * agents. All the decision returned is removed from the decision
   * list and cannot be get another time. The FloodRequest must be
   * defined. (thread safe)
   */
  corba_response_t getDecision() ;

  /**
   * returns the identifier of the request
   */
  RequestID getId() ;

  /**
   * adds a new response to the flood request. The response says that
   * a neighborhood agent didn't find a server which resolve the
   * request. (thread safe)
   */
  void addResponseNotFound() ;

  /**
   * adds a new response to the flood request. The response says that
   * the area of the senderId is completely flooded and it must not be
   * contacted again until a decision is found.
   *
   * @param senderId the identifier of the agent which give access to
   * the flooded area.
   */
  void addResponseFloodedArea(KeyString senderId) ;

  /**
   * adds a new response to the flood request. The response says that
   * the neighbor was already contacted by another MA and it must not be
   * contacted again until a decision is found.
   *
   * @param senderId the identifier of the agent which give access to
   * the neighbor.
   */
  void addResponseAlreadyContacted(KeyString senderId) ;

  /**
   * Stop the flooding and erase it. The FloodRequest must be
   * defined. The FloodRequest become undefined. (thread safe)
   */
  void stopFlooding() ;

  /**
   * adds a new response to the flood request. The response says that
   * some SeDs that can resolve the problem was found. (thread safe)
   *
   * @param decision the list of the founded SeDs.
   */
  void addResponseServiceFound(const corba_response_t& decision) ;

} ; // FloodRequest

#endif // HAVE_MULTI_MA

#endif // _FLOOD_REQUEST_HH_
