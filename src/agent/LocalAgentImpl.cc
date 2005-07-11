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
 * Revision 1.13  2005/07/11 16:31:56  hdail
 * Corrected bug wherein the MA would stop completely if a client requested
 * a service that didn't exist; this bug was introduced recently, so there should
 * be no reports of it from outside users.
 *
 * Revision 1.12  2005/05/15 15:51:15  alsu
 * to indicate sucess/failure, addServices not returns a value
 *
 * Revision 1.11  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.10  2004/06/09 15:10:38  mcolin
 * add stat_flush in statistics API in order to flush write access to
 * statistic file for agent and sed which never end and can't call
 * stat_finalize
 *
 * Revision 1.9  2004/03/01 18:45:52  rbolze
 * add logservice
 *
 * Revision 1.8  2003/09/28 22:06:11  ecaron
 * Take into account the new API of statistics module
 *
 * Revision 1.7  2003/09/22 21:19:49  pcombes
 * Set all the modules and their interfaces for data persistency.
 *
 * Revision 1.4  2003/07/04 09:47:59  pcombes
 * Use new ERROR, WARNING and TRACE macros.
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
#include "statistics.hh"

#include <iostream>
using namespace std;

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define LA_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "LA::");          \
  TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

LocalAgentImpl::LocalAgentImpl()
{
  this->childID = (ChildID)-1;
  this->parent = Agent::_nil();
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

  char* parentName = (char*)
    Parsers::Results::getParamValue(Parsers::Results::PARENTNAME);
  if (parentName == NULL)
    return 1;

  this->parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
							     parentName)));
  if (CORBA::is_nil(this->parent)) {
     ERROR("cannot locate agent " << parentName, 1);
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
	     "\nLocal Agent " << this->myName << " started.\n");
  fflush(stdout);

  return 0;
} // run()


/**
 * Add \c services into the service table, and attach them to child \c me.
 */
CORBA::Long
LocalAgentImpl::addServices(CORBA::ULong myID,
			    const SeqCorbaProfileDesc_t& services)
{
 LA_TRACE_FUNCTION(myID <<", " << services.length() << " services");

  if (this->childID == -1) { // still not registered ...
    SeqCorbaProfileDesc_t* tmp;

    /* Update local service table first */
    if (this->AgentImpl::addServices(myID, services) != 0) {
      return (-1);
    }
    /* Then propagate the complete service table to the parent */
    tmp = this->SrvT->getProfiles();
    this->childID = this->parent->agentSubscribe(this->_this(), 
						 this->localHostName, *tmp);
    if (this->childID < 0) {
      return (-1);
    }
    delete tmp;
  } else {
    /* First, propagate asynchronously the new services to parent */
    this->parent->addServices(this->childID, services);
    /* Then update local service table */
    if (this->AgentImpl::addServices(myID, services) != 0) {
      return (-1);
    }
  }

  return (0);
} // addServices((CORBA::ULong myID, ...)



/** Get a request from the parent */
void
LocalAgentImpl::getRequest(const corba_request_t& req)
{
  if (dietLogComponent != NULL) {
    dietLogComponent->logAskForSeD(&req);
  }

  Request* currRequest = new Request(&req);

  LA_TRACE_FUNCTION(req.reqID << ", " << req.pb.path);

  /* Initialize statistics module */
  stat_init();
  stat_in(this->myName,"getRequest");

  corba_response_t& resp = *(this->findServer(currRequest, 0));
  resp.myID = this->childID;

  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&req, &resp);
  }

  /* The agent is an LA, the response must be sent to the parent */
  this->parent->getResponse(resp);

  this->reqList[req.reqID] = NULL;
  delete currRequest;
  delete &resp;

  stat_out(this->myName,"getRequest");
  stat_flush();
} // getRequest(const corba_request_t& req)

