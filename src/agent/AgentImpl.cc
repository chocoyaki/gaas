/****************************************************************************/
/* DIET agent implementation source code                                    */
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
 * Revision 1.24  2004/10/14 15:03:31  hdail
 * Changed to cleaner, shorter debug message for response.
 *
 * Revision 1.23  2004/09/29 13:35:31  sdahan
 * Add the Multi-MAs feature.
 *
 * Revision 1.22  2004/09/15 10:38:31  hdail
 * Repaired a bug involving problems with service registration in upper levels of
 * the agent hierarchy.  Added debugging statements in debug level 10 to trace
 * registration.
 *
 * Revision 1.21  2004/09/14 12:43:11  hdail
 * Clarify "thread-safe" comments for Counter class.
 *
 * Revision 1.20  2004/09/13 14:10:26  hdail
 * Correcting memory management bugs:
 * - mc->children is now deleted with delete [] to match allocation by [].
 * - cleaned up allocation & copies involving class variables localHostName and
 *   myName
 *
 * Revision 1.19  2004/07/08 12:49:59  alsu
 * code to adjust time estimates disabled if DIET is not compiled with FAST
 *
 * Revision 1.18  2004/06/11 15:45:39  ctedesch
 * add DIET/JXTA
 *
 * Revision 1.17  2004/06/11 11:32:34  ecaron
 * Delete dead code (DEVELOPPING_DATA_PERSISTENCY part)
 ****************************************************************************/

#include "AgentImpl.hh"

#include <iostream>
using namespace std;
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "common_types.hh"
#include "debug.hh"
#include "FASTMgr.hh"
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"

/** The trace level. */
extern unsigned int TRACE_LEVEL;

#define AGT_TRACE_FUNCTION(formatted_text)       \
  TRACE_TEXT(TRACE_ALL_STEPS, "Agt::");          \
TRACE_FUNCTION(TRACE_ALL_STEPS,formatted_text)

#define MAX_HOSTNAME_LENGTH  256

AgentImpl::AgentImpl()
{
  this->childIDCounter       = 0;
  this->nbLAChildren         = 0;
  this->nbSeDChildren        = 0;
  this->SrvT                 = new ServiceTable();
  this->reqList.clear();
  this->locMgr               = NULL;
  this->myName               = NULL;
  this->localHostName        = NULL; 

#ifdef HAVE_LOGSERVICE
  dietLogComponent = NULL;
#endif
} // AgentImpl()

AgentImpl::~AgentImpl()
{
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
  //delete this->SrvT;
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");
  TRACE_TEXT(TRACE_STRUCTURES, "Remove the Request list...");
  this->reqList.clear();
  TRACE_TEXT(TRACE_STRUCTURES, "Done\n");
  if (ORBMgr::unbindObj(ORBMgr::AGENT, this->myName)) {
    WARNING("could not undeclare myself as " << this->myName);
  }
  stat_finalize();
  TRACE_TEXT(TRACE_STRUCTURES, "All Done\n");
} // ~AgentImpl()


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
  int
AgentImpl::run()
{
  char* name;

  /* Set host name */
  this->localHostName = new char[MAX_HOSTNAME_LENGTH];
  char* host = (char*)
    (Parsers::Results::getParamValue(Parsers::Results::DIETHOSTNAME));
  if (host != NULL) {
    strncpy(this->localHostName, host, MAX_HOSTNAME_LENGTH-1) ;
  } else {
    if (gethostname(this->localHostName, MAX_HOSTNAME_LENGTH)) {
      ERROR("could not get hostname", 1);
    }
  }
  this->localHostName[MAX_HOSTNAME_LENGTH-1] = '\0';

  /* Bind this agent to its name in the CORBA Naming Service */
  name = (char*)Parsers::Results::getParamValue(Parsers::Results::NAME);
  if (name == NULL)
    return 1;
  this->myName = new char[strlen(name)+1];
  strcpy(this->myName, name);
  if (ORBMgr::bindObjToName(_this(), ORBMgr::AGENT, this->myName)) {
    ERROR("could not declare myself as " << this->myName, 1);
  }

  // Init FAST (HAVE_FAST is managed by the FASTMgr class)
  return FASTMgr::init();

} // run()


