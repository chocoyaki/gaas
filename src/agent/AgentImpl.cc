/**
 * @file AgentImpl.cc
 *
 * @brief  DIET agent implementation source code
 *
 * @author
 *
 * @section Licence
 *   |LICENCE|
 */


#include "AgentImpl.hh"

#include <cmath>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "OSIndependance.hh"// For gethostname()

#include "common_types.hh"
#include "configuration.hh"
#include "debug.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "statistics.hh"
#include "est_internal.hh"
#include "GlobalSchedulers.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define AGT_TRACE_FUNCTION(formatted_text)              \
  TRACE_TEXT(TRACE_ALL_STEPS, "Agt::");                 \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)

#define MAX_HOSTNAME_LENGTH 256

AgentImpl::AgentImpl() {
  this->childIDCounter = 0;
  this->nbLAChildren = 0;
  this->nbSeDChildren = 0;
  this->SrvT = new ServiceTable();
  this->reqList.clear();
  this->dataManager = 0;
  this->myName = 0;
  this->localHostName = 0;
#ifdef USE_LOG_SERVICE
  this->dietLogComponent = 0;
#endif
}

AgentImpl::~AgentImpl() {
  TRACE_TEXT(TRACE_STRUCTURES, "Remove the Local Agents references...");
  this->LAChildren.clear();
  this->nbLAChildren = 0;
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");
  TRACE_TEXT(TRACE_STRUCTURES, "Remove the SeD references...");
  this->SeDChildren.clear();
  this->nbSeDChildren = 0;
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");

  this->childIDCounter = 0;

  TRACE_TEXT(TRACE_STRUCTURES, "Remove the Services references...");
  /* FIXME : Pb at the destruction of SrvT */
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");
  TRACE_TEXT(TRACE_STRUCTURES, "Remove the Request list...");
  this->reqList.clear();
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");
  ORBMgr::getMgr()->unbind(AGENTCTXT, this->myName);
  ORBMgr::getMgr()->fwdsUnbind(AGENTCTXT, this->myName);

  stat_finalize();
  TRACE_TEXT(TRACE_STRUCTURES, "All Done\n");

  // WTF, nobody cleans its memory ?
  delete[] localHostName;
  delete[] myName;
}


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
AgentImpl::run() {
  /* Set host name */
  this->localHostName = new char[MAX_HOSTNAME_LENGTH];
  std::string host;

  if (CONFIG_STRING(diet::DIETHOSTNAME, host)) {
    strncpy(this->localHostName, host.c_str(), MAX_HOSTNAME_LENGTH - 1);
  } else {
    if (gethostname(this->localHostName, MAX_HOSTNAME_LENGTH)) {
      ERROR_DEBUG("could not get hostname", 1);
    }
  }

  this->localHostName[MAX_HOSTNAME_LENGTH - 1] = '\0';

  /* Bind this agent to its name in the CORBA Naming Service */
  std::string name;
  if (!CONFIG_STRING(diet::NAME, name)) {
    return 1;
  }

  size_t sz = name.length() + 1;
  this->myName = new char[sz];

#ifdef __WIN32__
  _strncpy(this->myName, name.c_str(), sz);
#else
  strncpy(this->myName, name.c_str(), sz);
#endif

  std::string agtType = "MA";
  CONFIG_AGENT(diet::AGENTTYPE, agtType);

  ORBMgr::getMgr()->bind(AGENTCTXT, this->myName, _this(), true);
  if (agtType == "DIET_LOCAL_AGENT" || agtType == "LA") {
    ORBMgr::getMgr()->fwdsBind(LOCALAGENT, this->myName,
                               ORBMgr::getMgr()->getIOR(_this()));
  } else {
    ORBMgr::getMgr()->fwdsBind(MASTERAGENT, this->myName,
                               ORBMgr::getMgr()->getIOR(_this()));
  }

  return 0;
} // run

// Accessors for dataManager.
void
AgentImpl::setDataManager(Dagda_ptr dataManager) {
  this->dataManager = dataManager;
}

char *
AgentImpl::getDataManager() {
  return CORBA::string_dup(dataManager->getID());
}

