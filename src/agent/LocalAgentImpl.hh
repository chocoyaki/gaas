/****************************************************************************/
/* DIET local agent implementation header                                   */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2003/09/22 21:19:49  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:01:51  pcombes
 * Replace LocalAgent_impl.hh. Apply CS. Remove createData.
 ****************************************************************************/


#ifndef _LOCALAGENTIMPL_HH_
#define _LOCALAGENTIMPL_HH_

#include "Agent.hh"
#include "AgentImpl.hh"

class LocalAgentImpl : public POA_LocalAgent, public AgentImpl
{

public :

  LocalAgentImpl();
  ~LocalAgentImpl() {};

  /** Force call for POA_LocalAgent::_this. */
  inline LocalAgent_ptr
  _this()
  {
    return this->POA_LocalAgent::_this();
  };

  /** Launch this agent (initialization + registration in the hierarchy). */
  int
  run();

  /** Get a request from the parent */
  virtual void
  getRequest(const corba_request_t& req);

  /**
   * Add \c services into the service table, and attach them to child \c me.
   * Then, propagate information to the parent
   */
  virtual void
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);


private:

  /** Reference of the parent */
  Agent_var parent;
  /** ID of this agent amongst the children of its parent */
  ChildID childID;


}; // LocalAgentImpl

#endif // _LOCALAGENTIMPL_HH_