/** Set this->locMgr */
  int
AgentImpl::linkToLocMgr(LocMgrImpl* locMgr)
{
  this->locMgr = locMgr;
  return 0;
}

#if HAVE_LOGSERVICE
void
AgentImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}
#endif


/****************************************************************************/
/* Subscribing and adding services                                          */
/****************************************************************************/

/**
 * Subscribe an agent as a LA child. Remotely called by an LA.
 */
CORBA::ULong
AgentImpl::agentSubscribe(Agent_ptr me, const char* hostName,
    const SeqCorbaProfileDesc_t& services)
{
  CORBA::ULong retID = (this->childIDCounter)++; // thread safe Counter class

  TRACE_TEXT(TRACE_MAIN_STEPS, "An agent has registered from << " << hostName
      << ", with " << services.length() << " services.\n");

  /* the size of the list is childIDCounter+1 (first index is 0) */
  this->LAChildren.resize(this->childIDCounter);
  LocalAgent_var meLA = LocalAgent::_narrow(me) ;
  this->LAChildren[retID] = LAChild(meLA, hostName);
  (this->nbLAChildren)++; // thread safe Counter class

  this->addServices(retID, services); 

  return retID;
} // agentSubscribe(Agent_ptr me, const char* hostName, ...)


/**
 * Subscribe a server as a SeD child. Remotely called by an SeD.
 */

CORBA::ULong
AgentImpl::serverSubscribe(SeD_ptr me, const char* hostName,
#if HAVE_JXTA
			   const char* uuid,
#endif // HAVE_JXTA
    const SeqCorbaProfileDesc_t& services)
{
  CORBA::ULong retID;

  TRACE_TEXT(TRACE_MAIN_STEPS, "A server has registered from " << hostName
      << ", with " << services.length() << " services.\n");

  assert (hostName != NULL);
  retID = (this->childIDCounter)++; // thread safe

  // FIXME: resize calls default constructor + copy constructor + 1 desctructor
  this->SeDChildren.resize(childIDCounter);
  (this->SeDChildren[retID]).set(me, hostName
#if HAVE_JXTA
				 , uuid
#endif // HAVE_JXTA
				 );
  (this->nbSeDChildren)++; // thread safe Counter class

  this->addServices(retID, services);

  return retID;
} // serverSubscribe(SeD_ptr me, const char* hostName, ...)


/**
 * Add \c services into the service table, and attach them to child \c me.
 */
  void
AgentImpl::addServices(CORBA::ULong myID,
                       const SeqCorbaProfileDesc_t& services)
{
  int result;
  AGT_TRACE_FUNCTION(myID <<", " << services.length() << " services");

#if HAVE_LOGSERVICE
  // Commented out because we don't currently need to track services in 
  // agents via LogService.
#if 0
  for (CORBA::ULong i=0; i<services.length(); i++) {
    if (dietLogComponent != NULL) {
      dietLogComponent->logAddService(&(services[i]));
    }
  }
#endif
#endif

  this->srvTMutex.lock();
  for (size_t i = 0; i < services.length(); i++) {
    result = this->SrvT->addService(&(services[i]), myID);
    if(result == 0){
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i 
          << " added for child " << myID << ".\n");
    } else {
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i 
            << " is a duplicate for child " << myID << ". Not added.\n");
    }
  }
  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    this->SrvT->dump(stdout);
  this->srvTMutex.unlock();
} // addServices(CORBA::ULong myID, const SeqCorbaProfileDesc_t& services)



/****************************************************************************/
/* findServer                                                               */
/****************************************************************************/

/**
 * Forward a request, schedule and merge the responses associated.
 * @param req     the request.
 * @param max_srv the maximum number of servers to sort.
 * @return non NULL pointer to a response
 */
  corba_response_t*
