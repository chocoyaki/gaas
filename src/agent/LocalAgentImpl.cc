/****************************************************************************/
/* DIET local agent implementation source code                              */
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
 * Revision 1.3  2003/06/23 13:41:20  pcombes
 * Print my name at start time
 *
 * Revision 1.2  2003/05/10 08:53:34  pcombes
 * New format for configuration files, new Parsers.
 *
 * Revision 1.1  2003/04/10 13:02:06  pcombes
 * Replace LocalAgent_impl.cc. Apply CS. Use ChildID, Parsers, TRACE_LEVEL.
 ****************************************************************************/

#include "LocalAgentImpl.hh"
#include "debug.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "Request.hh"

#include <iostream>
using namespace std;


extern unsigned int TRACE_LEVEL;


LocalAgentImpl::LocalAgentImpl()
{
  this->parent = NULL;
}

/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
LocalAgentImpl::run()
{
  int res = this->AgentImpl::run();

  if (res)
    return res;

  char* parent_name = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (parent_name == NULL)
    return 1;

  this->parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getAgentReference(parent_name)));
  if (CORBA::is_nil(this->parent)) {
    cerr << "Cannot locate agent " << parent_name << ".\n";
    return 1;
  }

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "\nLocal Agent " << this->myName << " started.\n\n";

  return 0;
} // run()


/**
 * Add \c services into the service table, and attach them to child \c me.
 */
void
LocalAgentImpl::addServices(CORBA::ULong myID,
			    const SeqCorbaProfileDesc_t& services)
{
  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "LA::addServices(" << myID <<", "
	 << services.length() << " services)\n";

  if (this->childID == -1) { // still not registered ...
    SeqCorbaProfileDesc_t* tmp;

    /* Update local service table first */
    this->AgentImpl::addServices(myID, services);
    /* Then propagate the complete service table to the parent */
    tmp = this->SrvT->getProfiles();
    this->childID = this->parent->agentSubscribe(this->_this(),
						 this->localHostName, *tmp);
    delete tmp;
  } else {
    /* First, propagate asynchronously the new services to parent */
    this->parent->addServices(this->childID, services);
    /* Then update local service table */
    this->AgentImpl::addServices(myID, services);
  }
} // addServices((CORBA::ULong myID, ...)



/** Get a request from the parent */
void
LocalAgentImpl::getRequest(const corba_request_t& req)
{
  Request* currRequest = new Request(&req);

  if (TRACE_LEVEL >= TRACE_ALL_STEPS)
    cout << "LA::getRequest(" << req.reqID << ", " << req.pb.path << ")\n";

  corba_response_t& resp = *(this->findServer(currRequest, 0));

  /* The agent is an LA, the response must be sent to the parent */
  this->parent->getResponse(resp);

  this->reqList[req.reqID] = NULL;
  delete currRequest;
  delete &resp;

} // getRequest(const corba_request_t& req)

