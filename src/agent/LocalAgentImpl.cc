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
 * Revision 1.29  2011/02/24 11:55:46  bdepardo
 * Use the new CONFIG_XXX macros.
 *
 * Revision 1.28  2011/02/15 16:21:50  bdepardo
 * Added a new method: removeElementClean
 *
 * Revision 1.27  2011/02/09 17:16:42  bdepardo
 * Fixed problem during CVS merge
 *
 * Revision 1.26  2011/02/09 15:09:55  hguemar
 * configuration backend changed again: more CONFIG_XXX
 *
 * Revision 1.25  2011/02/08 16:53:51  bdepardo
 * Fixed dynamics. They didn't work anymore
 *
 * Revision 1.24  2011/02/04 15:20:48  hguemar
 * fixes to new configuration parser
 * some cleaning
 *
 * Revision 1.23  2010/12/17 16:30:08  bdepardo
 * searchData shouldn't be between #ifdef HAVE_DYNAMICS
 *
 * Revision 1.22  2010/12/17 09:47:59  kcoulomb
 * * Set diet to use the new log with forwarders
 * * Fix a CoRI problem
 * * Add library version remove DTM flag from ccmake because deprecated
 *
 * Revision 1.21  2010/11/24 15:18:08  bdepardo
 * searchData is now available on all agents. SeDs are now able to retrieve
 * a DAGDA data from an alias specified by a client.
 * Currently a SeD cannot declare an alias.
 *
 * Revision 1.20  2010/11/02 05:54:30  bdepardo
 * Removed ugly couts
 *
 * Revision 1.19  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.18  2010/03/03 10:19:03  bdepardo
 * Changed \n into endl
 *
 * Revision 1.17  2009/12/21 14:17:26  bdepardo
 * Write request number in getRequest statistics
 *
 * Revision 1.16  2009/10/26 09:17:37  bdepardo
 * Added methods for dynamic hierarchy management:
 * - bindParent(const char * parentName)
 * - disconnect()
 * - removeElement(bool recursive)
 * Renamed serverRemoveService(...) into childRemoveService(...)
 *
 * Revision 1.15  2008/11/18 10:15:22  bdepardo
 * - Added the possibility to dynamically create and destroy a service
 *   (even if the SeD is already started). An example is available.
 *   This feature only works with DAGDA.
 * - Added policy commands for CMake 2.6
 * - Added the name of the service in the profile. It was only present in
 *   the profile description, but not in the profile. Currently, the name is
 *   copied in each solve function, but this should certainly be moved
 *   somewhere else.
 *
 * Revision 1.14  2005/12/20 15:42:05  rbolze
 * the list of SeD return by LA do not exceed the max_srv specified in the client's request
 *
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
#include "configuration.hh"
#include "debug.hh"
#include "ORBMgr.hh"
#include "Request.hh"
#include "statistics.hh"

#include <iostream>
using namespace std;

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define LA_TRACE_FUNCTION(formatted_text)		\
    TRACE_TEXT(TRACE_ALL_STEPS, "LA::");		\
    TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

LocalAgentImpl::LocalAgentImpl()
{
    this->childID = (ChildID)-1;
    this->parent = Agent::_nil();
}

#ifdef HAVE_DYNAMICS
/* Method to disconnect from the parent */
CORBA::Long
LocalAgentImpl::disconnect() {
  long rv = 0;
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1)
        this->parent->childUnsubscribe(childID, *profiles);
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();

#ifdef USE_LOG_SERVICE
      /* Log */
      if (dietLogComponent)
        dietLogComponent->logDisconnect();
#endif /* USE_LOG_SERVICE */
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught (" << tc->name()
              << ") while unsubscribing to "
              << "parent: either the latter is down, "
              << "or there is a problem with the CORBA name server");
      rv = 1;
    }
  }

  delete profiles;
  return rv;
}