AgentImpl::findServer(Request* req, size_t max_srv)
{
  size_t i; //, j, k;
  corba_response_t* resp = new corba_response_t;
  const corba_request_t& creq = *(req->getRequest());

  TRACE_TEXT(TRACE_MAIN_STEPS,
      "\n**************************************************\n"
      << "Got request " << creq.reqID
      << " on problem " << creq.pb.path << endl);
  stat_in(this->myName,"findServer");

  /* Initialize the response */
  resp->reqID = creq.reqID;
  resp->sortedIndexes.length(0);
  resp->servers.length(0);

  /* Add the new request to the list */
  reqList[creq.reqID] = req;

  /* Find capable children */
  ServiceTable::ServiceReference_t serviceRef;
  srvTMutex.lock();
  serviceRef = SrvT->lookupService(&(creq.pb));

  if (serviceRef == -1) {
    WARNING("no service found for request " << creq.reqID);
    srvTMutex.unlock();

  } else { // then the request must be forwarded

    int nbChildrenContacted = 0;
    ServiceTable::matching_children_t *mc, *SrvTmc;
    SrvTmc = SrvT->getChildren(serviceRef);

    /* Contact the children. The responses array will not be ready until all
       children are contacted. Thus lock the responses mutex now.           */
    req->lock();

    // Copy matching children for sendRequest ...
    mc = new ServiceTable::matching_children_t(*SrvTmc);
    mc->children = new CORBA::ULong[mc->nb_children];
    for (i = 0; i < (size_t)mc->nb_children; i++)
      mc->children[i] = SrvTmc->children[i];

    srvTMutex.unlock();

    for (i = 0; i < (size_t)mc->nb_children; i++) {
      sendRequest(mc->children[i], &creq);
    } // for (i = 0; i < ms->nb_children; i++)

    delete[] mc->children;
    delete mc;

    srvTMutex.lock();
    nbChildrenContacted = SrvTmc->nb_children;
    srvTMutex.unlock();

    /* if no alive server can solve the problem, return */
    if (!nbChildrenContacted) {
      WARNING("no service found for request " << creq.reqID);
      TRACE_TEXT(TRACE_MAIN_STEPS,
          "**************************************************\n");
      req->unlock();
      //delete req; // do not delete since getRequest does not perform a copy
      return resp;
    }

    /* We don't need the locs table anymore */
    //delete [] locs;

    /* Everything is ready, we can now wait for the responses */
    /* (This call implicitly unlocks the responses mutex)     */
    TRACE_TEXT(TRACE_ALL_STEPS, "Waiting for " << nbChildrenContacted
        << " responses to request " << creq.reqID <<  "...\n");
    req->waitResponses(nbChildrenContacted);
    req->unlock();

    /* The thread is awakened when all responses are gathered */

#if HAVE_JXTA || ! HAVE_FAST
    /* this part seems to generate JNI problems */
    /* but this part should be removed in the next DIET version */
    /* that explains why we don't fix it */ 

    /* in fact, the following code is only useful if one has
    ** chosen to compile with fast.  furthermore, it causes
    ** problems in the custom estimators by fiddling with the
    ** estimation after the SeD has set it.
    **
    ** to include this kind of estimation, the plan is to
    ** either (i) send the transfer time estimation *down*
    ** the hierarchy as the request descends, or (ii) use a
    ** CORBA-based ping mechanism from the SeD to the MA (is
    ** this even possible?
    */
#else
    /* Update communication times for all non-persistent parameters:
     *  process IN and OUT args separately, which means process them according
     *  to the way of their transfers.
     * NB: This does not affect the order of the servers (only comm times). */

    size_t nb_resp = req->getResponsesSize();

    double* time = new double[nb_resp]; 

    for (i = 0; (int)i <= creq.pb.last_out; i++) {

      unsigned long
        size = sizeof(corba_data_t) + data_sizeof(&(creq.pb.param_desc[i]));

      for (j = 0; j < nb_resp; j++) {
        corba_response_t* resp_j = &(req->getResponses()[j]);

        time[j] = 0.0;

        if ((int)i <= creq.pb.last_in) {
          /* IN args : compute comm time if they are volatile or if their id is
             non assigned yet (which means it is not stored in the platform) */
          if (((creq.pb.param_desc[i].mode == DIET_VOLATILE)
                || (*(creq.pb.param_desc[i].id.idNumber) == '\0'))) {
            time[j] = getCommTime(resp_j->myID, size);
          }
        } else if ((int)i <= creq.pb.last_inout) {
          /* INOUT args: add comm times for both directions IN (with the
             conditions above) and OUT (with the conditions below) */
          if ((creq.pb.param_desc[i].mode == DIET_VOLATILE)
              || (*(creq.pb.param_desc[i].id.idNumber) == '\0')) {
            time[j] = getCommTime(resp_j->myID, size);
           }
          if (creq.pb.param_desc[i].mode <= DIET_PERSISTENT_RETURN) {
            time[j] += getCommTime(resp_j->myID, size, false);
          }
        } else if (creq.pb.param_desc[i].mode <= DIET_PERSISTENT_RETURN) {
          /* OUT args : compute comm time if they are volatile or specified as
             "return". */
          time[j] = getCommTime(resp_j->myID, size, false);
        }

        /* Inject the computed comm time (or 0.0 - default) into all servers */
        for (k = 0; k < resp_j->servers.length(); k++) {
          resp_j->servers[k].estim.commTimes[i] += time[j];
          resp_j->servers[k].estim.totalTime += time[j];
        } 
      }
    }
    delete [] time;
#endif // HAVE_JXTA

    resp = this->aggregate(req, max_srv);
    // Just for debugging
    if (TRACE_LEVEL >= TRACE_STRUCTURES){
      //displayResponse(stdout, resp);
      displayResponseShort(stdout,resp);
    }
  }

  stat_out(this->myName,"findServer");

  return resp;
} // findServer(Request* req)


