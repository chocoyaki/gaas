/**
 * @file  LocalAgentImpl.cc
 *
 * @brief  DIET local agent implementation source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENCE|
 */


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

#define LA_TRACE_FUNCTION(formatted_text)               \
  TRACE_TEXT(TRACE_ALL_STEPS, "LA::");                  \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)

LocalAgentImpl::LocalAgentImpl() {
  this->childID = (ChildID) - 1;
  this->parent = Agent::_nil();
}

/* Method to disconnect from the parent */
CORBA::Long
LocalAgentImpl::disconnect() {
  long rv = 0;
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1) {
        this->parent->childUnsubscribe(childID, *profiles);
      }
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();

#ifdef USE_LOG_SERVICE
      /* Log */
      if (dietLogComponent) {
        dietLogComponent->logDisconnect();
      }
#endif /* USE_LOG_SERVICE */
    } catch (CORBA::Exception &e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING(
        "exception caught (" << tc->name()
                             << ") while unsubscribing to "
                             << "parent: either the latter is down, "
                             <<
        "or there is a problem with the CORBA name server");
      rv = 1;
    }
  }

  delete profiles;
  return rv;
} // disconnect

/* Method to dynamically change the parent of the SeD */
CORBA::Long
LocalAgentImpl::bindParent(const char *parentName) {
  long rv = 0;
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Check that the parent isn't itself */
  if (!strcmp(parentName, this->myName)) {
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
  if (!CORBA::is_nil(this->parent)) {
    try {
      /* Unsubscribe from parent */
      if (childID != -1) {
        this->parent->childUnsubscribe(childID, *profiles);
      }
      this->parent = Agent::_nil();
      childID = -1;

      /* Unsubscribe data manager */
      this->dataManager->unsubscribeParent();
    } catch (CORBA::Exception &e) {
      CORBA::Any tmp;
      tmp <<= e;
      CORBA::TypeCode_var tc = tmp.type();
      WARNING(
        "exception caught (" << tc->name()
                             << ") while unsubscribing to "
                             << "parent: either the latter is down, "
                             <<
        "or there is a problem with the CORBA name server");
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
  } catch (CORBA::Exception &e) {
    CORBA::Any tmp;
    tmp <<= e;
    CORBA::TypeCode_var tc = tmp.type();
    WARNING(
      "exception caught (" << tc->name() << ") while subscribing to "
                           << parentName <<
      ": either the latter is down, "
                           <<
      "or there is a problem with the CORBA name server");
    rv = 1;
  }

  delete profiles;
  return rv;
} // bindParent


CORBA::Long
LocalAgentImpl::removeElement(bool recursive) {
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
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
} // removeElement

void
LocalAgentImpl::removeElementClean(bool recursive) {
  SeqCorbaProfileDesc_t *profiles(NULL);
  profiles = SrvT->getProfiles();

  /* Do we already have a parent?
   * If yes, we need to unsubscribe.
   */
  if (!CORBA::is_nil(this->parent)) {
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
} // removeElementClean


SeqString *
LocalAgentImpl::searchData(const char *request) {
  return this->parent->searchData(request);
}

/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
LocalAgentImpl::run() {
  int res = this->AgentImpl::run();

  if (res) {
    return res;
  }

  std::string parentName;
  if (!CONFIG_STRING(diet::PARENTNAME, parentName)) {
    WARNING("no parent specified, will now wait");
  }

  try {
    this->parent =
      ORBMgr::getMgr()->resolve<Agent, Agent_ptr>(AGENTCTXT, parentName.c_str());
  } catch (...) {
    parent = Agent::_nil();
  }
  if (CORBA::is_nil(this->parent)) {
    WARNING("cannot locate agent " << parentName << ", will now wait");
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
                            const SeqCorbaProfileDesc_t &services) {
  LA_TRACE_FUNCTION(myID << ", " << services.length() << " services");

  if (this->childID == -1) { // still not registered ...
    SeqCorbaProfileDesc_t *tmp;

    /* Update local service table first */
    if (this->AgentImpl::addServices(myID, services) != 0) {
      return (-1);
    }
    /* Then propagate the complete service table to the parent */

    if (!CORBA::is_nil(this->parent)) {
      tmp = this->SrvT->getProfiles();
      this->childID =
        this->parent->agentSubscribe(myName,
                                     this->localHostName,
                                     *tmp);
      if (this->childID < 0) {
        return (-1);
      }
      delete tmp;
    }
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


/**
 * Unsubscribe a child. Remotely called by a SeD or another LA.
 */
CORBA::Long
LocalAgentImpl::childUnsubscribe(CORBA::ULong childID,
                                 const SeqCorbaProfileDesc_t &services) {
  if (this->AgentImpl::childUnsubscribe(childID, services) != 0) {
    return (-1);
  }
  if (!CORBA::is_nil(this->parent)) {
    for (size_t i = 0; i < services.length(); i++) {
      if (this->SrvT->lookupService(&(services[i])) == -1) {
        this->parent->childRemoveService(this->childID, services[i]);
      }
    }
  }

  return 0;
} // childUnsubscribe


/**
 * Remove \c services from the service table, and inform upper hierarchy.
 */
CORBA::Long
LocalAgentImpl::childRemoveService(CORBA::ULong childID,
                                   const corba_profile_desc_t &profile) {
  LA_TRACE_FUNCTION(childID);

  /* Update local service table first */
  if (this->AgentImpl::childRemoveService(childID, profile) != 0) {
    return (-1);
  }
  /* Then propagate the complete service table to the parent */
  if (!CORBA::is_nil(this->parent)) {
    if (this->SrvT->lookupService(&profile) == -1) {
      return this->parent->childRemoveService(this->childID, profile);
    }
  }
  return 0;
} // childRemoveService(...)


/** Get a request from the parent */
void
LocalAgentImpl::getRequest(const corba_request_t &req) {
  Agent_var parentTmp = this->parent;
  char statMsg[128];

#ifdef USE_LOG_SERVICE
  if (dietLogComponent) {
    dietLogComponent->logAskForSeD(&req);
  }
#endif /* USE_LOG_SERVICE */

  Request *currRequest = new Request(&req);

  LA_TRACE_FUNCTION(req.reqID << ", " << req.pb.path);

  /* Initialize statistics module */
  stat_init();
  sprintf(statMsg, "getRequest %ld", (unsigned long) req.reqID);
  stat_in(this->myName, statMsg);

  corba_response_t &resp = *(this->findServer(currRequest, req.max_srv));
  resp.myID = this->childID;

#ifdef USE_LOG_SERVICE
  if (dietLogComponent) {
    dietLogComponent->logSedChosen(&req, &resp);
  }
#endif /* USE_LOG_SERVICE */

  /* The agent is an LA, the response must be sent to the parent */
  parentTmp->getResponse(resp);

  this->reqList[req.reqID] = 0;
  delete currRequest;
  delete &resp;

  stat_out(this->myName, statMsg);
  stat_flush();
} // getRequest(const corba_request_t& req)

LocalAgentFwdrImpl::LocalAgentFwdrImpl(Forwarder_ptr fwdr,
                                       const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long
LocalAgentFwdrImpl::agentSubscribe(const char *me, const char *hostName,
                                   const SeqCorbaProfileDesc_t &services) {
  return forwarder->agentSubscribe(me, hostName, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::serverSubscribe(const char *me, const char *hostName,
                                    const SeqCorbaProfileDesc_t &services) {
  return forwarder->serverSubscribe(me, hostName, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::childUnsubscribe(CORBA::ULong childID,
                                     const SeqCorbaProfileDesc_t &services) {
  return forwarder->childUnsubscribe(childID, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::bindParent(const char *parentName) {
  return forwarder->bindParent(parentName, objName);
}

CORBA::Long
LocalAgentFwdrImpl::disconnect() {
  return forwarder->disconnect(objName);
}

CORBA::Long
LocalAgentFwdrImpl::removeElement(bool recursive) {
  return forwarder->removeElement(recursive, objName);
}

SeqString *
LocalAgentFwdrImpl::searchData(const char *request) {
  return forwarder->searchData(request, objName);
}

void
LocalAgentFwdrImpl::getRequest(const corba_request_t &req) {
  forwarder->getRequest(req, objName);
}

CORBA::Long
LocalAgentFwdrImpl::addServices(CORBA::ULong myID,
                                const SeqCorbaProfileDesc_t &services) {
  return forwarder->addServices(myID, services, objName);
}

CORBA::Long
LocalAgentFwdrImpl::childRemoveService(CORBA::ULong childID,
                                       const corba_profile_desc_t &profile) {
  return forwarder->childRemoveService(childID, profile, objName);
}

char *
LocalAgentFwdrImpl::getDataManager() {
  return forwarder->getDataManager(objName);
}

void
LocalAgentFwdrImpl::getResponse(const corba_response_t &resp) {
  forwarder->getResponse(resp, objName);
}

CORBA::Long
LocalAgentFwdrImpl::ping() {
  return forwarder->ping(objName);
}

char *
LocalAgentFwdrImpl::getHostname() {
  return forwarder->getHostname(objName);
}
