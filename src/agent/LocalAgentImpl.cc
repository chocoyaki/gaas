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
 * Revision 1.1  2003/04/10 13:02:06  pcombes
 * Replace LocalAgent_impl.cc. Apply CS. Use ChildID, Parsers, TRACE_LEVEL.
 *
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
LocalAgentImpl::run(char* configFileName, char* parentName)
{
  char parent_name[257];
  int res = this->AgentImpl::run(configFileName, parent_name);

  if (res)
    return res;
  
  this->parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getAgentReference(parent_name)));
  if (CORBA::is_nil(this->parent)) {
    cerr << "Cannot locate agent " << parent_name << ".\n";
    return 1;
  }

  if (TRACE_LEVEL >= TRACE_MAIN_STEPS)
    cout << "\nLocal Agent started.\n\n";

  return 0;
} // run(char *config_file_name)


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
    SeqCorbaProfileDesc_t* profiles;

    /* Update local service table first */
    this->AgentImpl::addServices(myID, services);
    /* Then propagate the complete service table to the parent */
    profiles = this->SrvT->getProfiles();
    this->childID =
      this->parent->agentSubscribe(_this(), this->localHostName, *profiles);
    delete profiles;
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



/****************************************************************************/
/* Private methods                                                          */
/****************************************************************************/


int
LocalAgentImpl::parseConfigFile(char* configFileName, char* parentName)
{
  int res =
    Parsers::beginParsing(configFileName)
    || Parsers::parseName(myName)
    || Parsers::parseName(parentName)
    || Parsers::parseTraceLevel()//(&TRACE_LEVEL)
    || Parsers::parseFASTEntries(&this->ldapUse,   this->ldapHost,
				 &this->ldapPort,  this->ldapMask,
				 &this->nwsUse,    this->nwsNSHost,
				 &this->nwsNSPort, this->nwsForecasterHost,
				 &this->nwsForecasterPort);

  if (!res && TRACE_LEVEL >= TRACE_STRUCTURES) {
    cout << "TRACE_LEVEL = "         << TRACE_LEVEL             << endl
	 << "LDAP_USE = "            << this->ldapUse           << endl
	 << "LDAP_HOST = "           << this->ldapHost          << " | "
	 << "LDAP_PORT = "           << this->ldapPort          << " | "
	 << "LDAP_MASK = "           << this->ldapMask          << endl
	 << "NWS_USE = "             << this->nwsUse            << endl
	 << "NWS_NAMESERVER_HOST = " << this->nwsNSHost         << " | "
	 << "NWS_NAMESERVER_PORT = " << this->nwsNSPort         << endl
	 << "NWS_FORECASTER_HOST = " << this->nwsForecasterHost << " | "
	 << "NWS_FORECASTER_PORT = " << this->nwsForecasterPort << endl;
  }

  Parsers::endParsing();
      
  return res;
}