/****************************************************************************/
/* getResponse                                                              */
/****************************************************************************/

/** Get the response of a child */
  void
AgentImpl::getResponse(const corba_response_t& resp)
{
  TRACE_TEXT(TRACE_MAIN_STEPS, "Got a response from " << resp.myID
      <<"th child" << " to request " << resp.reqID << endl);
  stat_in(this->myName,"getResponse");
  /* The response should be copied in the logs */
  /* Look for the concerned request in the logs */
  Request* req = reqList[resp.reqID];
  if (req) { // req == NULL if it doesn't exist.
    req->lock();
    req->addResponse(&resp);
    req->unlock();
  } else {
    WARNING("response to unknown request");
  } // if (req)
  stat_out(this->myName,"getResponse");
} // getResponse(const corba_response_t & resp)


/**
 * Used to test if this agent is alive.
 */
  CORBA::Long
AgentImpl::ping()
{
  AGT_TRACE_FUNCTION("");
  return 0;
} // ping()


  char*
AgentImpl::getHostname()
{
  return CORBA::string_dup(localHostName) ;
}


/**
 * Send the request structure \c req to the child whose ID is \c childID.
 */
  void
AgentImpl::sendRequest(CORBA::ULong childID, const corba_request_t* req)
{
  bool childFound = false;
  typedef size_t comm_failure_t;

  AGT_TRACE_FUNCTION(childID << ", " << req->pb.path);
  stat_in(this->myName,"sendRequest");
  try {
    /* Is the child an agent ? */
    if (childID < static_cast<CORBA::ULong>(LAChildren.size())) {
      LAChild& childDesc = LAChildren[childID];
      if (childDesc.defined()) {
        try {
          try {
            /* checks if the child is alive with a ping */
            childDesc->ping();
            childDesc->getRequest(*req);
          } catch (CORBA::COMM_FAILURE& ex) {
            throw (comm_failure_t)0;
          } catch (CORBA::TRANSIENT& e) {
            throw (comm_failure_t)1;
          }
        } catch (comm_failure_t& e) {
          if (e == 0 || e == 1) {
            WARNING("connection problems with LA child " << childID
                << " occured - remove it from known children");
            srvTMutex.lock();
            SrvT->rmChild(childID);
            if (TRACE_LEVEL >= TRACE_STRUCTURES) {
              SrvT->dump(stdout);
            }
            srvTMutex.unlock();
            LAChildren[childID] = LAChild();
            --nbLAChildren;
          } else {
            throw e;
          }
        }
        childFound = true;
      }
    }
    if (!childFound && childID < static_cast<CORBA::ULong>(SeDChildren.size())) {
      /* Then it must be a server */
      SeDChild& childDesc = SeDChildren[childID];
      if (childDesc.defined()) {
        try {
          try {
            /* checks if the child is alive with a ping */
            childDesc->ping();
            childDesc->getRequest(*req);
          } catch (CORBA::COMM_FAILURE& ex) {
            throw (comm_failure_t)0;
          } catch (CORBA::TRANSIENT& e) {
            throw (comm_failure_t)1;
          }
        } catch (comm_failure_t& e) {
          if (e == 0 || e == 1) {
            WARNING("connection problems with SeD child " << childID
                << " occured - remove it from known children");
            srvTMutex.lock();
            SrvT->rmChild(childID);
            if (TRACE_LEVEL >= TRACE_STRUCTURES) {
              SrvT->dump(stdout);
            }
            srvTMutex.unlock();
            SeDChildren[childID] = SeDChild();
            --nbSeDChildren;
          } else {
            throw e;
          }
        }
        childFound = true;
      }

      if (!childFound) {
        WARNING("trying to send a request to an unknown child");
      }
    }
  } catch(...) {
    WARNING("exception thrown in Agt::" << __FUNCTION__);
  }
  stat_out(this->myName,"sendRequest");
} // sendRequest(CORBA::Long childID, const corba_request_t* req)