#ifdef USE_LOG_SERVICE
void
AgentImpl::setDietLogComponent(DietLogComponent *dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif

/****************************************************************************/
/* Subscribing and adding services                                          */
/****************************************************************************/

/**
 * Subscribe an agent as a LA child. Remotely called by an LA.
 */
CORBA::Long
AgentImpl::agentSubscribe(const char *name, const char *hostName,
                          const SeqCorbaProfileDesc_t &services) {
  CORBA::ULong retID = (this->childIDCounter)++;  // thread safe Counter class

  TRACE_TEXT(
    TRACE_MAIN_STEPS, "An agent has registered from << " << hostName
                                                         << ", with "
                                                         << services.length()
                                                         << " services.\n");

  /* the size of the list is childIDCounter+1 (first index is 0) */
  this->LAChildren.resize(this->childIDCounter);

  TRACE_TEXT(TRACE_MAIN_STEPS, "Local agent name: " << name << "\n");
  LocalAgent_var meLA =
    ORBMgr::getMgr()->resolve<LocalAgent, LocalAgent_var>(LOCALAGENT, name);
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Local agent IOR: " << ORBMgr::getMgr()->getIOR(meLA) << "\n");

  this->LAChildren[retID] = LAChild(meLA, hostName);
  (this->nbLAChildren)++;  // thread safe Counter class

  if (this->addServices(retID, services) != 0) {
    return (-1);
  }

  return retID;
} // agentSubscribe


/**
 * Subscribe a server as a SeD child. Remotely called by a SeD.
 */
CORBA::Long
AgentImpl::serverSubscribe(const char *name, const char *hostName,
                           const SeqCorbaProfileDesc_t &services) {
  SeD_ptr me = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, name);
  CORBA::ULong retID;

  TRACE_TEXT(
    TRACE_MAIN_STEPS, "A server has registered from " << hostName
                                                      << ", with " <<
    services.length() << " services.\n");
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "subscribed SeD IOR: " << ORBMgr::getMgr()->getIOR(me) << "\n");

  assert(hostName != NULL);
  retID = (this->childIDCounter)++;  // thread safe

  // FIXME: resize calls default constructor + copy constructor + 1 desctructor
  this->SeDChildren.resize(childIDCounter);
  (this->SeDChildren[retID]).set(me, hostName);
  (this->nbSeDChildren)++;  // thread safe Counter class

  if (this->addServices(retID, services) != 0) {
    return (-1);
  }

  return retID;
} // serverSubscribe


/**
 * Unsubscribe a child. Remotely called by a SeD or an LA.
 */
CORBA::Long
AgentImpl::childUnsubscribe(CORBA::ULong childID,
                            const SeqCorbaProfileDesc_t &services) {
  bool childFound = false;
  TRACE_TEXT(TRACE_ALL_STEPS,
             "Unsubscription request from child " << childID << "\n");
  this->srvTMutex.lock();
  for (size_t i = 0; i < services.length(); i++) {
    this->SrvT->rmChildService(&(services[i]), childID);

    if (TRACE_LEVEL >= TRACE_STRUCTURES) {
      this->SrvT->dump(stdout);
    }
  }
  this->srvTMutex.unlock();

  /* Is the child an agent ? */
  if (childID < static_cast<CORBA::ULong>(LAChildren.size())) {
    LAChild &childDesc = LAChildren[childID];
    if (childDesc.defined()) {
      LAChildren[childID] = LAChild();
      --nbLAChildren;
      childFound = true;
    }
  } else if (!childFound &&
             (childID < static_cast<CORBA::ULong>(SeDChildren.size()))) {
    /* Then it must be a server */
    SeDChild &childDesc = SeDChildren[childID];
    if (childDesc.defined()) {
      SeDChildren[childID] = SeDChild();
      --nbSeDChildren;
    }
  }
  return 0;
} // childUnsubscribe

CORBA::Long
AgentImpl::removeElement(bool recursive) {
  removeElementChildren(recursive);

  /* Send signal to commit suicide */
  return raise(SIGINT);  // Or SIGTERM
}

void
AgentImpl::removeElementChildren(bool recursive) {
  /* Do we need to recursively destroy the underlying hierarchy? */
  if (recursive) {
    unsigned long childID;
    /* Forward to LAs */
    for (childID = 0; childID < LAChildren.size(); ++childID) {
      LAChild &childDesc = LAChildren[childID];
      if (childDesc.defined()) {
        try {
          childDesc->removeElement(recursive);
        } catch (CORBA::COMM_FAILURE &ex) {
          WARNING("COMM_FAILURE when contacting LAChild " << childID);
        } catch (CORBA::TRANSIENT &e) {
          WARNING("TRANSIENT when contacting LAChild " << childID);
        }
      }
    }
    LAChildren.clear();

    /* Forward to SeDs */
    for (childID = 0; childID < SeDChildren.size(); ++childID) {
      SeDChild &childDesc = SeDChildren[childID];
      if (childDesc.defined()) {
        try {
          childDesc->removeElement();
        } catch (CORBA::COMM_FAILURE &ex) {
          WARNING("COMM_FAILURE when contacting SeDChild " << childID);
        } catch (CORBA::TRANSIENT &e) {
          WARNING("TRANSIENT when contacting SeDChild " << childID);
        }
      }
    }
    SeDChildren.clear();
  }
} // removeElementChildren

