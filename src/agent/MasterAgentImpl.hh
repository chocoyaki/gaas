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
#include "AgentImpl.hh"
#include "LinkedList.hh"


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

#if HAVE_MULTI_MA
  virtual CORBA::Long
  handShake(MasterAgent_ptr me, const char* myName);
  void
  updateRefs();
#endif // HAVE_MULTI_MA

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


private :
  /** ID of next incoming request. */
  Counter reqIDCounter;
  Counter num_session;
  Counter num_data;
#if HAVE_MULTI_MA
  /* Known MAs : */
  typedef NodeDescription<MasterAgent_ptr, MasterAgent_var> MADescription;
  typedef LinkedList<MADescription> MAList;
  typedef LinkedList<const char*> StrList;
  MAList knownMAs;
#endif // HAVE_MULTI_MA
  void
  cp_arg_to_pb(corba_data_desc_t& pb, corba_data_desc_t arg_desc);

}; // MasterAgentImpl

#endif // _MASTERAGENTIMPL_HH_