/* Method to dynamically change the parent of the SeD */
CORBA::Long
LocalAgentImpl::bindParent(const char * parentName) {
  long rv = 0;
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Check that the parent isn't itself */
  if (! strcmp(parentName, this->myName)) {
    WARNING("given parent name is the same as LA name. Won't try to connect");
    return 1;
  }

  /* Does the new parent exists? */
  Agent_var parentTmp;
  try {
    parentTmp =
      ORBMgr::getMgr()->resolve<Agent, Agent_var>(AGENTCTXT, parentName);
  } catch (...) {
    parentTmp = Agent::_nil();
  }
  if (CORBA::is_nil(parentTmp)) {
    if (CORBA::is_nil(this->parent)) {
      WARNING("cannot locate agent "
              << parentName
              << ", will now wait");
    } else {
      WARNING("cannot locate agent "
              << parentName
              << ", won't change current parent");
    }
    return 1;
  }

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1) {
        this->parent->childUnsubscribe(childID, *profiles);
      }
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught (" << tc->name()
              << ") while unsubscribing to "
              << "parent: either the latter is down, "
              << "or there is a problem with the CORBA name server");
    }
  }

  /* Now we try to subscribe to a new parent */
  this->parent = parentTmp;

  try {
    if (profiles->length()) {
      childID = parent->agentSubscribe(myName, localHostName,
					     *profiles);
    }
    TRACE_TEXT(TRACE_ALL_STEPS, "* Bound myself to parent: "
               << parentName << std::endl);

    /* Data manager also needs to connect to the new parent */
    this->dataManager->subscribeParent(parentName);

#ifdef USE_LOG_SERVICE
    /* Log */
    if (dietLogComponent) {
      dietLogComponent->logNewParent("LA", parentName);
    }
#endif /* USE_LOG_SERVICE */
  } catch (CORBA::Exception& e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    WARNING("exception caught (" << tc->name() << ") while subscribing to "
            << parentName << ": either the latter is down, "
            << "or there is a problem with the CORBA name server");
    rv = 1;
  }

  delete profiles;
  return rv;
}


CORBA::Long
LocalAgentImpl::removeElement(bool recursive) {
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    /* Unsubscribe from parent */
    if (childID != -1) {
      try {
        this->parent->childUnsubscribe(childID, *profiles);
      } catch (...) {
        // TODO
      }
    }
    this->parent = Agent::_nil();
    childID = -1;

    /* Unsubscribe data manager */
    try {
      this->dataManager->unsubscribeParent();
    } catch (...) {
      // TODO
    }
  }

  delete profiles;

  /* Destroy or not the underlying hierarchy and commit suicide */
  return this->AgentImpl::removeElement(recursive);
}

void
LocalAgentImpl::removeElementClean(bool recursive) {
  SeqCorbaProfileDesc_t* profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (! CORBA::is_nil(this->parent)) {
    /* Unsubscribe from parent */
    if (childID != -1) {
      try {
        this->parent->childUnsubscribe(childID, *profiles);
      } catch (...) {
        // TODO
      }
    }
    this->parent = Agent::_nil();
    childID = -1;

    /* Unsubscribe data manager */
    try {
      this->dataManager->unsubscribeParent();
    } catch (...) {
      // TODO
    }
  }

  delete profiles;

  return this->AgentImpl::removeElementClean(recursive);
}
#endif /* HAVE_DYNAMICS */