void
AgentImpl::removeElementClean(bool recursive) {
  removeElementChildren(recursive);
#ifdef USE_LOG_SERVICE
  /* Log */
  if (dietLogComponent != NULL) {
    dietLogComponent->logRemoveElement();
  }
#endif
}


/**
 * Add \c services into the service table, and attach them to child \c me.
 */
CORBA::Long
AgentImpl::addServices(CORBA::ULong myID,
                       const SeqCorbaProfileDesc_t &services) {
  int result;
  AGT_TRACE_FUNCTION(myID << ", " << services.length() << " services");

  this->srvTMutex.lock();
  for (size_t i = 0; i < services.length(); i++) {
    result = this->SrvT->addService(&(services[i]), myID);
    if (result == 0) {
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i
                                              << " added for child " << myID <<
                 ".\n");
    } else if (result == -1) {
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i
                                              << " is a duplicate for child "
                                              << myID << ". Not added.\n");
    } else if (result == -2) {
      this->SrvT->rmChild(myID);
      return (-1);
    } else {
      INTERNAL_ERROR(__FUNCTION__ <<
                     ": unexpected return code from " <<
                     "ServiceTable::addService (" <<
                     result <<
                     ")", -1);
    }
  }
  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    this->SrvT->dump(stdout);
  }
  this->srvTMutex.unlock();

  return (0);
} // addServices


CORBA::Long
AgentImpl::childRemoveService(CORBA::ULong childID,
                              const corba_profile_desc_t &profile) {
  int result;

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "A server is removing a service from its service table, "
             << childID << "\n");

  this->srvTMutex.lock();

  result = this->SrvT->rmChildService(&profile, childID);

  if (TRACE_LEVEL >= TRACE_STRUCTURES) {
    this->SrvT->dump(stdout);
  }
  this->srvTMutex.unlock();

  return (result);
} // childRemoveService


/****************************************************************************/
/* findServer                                                               */
/****************************************************************************/

/**
 * Forward a request, schedule and merge the responses associated.
 * @param req     the request.
 * @param max_srv the maximum number of servers to sort.
 * @return non NULL pointer to a response
 */
corba_response_t *
AgentImpl::findServer(Request *req, size_t max_srv) {
  corba_response_t *resp;
  const corba_request_t &creq = *(req->getRequest());
  char statMsg[128];
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\n**************************************************\n"
             << "Got request " << creq.reqID
             << " on problem " << creq.pb.path << "\n");
#ifdef __WIN32__
  _snprintf(statMsg, 128, "findServer %ld", (unsigned long) creq.reqID);
#else
    snprintf(statMsg, 128, "findServer %ld", (unsigned long) creq.reqID);
