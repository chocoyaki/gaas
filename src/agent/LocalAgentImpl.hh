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
 * Revision 1.1  2003/04/10 13:01:51  pcombes
 * Replace LocalAgent_impl.hh. Apply CS. Remove createData.
 *
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
  run(char* configFileName, char* parentName = NULL);

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

  /** Pointer to the parent */
  Agent_var parent;
  
  /**
   * Parse the configration file \c configFileName to fill in this MA fields.
   */
  int
  parseConfigFile(char* configFileName, char* parentName = NULL);


}; // LocalAgentImpl

#endif // _LOCALAGENTIMPL_HH_
