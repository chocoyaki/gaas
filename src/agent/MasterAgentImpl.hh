/****************************************************************************/
/* DIET master agent implementation header                                  */
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
 * Revision 1.6  2004/10/05 07:45:34  hdail
 * Grouped private and public members together.
 *
 * Revision 1.5  2004/10/04 09:40:43  sdahan
 * warning fix :
 *  - debug.cc : change the printf format from %ul to %lu and from %l to %ld
 *  - ReferenceUpdateThread and BindService : The omniORB documentation said that
 *    it's better to create private destructor for the thread subclasses. But
 *    private destructors generate warning, so I set the destructors public.
 *  - CORBA.h and DIET_config.h define the same macros. So I include the CORBA.h
 *    before the DIET_config.h to avoid to define two times the same macros.
 *  - remove the deprecated warning when including iostream.h and set.h
 *
 * Revision 1.4  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.3  2004/02/27 10:25:11  bdelfabr
 * methods for data id creation and  methods to retrieve data descriptor are added
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:00:55  pcombes
 * Replace MasterAgent_impl.hh. Apply CS. Update submit profile.
 ****************************************************************************/

#ifndef _MASTERAGENTIMPL_HH_
#define _MASTERAGENTIMPL_HH_

#include "MasterAgent.hh"
#include "DIET_config.h"
#include "AgentImpl.hh"
#include "LinkedList.hh"
#include "Parsers.hh"

#if HAVE_MULTI_MA
#include "BindService.hh"
#include "KeyString.hh"
#include "ts_container/ts_set.hh"
class FloodRequestsList ;
#endif

class MasterAgentImpl : public POA_MasterAgent, public AgentImpl
{

public :
  MasterAgentImpl();
  virtual
  ~MasterAgentImpl();
  
  /** Force call for POA_MasterAgent::_this. */
  inline MasterAgent_ptr
  _this()
  {
    return this->POA_MasterAgent::_this();
  };
  
  /** Launch this agent (initialization + registration in the hierarchy). */
  int
  run();
  
  /** Problem submission : remotely called by client. */
  corba_response_t*
  submit(const corba_pb_desc_t& pb_profile, CORBA::ULong maxServers);
  
  /** Problem submission. Looking for SeDs that can resolve the
      problem in the local domain. */
  corba_response_t*
  submit_local(const corba_request_t& creq);

  /** get session id */
  virtual  CORBA::Long 
  get_session_num();

  /** returns id of a data */
  virtual  char * 
  get_data_id();
  
  /** returns true if data present, FALSE elsewhere */
  virtual CORBA::ULong  
  dataLookUp(const char* argID);

  /** return the descriptor the the data */
   corba_data_desc_t* 
   get_data_arg(const char* argID);

  /** removes data from the list */
  virtual CORBA::Long 
  diet_free_pdata(const char *argID);

#ifdef HAVE_MULTI_MA
  /** Ask the authorization to create a link with this Master Agent */
  virtual CORBA::Boolean
  handShake(MasterAgent_ptr me, const char* myName);
  /** returns the address of the bind service of the Master Agent */
  virtual char*
  getBindName() ;
  /** Updates all the references to the other Master Agent. It also
      connect to some new Master Agent if there is not enough links. */
  void
  updateRefs();
  virtual void searchService(MasterAgent_ptr predecessor,
			     const char* predecessorId,
			     const corba_request_t& request);
  virtual void stopFlooding(CORBA::Long reqId,
			    const char* senderId);
  virtual void serviceNotFound(CORBA::Long reqId,
			       const char* senderId);
  virtual void newFlood(CORBA::Long reqId,
			const char* senderId);
  virtual void floodedArea(CORBA::Long reqId,
			   const char* senderId);
  virtual void alreadyContacted(CORBA::Long reqId,
				const char* senderId);
  virtual void serviceFound(CORBA::Long reqId,
			    const corba_response_t& decision);
  typedef NodeDescription<MasterAgent, MasterAgent_ptr> MADescription;
  typedef ts_map<KeyString, MADescription> MAList;
#endif // HAVE_MULTI_MA

private :
  /** ID of next incoming request. */
  Counter reqIDCounter;
  Counter num_session;
  Counter num_data;
#ifdef HAVE_MULTI_MA
  typedef ts_set<KeyString> StrList;
  StrList MAIds ;
  MAList knownMAs ;
  int minMAlinks ;
  int maxMAlinks ;
  BindService* bindSrv ;
  CORBA::String_var bindName ;
  FloodRequestsList* floodRequestsList ;
#endif // HAVE_MULTI_MA
  void
  cp_arg_to_pb(corba_data_desc_t& pb, corba_data_desc_t arg_desc);

}; // MasterAgentImpl

#endif // _MASTERAGENTIMPL_HH_