#endif

  stat_in(this->myName, statMsg);

  /* Add the new request to the list */
  reqList[creq.reqID] = req;

  /* Find capable children */
  ServiceTable::ServiceReference_t serviceRef;
  srvTMutex.lock();
  serviceRef = SrvT->lookupService(&(creq.pb));

  if (serviceRef == -1) {
    WARNING("no service found for request " << creq.reqID);
    srvTMutex.unlock();

    /* Initialize the response */
    resp = new corba_response_t;
    resp->reqID = creq.reqID;
    resp->servers.length(0);
  } else {
    // then the request must be forwarded
    size_t i;

    int nbChildrenContacted = 0;

#ifndef HAVE_ALT_BATCH

    const ServiceTable::matching_children_t *SrvTmc;
    ServiceTable::matching_children_t* mc;

    SrvTmc = SrvT->getChildren(serviceRef);

    /* Contact the children. The responses array will not be ready until all
       children are contacted. Thus lock the responses mutex now.           */

    req->lock();

    // Copy matching children for sendRequest ...
    mc = new ServiceTable::matching_children_t(*SrvTmc);
    mc->children = new CORBA::ULong[mc->nb_children];
    for (i = 0; i < (size_t) mc->nb_children; i++) {
      mc->children[i] = SrvTmc->children[i];
    }

    srvTMutex.unlock();

    for (i = 0; i < (size_t) mc->nb_children; ++i) {
      AgentImpl::sendRequest(mc->children[i], &creq);
    }

    delete[] mc->children;
    delete mc;

    srvTMutex.lock();
    nbChildrenContacted = SrvTmc->nb_children;
    srvTMutex.unlock();

#else // if  defined HAVE_ALT_BATCH
      /* Need to know children for parallel and/or seq.
         One service can be registered as parallel and as seq on a child.
         It then appears 2 times.
         Search from the profile and concatene results. */

    /* Contact the children. The responses array will not be ready until all
       children are contacted. Thus lock the responses mutex now. */
    ServiceTable::matching_children_t *mc = NULL;
    CORBA::ULong frontier;

    req->lock();
    mc = SrvT->getChildren(&creq.pb, serviceRef, &frontier);

    srvTMutex.unlock();

    nbChildrenContacted = mc->nb_children;
    /* Perform requests */
    for (i = 0; i < (size_t) mc->nb_children; i++) {
      sendRequest(mc->children, i, &creq, &nbChildrenContacted, frontier);
    }

    delete[] mc->children;
    delete mc;
#endif // if not defined HAVE_ALT_BATCH

    /* if no alive server can solve the problem, return */
    if (!nbChildrenContacted) {
      WARNING("no service found for request " << creq.reqID);
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "**************************************************\n");
      req->unlock();

      resp = new corba_response_t;
      resp->reqID = creq.reqID;
      resp->servers.length(0);
      return resp;
    }

    /* Everything is ready, we can now wait for the responses */
    /* (This call implicitly unlocks the responses mutex)     */
    TRACE_TEXT(
      TRACE_ALL_STEPS, "Waiting for " << nbChildrenContacted
                                      << " responses to request " <<
      creq.reqID << "...\n");
    req->waitResponses(nbChildrenContacted);
    req->unlock();

    /* The thread is awakened when all responses are gathered */
    resp = this->aggregate(req, max_srv);

    if (TRACE_LEVEL >= TRACE_STRUCTURES) {
      displayResponseShort(stdout, resp);
    }
  }

  stat_out(this->myName, statMsg);

  return resp;
} // findServer

/****************************************************************************/
/* getResponse                                                              */
/****************************************************************************/

/** Get the response of a child */
void
AgentImpl::getResponse(const corba_response_t &resp) {
  TRACE_TEXT(
    TRACE_MAIN_STEPS, "Got a response from " << resp.myID
                                             << "th child" <<
    " to request " << resp.reqID << "\n");
  char statMsg[128];
#ifdef __WIN32__
  _snprintf(statMsg, 128, "getResponse %ld %ld",
           (unsigned long) resp.reqID, (long) resp.myID);
#else
    snprintf(statMsg, 128, "getResponse %ld %ld",
           (unsigned long) resp.reqID, (long) resp.myID);
#endif
  stat_in(this->myName, statMsg);
  /* The response should be copied in the logs */
  /* Look for the concerned request in the logs */
  Request *req = reqList[resp.reqID];
  if (req) {
    req->lock();
    req->addResponse(&resp);
    req->unlock();
  } else {
    WARNING("response to unknown request");
  }
  stat_out(this->myName, statMsg);
} // getResponse

/**
 * Used to test if this agent is alive.
 */
CORBA::Long
AgentImpl::ping() {
  TRACE_TEXT(TRACE_ALL_STEPS, "ping()\n");
  fflush(stdout);
  return getpid();
}

char *
AgentImpl::getHostname() {
  return CORBA::string_dup(localHostName);
}

#ifndef HAVE_ALT_BATCH
/**
 * Send the request structure \c req to the child whose ID is \c childID.
 */
