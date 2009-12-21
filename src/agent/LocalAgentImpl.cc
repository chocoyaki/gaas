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

      /* Log */
      if (dietLogComponent != NULL)
      	dietLogComponent->logDisconnect();
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught (" << tc->name() << ") while unsubscribing to "
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
  Agent_var parentTmp =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
							     parentName)));
  if (CORBA::is_nil(parentTmp)) {
    if (CORBA::is_nil(this->parent)) {
      WARNING("cannot locate agent " << parentName << ", will now wait");
    } else {
      WARNING("cannot locate agent " << parentName << ", won't change current parent");
    }
    return 1;
  }

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
    } catch (CORBA::Exception& e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING("exception caught (" << tc->name() << ") while unsubscribing to "
	      << "parent: either the latter is down, "
	      << "or there is a problem with the CORBA name server");
    }
  }

  /* Now we try to subscribe to a new parent */
  this->parent = parentTmp;
    
  try {
    if (profiles->length())
      childID = parent->agentSubscribe(this->_this(), localHostName,
				       *profiles);
    TRACE_TEXT(TRACE_ALL_STEPS, "* Bound myself to parent: " << parentName << std::endl);

    /* Data manager also needs to connect to the new parent */
    this->dataManager->subscribeParent(parentName);

    /* Log */
    if (dietLogComponent != NULL)
      dietLogComponent->logNewParent("LA", parentName);
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
    if (childID != -1)
      this->parent->childUnsubscribe(childID, *profiles);
    this->parent = Agent::_nil();
    childID = -1;

    /* Unsubscribe data manager */
    this->dataManager->unsubscribeParent();
  }

  delete profiles;

  /* Destroy or not the underlying hierarchy and commit suicide */
  return this->AgentImpl::removeElement(recursive);
}

#endif // HAVE_DYNAMICS


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
#ifndef HAVE_DYNAMICS
    return 1;
#else
    WARNING("no parent specified, will now wait");
#endif

  this->parent =
    Agent::_duplicate(Agent::_narrow(ORBMgr::getObjReference(ORBMgr::AGENT,
							     parentName)));
  if (CORBA::is_nil(this->parent)) {
#ifndef HAVE_DYNAMICS
     ERROR("cannot locate agent " << parentName, 1);
#else
    WARNING("cannot locate agent " << parentName << ", will now wait");
#endif // HAVE_DYNAMICS
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
#ifdef HAVE_DYNAMICS
    if (! CORBA::is_nil(this->parent)) {
#endif
      tmp = this->SrvT->getProfiles();
      this->childID = this->parent->agentSubscribe(this->_this(), 
						   this->localHostName, *tmp);
      if (this->childID < 0) {
	return (-1);
      }
      delete tmp;
#ifdef HAVE_DYNAMICS
    }
#endif
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
			     const SeqCorbaProfileDesc_t& services) {
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
#endif // HAVE_DYNAMICS


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
#endif
  if (this->SrvT->lookupService(&profile) == -1)
    return this->parent->childRemoveService(this->childID, profile);
  return 0;
} // childRemoveService(...)
#endif // HAVE_DAGDA


/** Get a request from the parent */
void
LocalAgentImpl::getRequest(const corba_request_t& req)
{
#ifdef HAVE_DYNAMICS
  Agent_var parentTmp = this->parent;
#endif
  char statMsg[128];

  if (dietLogComponent != NULL) {
    dietLogComponent->logAskForSeD(&req);
  }

  Request* currRequest = new Request(&req);

  LA_TRACE_FUNCTION(req.reqID << ", " << req.pb.path);

  /* Initialize statistics module */
  stat_init();
  sprintf(statMsg, "getRequest %ld", (unsigned long) req.reqID);
  stat_in(this->myName,statMsg);

  corba_response_t& resp = *(this->findServer(currRequest, req.max_srv));
  resp.myID = this->childID;

  if (dietLogComponent != NULL) {
    dietLogComponent->logSedChosen(&req, &resp);
  }

  /* The agent is an LA, the response must be sent to the parent */
#ifndef HAVE_DYNAMICS
  this->parent->getResponse(resp);
#else
  parentTmp->getResponse(resp);
#endif // HAVE_DYNAMICS

  this->reqList[req.reqID] = NULL;
  delete currRequest;
  delete &resp;

  stat_out(this->myName,statMsg);
  stat_flush();
} // getRequest(const corba_request_t& req)