#if HAVE_DAGDA
SeqString*
LocalAgentImpl::searchData(const char* request)
{
    return this->parent->searchData(request);
}
#endif /* HAVE_DAGDA */


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
LocalAgentImpl::run()
{
    int res = this->AgentImpl::run();

    if (res)
	return res;

    std::string parentName;
    if (!CONFIG_STRING(diet::PARENTNAME, parentName))
#ifndef HAVE_DYNAMICS
    return 1;
#else /* HAVE_DYNAMICS */
  WARNING("no parent specified, will now wait");
#endif /* HAVE_DYNAMICS */

  try {
    this->parent =
      ORBMgr::getMgr()->resolve<Agent, Agent_ptr>(AGENTCTXT, parentName.c_str());
  } catch (...) {
    parent = Agent::_nil();
  }
  if (CORBA::is_nil(this->parent)) {
#ifndef HAVE_DYNAMICS
    ERROR("cannot locate agent " << parentName, 1);
#else /* HAVE_DYNAMICS */
    WARNING("cannot locate agent " << parentName << ", will now wait");
#endif /* HAVE_DYNAMICS */
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
             endl << "Local Agent " << this->myName << " started." << endl);
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
#ifdef HAVE_DYNAMICS
	if (! CORBA::is_nil(this->parent)) {
#endif /* HAVE_DYNAMICS */
	    tmp = this->SrvT->getProfiles();
	    this->childID =
		this->parent->agentSubscribe(myName,
					     this->localHostName,
					     *tmp);
	    if (this->childID < 0) {
		return (-1);
	    }
	    delete tmp;
#ifdef HAVE_DYNAMICS
	}
#endif /* HAVE_DYNAMICS */
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


#ifdef HAVE_DYNAMICS
/**
 * Unsubscribe a child. Remotely called by a SeD or another LA.
 */
CORBA::Long
LocalAgentImpl::childUnsubscribe(CORBA::ULong childID,
				 const SeqCorbaProfileDesc_t& services)
{
    if (this->AgentImpl::childUnsubscribe(childID, services) != 0) {
	return (-1);
    }
    if (! CORBA::is_nil(this->parent))
	for (size_t i = 0; i < services.length(); i++) {
	    if (this->SrvT->lookupService(&(services[i])) == -1)
		this->parent->childRemoveService(this->childID, services[i]);
	}

    return 0;
}
#endif /* HAVE_DYNAMICS */


#ifdef HAVE_DAGDA
/**
 * Remove \c services from the service table, and inform upper hierarchy.
 */
CORBA::Long
LocalAgentImpl::childRemoveService(CORBA::ULong childID,
				   const corba_profile_desc_t& profile)
{
    LA_TRACE_FUNCTION(childID);

    /* Update local service table first */
    if (this->AgentImpl::childRemoveService(childID, profile) != 0) {
	return (-1);
    }
    /* Then propagate the complete service table to the parent */
#ifdef HAVE_DYNAMICS
    if (! CORBA::is_nil(this->parent))
#endif /* HAVE_DYNAMICS */
	if (this->SrvT->lookupService(&profile) == -1)
	    return this->parent->childRemoveService(this->childID, profile);
    return 0;
} // childRemoveService(...)
#endif /* HAVE_DAGDA */


/** Get a request from the parent */
void
LocalAgentImpl::getRequest(const corba_request_t& req)
{
#ifdef HAVE_DYNAMICS
    Agent_var parentTmp = this->parent;
#endif /* HAVE_DYNAMICS */
    char statMsg[128];

#ifdef USE_LOG_SERVICE
    if (dietLogComponent) {
	dietLogComponent->logAskForSeD(&req);
    }
#endif /* USE_LOG_SERVICE */

    Request* currRequest = new Request(&req);

    LA_TRACE_FUNCTION(req.reqID << ", " << req.pb.path);

    /* Initialize statistics module */
    stat_init();
    sprintf(statMsg, "getRequest %ld", (unsigned long) req.reqID);
    stat_in(this->myName,statMsg);

    corba_response_t& resp = *(this->findServer(currRequest, req.max_srv));
    resp.myID = this->childID;

#ifdef USE_LOG_SERVICE
    if (dietLogComponent) {
	dietLogComponent->logSedChosen(&req, &resp);
    }
#endif /* USE_LOG_SERVICE */

    /* The agent is an LA, the response must be sent to the parent */
#ifndef HAVE_DYNAMICS
    this->parent->getResponse(resp);
#else /* HAVE_DYNAMICS */
    parentTmp->getResponse(resp);
#endif /* HAVE_DYNAMICS */

    this->reqList[req.reqID] = 0;
    delete currRequest;
    delete &resp;

    stat_out(this->myName,statMsg);
    stat_flush();
} // getRequest(const corba_request_t& req)

LocalAgentFwdrImpl::LocalAgentFwdrImpl(Forwarder_ptr fwdr, const char* objName)
{
    this->forwarder = Forwarder::_duplicate(fwdr);
    this->objName = CORBA::string_dup(objName);
}

CORBA::Long
LocalAgentFwdrImpl::agentSubscribe(const char* me, const char* hostName,
				   const SeqCorbaProfileDesc_t& services)
{
    return forwarder->agentSubscribe(me, hostName, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::serverSubscribe(const char* me, const char* hostName,
				    const SeqCorbaProfileDesc_t& services)
{
    return forwarder->serverSubscribe(me, hostName, services, objName);
}

#ifdef HAVE_DYNAMICS
CORBA::Long
LocalAgentFwdrImpl::childUnsubscribe(CORBA::ULong childID,
				     const SeqCorbaProfileDesc_t& services)
{
    return forwarder->childUnsubscribe(childID, services, objName);
}

CORBA::Long LocalAgentFwdrImpl::bindParent(const char * parentName)
{
    return forwarder->bindParent(parentName, objName);
}

CORBA::Long LocalAgentFwdrImpl::disconnect()
{
    return forwarder->disconnect(objName);
}

CORBA::Long LocalAgentFwdrImpl::removeElement(bool recursive)
{
    return forwarder->removeElement(recursive, objName);
}
#endif /* HAVE_DYNAMICS */

#if HAVE_DAGDA
SeqString*
LocalAgentFwdrImpl::searchData(const char* request)
{
    return forwarder->searchData(request, objName);
}
#endif

void LocalAgentFwdrImpl::getRequest(const corba_request_t& req)
{
    forwarder->getRequest(req, objName);
}

CORBA::Long
LocalAgentFwdrImpl::addServices(CORBA::ULong myID,
				const SeqCorbaProfileDesc_t& services)
{
    return forwarder->addServices(myID, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::childRemoveService(CORBA::ULong childID,
				       const corba_profile_desc_t& profile)
{
    return forwarder->childRemoveService(childID, profile, objName);
}

#ifdef HAVE_DAGDA
char* LocalAgentFwdrImpl::getDataManager() {
    return forwarder->getDataManager(objName);
}
#endif /* HAVE_DAGDA */

void LocalAgentFwdrImpl::getResponse(const corba_response_t& resp)
{
    forwarder->getResponse(resp, objName);
}

CORBA::Long LocalAgentFwdrImpl::ping()
{
    return forwarder->ping(objName);
}

char* LocalAgentFwdrImpl::getHostname()
{
    return forwarder->getHostname(objName);
}