/**
 * Get communication time between this agent and the child \c childID for a data
 * amount of size \c size. The way of the data transfer can be specified with
 * \c to : if (to), from this agent to the child, else from the child to this
 * agent.
 */
  inline double
AgentImpl::getCommTime(CORBA::Long childID, unsigned long size, bool to)
{

  double time;
  char* child_name = getChildHostName(childID);

  AGT_TRACE_FUNCTION(childID <<", " << size);
  stat_in(this->myName,"getCommTime");
  time = FASTMgr::commTime(this->localHostName, child_name, size, to);

  ms_strfree(child_name);

  stat_out(this->myName,"getCommTime");
  return(time);
} // getCommTime(int childID, unsigned long size, bool to)



/**
 * Return a pointer to a unique aggregated response from various responses.
 * @param request contains pointers to the scheduler and the responses.
 * @param max_srv the maximum number of servers to aggregate (all if 0).
 */
  corba_response_t*
AgentImpl::aggregate(Request* request, size_t max_srv)
{
  corba_response_t* aggregResp = new corba_response_t;
  GlobalScheduler* GS = request->getScheduler();

  AGT_TRACE_FUNCTION(request->getRequest()->pb.path << ", " <<
      request->getResponsesSize() << " responses, " << max_srv);
  stat_in(this->myName,"aggregate");
  GS->aggregate(aggregResp, max_srv,
      request->getResponsesSize(), request->getResponses());
  aggregResp->reqID = request->getRequest()->reqID;

  stat_out(this->myName,"aggregate");
  return aggregResp;
} // aggregate(Request* request, size_t max_srv)


/** Get host name of a child (returned string is ms_stralloc'd). */
  char*
AgentImpl::getChildHostName(CORBA::Long childID)
{
  char* hostName = NULL;

  int childFound = 0;

  AGT_TRACE_FUNCTION(childID);
  stat_in(this->myName,"getChildHostName");
  /* Return local host name if childID == -1 */
  /* (This hack is used during the aggregation */

  if (childID == -1) {
    return ms_strdup(localHostName);
  }

  /* Is the child an agent ? */
  // to avoid the overflow
  if(childID < static_cast<CORBA::Long>(LAChildren.size())) {
    LAChild childDesc = LAChildren[childID];
    if(childDesc.defined()) {
      hostName = ms_strdup(childDesc.getHostName());
      childFound = 1;
    }
  }

  if (!childFound && childID < static_cast<CORBA::Long>(SeDChildren.size())) {
    SeDChild childDesc = SeDChildren[childID];
    if(childDesc.defined()) {
      hostName = ms_strdup(childDesc.getHostName());
      childFound = 1;
    }
  }

  if (!childFound) {
    WARNING("trying to extract IOR of an unknown child");
  }
  stat_out(this->myName,"getChildHostName");
  return hostName;
} // getChildHostName(CORBA::Long childID)