void
AgentImpl::sendRequest(CORBA::ULong childID, const corba_request_t *req) {
#else
/**
 * Send the request structure \c req to the child whose ID is \c childID.
 * Decremente \c nb_children contacted when error
 * A \c numero_child SeD strictly inf�rior to \c frontier must been submitted
   with a request with a parallel flag equal to 1
 */
void
AgentImpl::sendRequest(CORBA::ULong *children, size_t numero_child,
                       const corba_request_t *req, int *nb_children_contacted,
                       CORBA::ULong frontier) {
#endif
  bool childFound = false;
  typedef size_t comm_failure_t;
#ifdef HAVE_ALT_BATCH

  CORBA::ULong childID = children[numero_child];
#endif
  char statMsg[128];
#ifdef __WIN32__
  _snprintf(statMsg, 128, "sendRequest %ld %ld",
           (unsigned long) req->reqID, (unsigned long) childID);
#else
  snprintf(statMsg, 128, "sendRequest %ld %ld",
           (unsigned long) req->reqID, (unsigned long) childID);
#endif
  AGT_TRACE_FUNCTION(childID << ", " << req->pb.path);

  stat_in(this->myName, statMsg);
  try {
    /* Is the child an agent ? */
    if (childID < static_cast<CORBA::ULong>(LAChildren.size())) {
      LAChild &childDesc = LAChildren[childID];
      if (childDesc.defined()) {
        try {
          try {
            /* checks if the child is alive with a ping */
            childDesc->ping();
            childDesc->getRequest(*req);
          } catch (CORBA::COMM_FAILURE &ex) {
            throw(comm_failure_t) 0;
          } catch (CORBA::TRANSIENT &e) {
            throw(comm_failure_t) 1;
          }
        } catch (comm_failure_t &e) {
          if (e == 0 || e == 1) {
            WARNING(
              "connection problems with LA child " << childID
                                                   <<
              " occured - remove it from known children");
            srvTMutex.lock();
            SrvT->rmChild(childID);
#if defined HAVE_ALT_BATCH
            (*nb_children_contacted)--;
#endif
            if (TRACE_LEVEL >= TRACE_STRUCTURES) {
              SrvT->dump(stdout);
            }
            srvTMutex.unlock();
            LAChildren[childID] = LAChild();
            --nbLAChildren;
          } else {
            throw;
          }
        }
        childFound = true;
      }
    }
    if (!childFound &&
        (childID < static_cast<CORBA::ULong>(SeDChildren.size()))) {
      /* Then it must be a server */
      SeDChild &childDesc = SeDChildren[childID];
      if (childDesc.defined()) {
        try {
          try {
            /* checks if the child is alive with a ping */
            childDesc->ping();
#if defined HAVE_ALT_BATCH
            /* When requesting to a SeD, the request must be set
               to the flag of the registered profile.
               Hence, if the profile wants // or seq services,
               there will be two requests, so two answers with
               the corresponding good flag from SeDs
             */
            corba_request_t *req_copy = new corba_request_t(*req);
            if (numero_child < frontier) {
              req_copy->pb.parallel_flag = 1;  // sequentialjob
            } else {
              req_copy->pb.parallel_flag = 2;  // parallel job
            }
            childDesc->getRequest(*req_copy);
            delete req_copy;
#else
            childDesc->getRequest(*req);
#endif // if defined HAVE_ALT_BATCH
          } catch (CORBA::COMM_FAILURE &ex) {
            throw(comm_failure_t) 0;
          } catch (CORBA::TRANSIENT &e) {
            throw(comm_failure_t) 1;
          }
        } catch (comm_failure_t &e) {
          if (e == 0 || e == 1) {
            WARNING(
              "connection problems with SeD child " << childID
                                                    <<
              " occured - remove it from known children");
            srvTMutex.lock();
            SrvT->rmChild(childID);
#if defined HAVE_ALT_BATCH
            (*nb_children_contacted)--;
#endif
            if (TRACE_LEVEL >= TRACE_STRUCTURES) {
              SrvT->dump(stdout);
            }
            srvTMutex.unlock();
            SeDChildren[childID] = SeDChild();
            --nbSeDChildren;
          } else {
            throw;
          }
        }
        childFound = true;
      }

      if (!childFound) {
        WARNING("trying to send a request to an unknown child");
      }
    }
  } catch (...) {
    WARNING("exception thrown in Agt::" << __FUNCTION__);
  }
  stat_out(this->myName, statMsg);
} // sendRequest

/**
 * Get communication time between this agent and the child \c childID for a
 * data amount of size \c size. The way of the data transfer can be specified
 * with \c to : if (to), from this agent to the child, else from the child to
 * this agent.
 */
inline double
AgentImpl::getCommTime(CORBA::Long childID, unsigned long size, bool to) {
  double time(0.0);
  char *child_name = getChildHostName(childID);

  AGT_TRACE_FUNCTION(childID << ", " << size);
  stat_in(this->myName, "getCommTime");
  // FIXME: this code was too much relying on FAST
  // so it returns 0 every time
  ms_strfree(child_name);

  stat_out(this->myName, "getCommTime");
  return(time);
} // getCommTime

/**
 * Return a pointer to a unique aggregated response from various responses.
 * @param request contains pointers to the scheduler and the responses.
 * @param max_srv the maximum number of servers to aggregate (all if 0).
 */
corba_response_t *
AgentImpl::aggregate(Request *request, size_t max_srv) {
  char statMsg[128];
  GlobalScheduler *GS = request->getScheduler();
  corba_response_t *aggregResp = new corba_response_t;
  aggregResp->reqID = request->getRequest()->reqID;
  AGT_TRACE_FUNCTION(request->getRequest()->pb.path << ", " <<
                     request->getResponsesSize() << " responses, " << max_srv);
#ifdef __WIN32__
  _snprintf(statMsg, 128, "aggregate %ld",
           (unsigned long) request->getRequest()->reqID);
#else
  snprintf(statMsg, 128, "aggregate %ld",
           (unsigned long) request->getRequest()->reqID);
#endif

  stat_in(this->myName, statMsg);
  GS->aggregate(aggregResp, max_srv,
                request->getResponsesSize(), request->getResponses());

  stat_out(this->myName, statMsg);
  return aggregResp;
} // aggregate

/** Get host name of a child (returned string is ms_stralloc'd). */
char *
AgentImpl::getChildHostName(CORBA::Long childID) {
  char *hostName = NULL;

  int childFound = 0;

  AGT_TRACE_FUNCTION(childID);
  stat_in(this->myName, "getChildHostName");
  /* Return local host name if childID == -1 */
  /* (This hack is used during the aggregation */

  if (childID == -1) {
    return ms_strdup(localHostName);
  }

  /* Is the child an agent ? */
  // to avoid the overflow
  if (childID < static_cast<CORBA::Long>(LAChildren.size())) {
    LAChild childDesc = LAChildren[childID];
    if (childDesc.defined()) {
      hostName = ms_strdup(childDesc.getHostName());
      childFound = 1;
    }
  }

  if (!childFound && childID < static_cast<CORBA::Long>(SeDChildren.size())) {
    SeDChild childDesc = SeDChildren[childID];
    if (childDesc.defined()) {
      hostName = ms_strdup(childDesc.getHostName());
      childFound = 1;
    }
  }

  if (!childFound) {
    WARNING("trying to extract IOR of an unknown child");
  }
  stat_out(this->myName, "getChildHostName");
  return hostName;
} // getChildHostName

// Forwarder part
AgentFwdrImpl::AgentFwdrImpl(Forwarder_ptr fwdr, const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

CORBA::Long
AgentFwdrImpl::agentSubscribe(const char *me, const char *hostName,
                              const SeqCorbaProfileDesc_t &services) {
  return forwarder->agentSubscribe(me, hostName, services, objName);
}

CORBA::Long
AgentFwdrImpl::serverSubscribe(const char *me, const char *hostName,
                               const SeqCorbaProfileDesc_t &services) {
  return forwarder->serverSubscribe(me, hostName, services, objName);
}

CORBA::Long
AgentFwdrImpl::childUnsubscribe(CORBA::ULong childID,
                                const SeqCorbaProfileDesc_t &services) {
  return forwarder->childUnsubscribe(childID, services, objName);
}

CORBA::Long
AgentFwdrImpl::removeElement(bool recursive) {
  return forwarder->removeElement(recursive, objName);
}

CORBA::Long
AgentFwdrImpl::childRemoveService(CORBA::ULong childID,
                                  const corba_profile_desc_t &profile) {
  return forwarder->childRemoveService(childID, profile, objName);
}

char *
AgentFwdrImpl::getDataManager() {
  return forwarder->getDataManager(objName);
}

SeqString *
AgentFwdrImpl::searchData(const char *request) {
  return forwarder->searchData(request, objName);
}

CORBA::Long
AgentFwdrImpl::addServices(CORBA::ULong myID,
                           const SeqCorbaProfileDesc_t &services) {
  return forwarder->addServices(myID, services, objName);
}

void
AgentFwdrImpl::getResponse(const corba_response_t &resp) {
  forwarder->getResponse(resp, objName);
}

CORBA::Long
AgentFwdrImpl::ping() {
  return forwarder->ping(objName);
}

char *
AgentFwdrImpl::getHostname() {
  return forwarder->getHostname(objName);
}
