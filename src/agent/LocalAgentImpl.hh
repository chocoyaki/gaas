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
 * Revision 1.6  2008/11/18 10:15:23  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.5  2005/05/15 15:51:15  alsu
 * to indicate sucess/failure, addServices not returns a value
 *
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
  virtual CORBA::Long
  addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services);

#ifdef HAVE_DAGDA
  /** Remove services into the service table for a given child */
  virtual CORBA::Long
  serverRemoveService(CORBA::ULong childID, const corba_profile_desc_t& profile);
#endif


private:

  /** Reference of the parent */
  Agent_var parent;
  /** ID of this agent amongst the children of its parent */
  ChildID childID;


}; // LocalAgentImpl

#endif // _LOCALAGENTIMPL_HH_
