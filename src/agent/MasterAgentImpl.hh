/****************************************************************************/
/* $Id$ */
/* DIET master agent implementation header                                  */
/*                                                                          */
/*  Author(s):                                                              */
/*    - Philippe COMBES (Philippe.Combes@ens-lyon.fr)                       */
/*    - Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)                   */
/*    - Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)             */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/*
 * $Log$
 * Revision 1.1  2003/04/10 13:00:55  pcombes
 * Replace MasterAgent_impl.hh. Apply CS. Update submit profile.
 *
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
  run(char* configFileName, char* parentName = NULL);
  
  /** Problem submission : remotely called by client. */
  corba_response_t*
  submit(const corba_pb_desc_t& pb_profile, CORBA::ULong maxServers);

#if HAVE_MULTI_MA
  virtual CORBA::Long
  handShake(MasterAgent_ptr me, const char* myName);
  void
  updateRefs();
#endif // HAVE_MULTI_MA

private :
  /** ID of next incoming request. */
  Counter reqIDCounter;

#if HAVE_MULTI_MA
  /* Known MAs : */
  typedef NodeDescription<MasterAgent_ptr, MasterAgent_var> MADescription;
  typedef LinkedList<MADescription> MAList;
  typedef LinkedList<const char*> StrList;
  MAList knownMAs;
#endif // HAVE_MULTI_MA

  /**
   * Parse the configration file \c configFileName to fill in this MA fields.
   */
  int
  parseConfigFile(char* configFileName, char* parentName = NULL);

}; // MasterAgentImpl

#endif // _MASTERAGENTIMPL_HH_
