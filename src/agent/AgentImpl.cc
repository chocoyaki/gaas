/****************************************************************************/
/* DIET agent implementation source code                                    */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.39  2006/01/25 21:07:59  pfrauenk
 * CoRI - plugin scheduler: the type diet_est_tag_t est replace by int
 *        some new fonctions in DIET_server.h to manage the estVector
 *
 * Revision 1.38  2006/01/19 21:35:42  pfrauenk
 * CoRI : when --enable-cori - round-robin is the default scheduler -
 *        CoRI is not called (any more) for collecting information
 *        (so no FAST possible any more)
 *
 * Revision 1.37  2005/11/10 14:37:51  eboix
 *     Clean-up of Cmake/DIET_config.h.in and related changes. --- Injay2461
 *
 * Revision 1.36  2005/09/07 07:41:02  hdail
 * Cleanup of alternative prediction handling
 *
 * Revision 1.35  2005/09/05 16:06:09  hdail
 * - Addition of method for aggregating parameter location data.
 * - Handling of alternative performance prediction for parameter transfer
 *   times by agents. (experimental and protected by HAVE_ALTPREDICT)
 *
 * Revision 1.34  2005/08/11 17:45:02  alsu
 * correcting bug __addCommTime: first argument was a struct, which was
 * copied onto the stack of the function.  changes to this struct were
 * thus not visible in the caller's scope.  new version takes a pointer
 * as the first argument.
 *
 * Revision 1.33  2005/05/16 12:27:02  alsu
 * eliminate warning on uninitialized corba_response_t
 *
 * Revision 1.32  2005/05/15 15:49:40  alsu
 * to indicate sucess/failure, addServices not returns a value
 *
 * Revision 1.31  2005/05/02 16:47:45  ycaniou
 * Nothing but indentation of the code
 *
 * Revision 1.30  2005/04/25 08:56:43  hdail
 * Cleaned up memory leak for corba_response_t* resp.
 *
 * Revision 1.29  2005/04/13 08:49:11  hdail
 * Beginning of adoption of new persistency model: DTM is enabled by default and
 * JuxMem will be supported via configure flags.  DIET will always provide at
 * least one type of persistency.  As a first step, persistency across DTM and
 * JuxMem is not supported so all persistency handling should be surrounded by
 *     #if HAVE_JUXMEM
 *         // JuxMem code
 *     #else
 *         // DTM code
 *     #endif
 * This check-in prepares for the JuxMem check-in by cleaning up old
 * DEVELOPPING_DATA_PERSISTENCY flags and surrounding DTM code with
 * #if ! HAVE_JUXMEM / #endif flags to be replaced by above format by Mathieu's
 * check-in.  Currently the HAVE_JUXMEM flag is set in AgentImpl.hh - to be
 * replaced by Mathieu's check-in of a configure system for JuxMem.
 *
 * Revision 1.28  2005/04/08 13:02:43  hdail
 * The code for LogCentral has proven itself stable and it seems bug free.
 * Since no external libraries are required to compile in LogCentral, its now
 * going to be compiled in by default always ... its usage is easily controlled by
 * configuration file.
 *
 * Revision 1.27  2004/12/14 16:17:51  alsu
 * in maintainer mode without FAST, we need to avoid compiling the
 * __addCommTime static function so that the -Werror gcc option doesn't
 * break the build
 *
 * Revision 1.26  2004/12/08 15:02:51  alsu
 * plugin scheduler first-pass validation testing complete.  merging into
 * main CVS trunk; ready for more rigorous testing.
 *
 * Revision 1.25.2.1  2004/11/30 15:48:00  alsu
 * fixing problems found testing FAST with plugin schedulers
 *
 * Revision 1.25  2004/10/15 08:19:56  hdail
 * Removed references to corba_response_t->sortedIndexes - no longer useful.
 *
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
#include "DIET_config.h"
#include <iostream>
using namespace std;
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  // For gethostname()

#include "common_types.hh"
#include "debug.hh"
#if !HAVE_CORI
#include "FASTMgr.hh"
#else //HAVE_CORI
#include "CORIMgr.hh"
#endif //!HAVE_CORI
#include "ms_function.hh"
#include "ORBMgr.hh"
#include "Parsers.hh"
#include "statistics.hh"
#include "est_internal.hh"

#if HAVE_ALTPREDICT
bool
__estimateTransfers(Request* request, corba_response_t* aggResponse);
void
__estimateTotalCosts(Request* request, corba_response_t* aggResponse);
static double
__predictTransferTime(corba_data_loc_t *dataLoc, corba_server_t *server);
static double
__estimateProximity(corba_data_loc_t *dataLoc, corba_server_t *server);
bool
__isTransferWithClient(corba_pb_desc_t* pb, int idx);
double
__calculateTransferEffort(estVector_t ev, SeqDataLoc_t locations);
#endif // HAVE_ALTPREDICT

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
  this->dietLogComponent     = NULL;
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

#if !HAVE_CORI 
    // Init FAST (HAVE_FAST is managed by the FASTMgr class)
  return FASTMgr::init();
#else 
   int use =
     *((size_t*)Parsers::Results::getParamValue(Parsers::Results::FASTUSE));
  if (use > 0){
     CORIMgr::add(EST_COLL_FAST,NULL);
     return CORIMgr::startCollectors();}
  else return 0;
#endif //HAVE_CORI
} // run()

#if ! HAVE_JUXMEM
/** Set this->locMgr for DTM usage */
int
AgentImpl::linkToLocMgr(LocMgrImpl* locMgr)
{
  this->locMgr = locMgr;
  return 0;
}
#endif // ! HAVE_JUXMEM

void
AgentImpl::setDietLogComponent(DietLogComponent* dietLogComponent) {
  this->dietLogComponent = dietLogComponent;
}


/****************************************************************************/
/* Subscribing and adding services                                          */
/****************************************************************************/

/**
 * Subscribe an agent as a LA child. Remotely called by an LA.
 */
CORBA::Long
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

  if (this->addServices(retID, services) != 0) {
    return (-1);
  }

  return retID;
} // agentSubscribe(Agent_ptr me, const char* hostName, ...)


/**
 * Subscribe a server as a SeD child. Remotely called by an SeD.
 */

CORBA::Long
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

  if (this->addServices(retID, services) != 0) {
    return (-1);
  }

  return retID;
} // serverSubscribe(SeD_ptr me, const char* hostName, ...)


/**
 * Add \c services into the service table, and attach them to child \c me.
 */
CORBA::Long
AgentImpl::addServices(CORBA::ULong myID,
                       const SeqCorbaProfileDesc_t& services)
{
  int result;
  AGT_TRACE_FUNCTION(myID <<", " << services.length() << " services");

/* Commented to reduce overhead of un-needed log messages.  Enable if you
 * want to track service information at the agent-level.
  for (CORBA::ULong i=0; i<services.length(); i++) {
    if (dietLogComponent != NULL) {
      dietLogComponent->logAddService(&(services[i]));
    }
  }
*/

  this->srvTMutex.lock();
  for (size_t i = 0; i < services.length(); i++) {
    result = this->SrvT->addService(&(services[i]), myID);
    if(result == 0){
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i 
          << " added for child " << myID << ".\n");
    }
    else if (result == -1) {
      TRACE_TEXT(TRACE_STRUCTURES, "Service " << i 
            << " is a duplicate for child " << myID << ". Not added.\n");
    }
    else if (result == -2) {
      this->SrvT->rmChild(myID);
      return (-1);
    }
    else {
      INTERNAL_ERROR(__FUNCTION__ <<
                     ": unexpected return code from " <<
                     "ServiceTable::addService (" <<
                     result <<
                     ")", -1);
    }
  }
  if (TRACE_LEVEL >= TRACE_STRUCTURES)
    this->SrvT->dump(stdout);
  this->srvTMutex.unlock();

  return (0);
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
  corba_response_t* resp;
  const corba_request_t& creq = *(req->getRequest());

  TRACE_TEXT(TRACE_MAIN_STEPS,
      "\n**************************************************\n"
      << "Got request " << creq.reqID
      << " on problem " << creq.pb.path << endl);
  stat_in(this->myName,"findServer");

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
#if HAVE_ALTPREDICT
    resp->dataLoc.length(0);
#endif // HAVE_ALTPREDICT
  }
  else { // then the request must be forwarded

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

      resp = new corba_response_t;
      resp->reqID = creq.reqID;
      resp->servers.length(0);
#if HAVE_ALTPREDICT
      resp->dataLoc.length(0);
#endif // HAVE_ALTPREDICT
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

#if HAVE_JXTA || ! HAVE_FAST || ! HAVE_ALTPREDICT
    /* this part seems to generate JNI problems */
    /* but this part should be removed in the next DIET version */
    /* that explains why we don't fix it */ 

    /* in fact, the following code is only useful if one has
    ** chosen to compile with fast.  
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

      size_t j;

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

        // if no time is needed, we can skip this iteration
        if (time[j] > 0.0) {

          size_t server_iter;

          /* Inject the computed comm time into all servers */
          for (server_iter = 0 ;
               server_iter < resp_j->servers.length() ;
               server_iter++) {
//             resp_j->servers[server_iter].estim.commTimes[i] += time[j];
//             resp_j->servers[server_iter].estim.totalTime += time[j];
            __addCommTime(&(resp_j->servers[server_iter].estim), i, time[j]);
          }
        }
      }
    }

    delete [] time;
#endif // HAVE_JXTA || ! HAVE_FAST || ! HAVE_ALTPREDICT

    resp = this->aggregate(req, max_srv);

#if ! HAVE_ALTPREDICT
    if (TRACE_LEVEL >= TRACE_STRUCTURES){
      //displayResponse(stdout, resp);
      displayResponseShort(stdout,resp);
    }
#else
    displayResponse(stdout,resp);
#endif // HAVE_ALTPREDICT
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


#if ! HAVE_ALTPREDICT
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
#if !HAVE_CORI
 time = FASTMgr::commTime(this->localHostName, child_name, size, to);
#else //HAVE_CORI
  estVector_t ev=new corba_estimation_t();
#if HAVE_FAST
  commTime_t commTime_param={this->localHostName,child_name,size,to};

  CORIMgr::call_cori_mgr(&ev,
		EST_COMMTIME,
		EST_COLL_FAST,	
		&commTime_param);
  time = diet_est_get(ev, EST_COMMTIME, HUGE_VAL);
#endif //HAVE_FAST
  time = HUGE_VAL;
#endif //HAVE_CORI

  ms_strfree(child_name);

  stat_out(this->myName,"getCommTime");
  return(time);
} // getCommTime(int childID, unsigned long size, bool to)
#endif // ! HAVE_ALTPREDICT


/**
 * Return a pointer to a unique aggregated response from various responses.
 * @param request contains pointers to the scheduler and the responses.
 * @param max_srv the maximum number of servers to aggregate (all if 0).
 */
corba_response_t*
AgentImpl::aggregate(Request* request, size_t max_srv)
{
  GlobalScheduler* GS = request->getScheduler();
#if ! HAVE_ALTPREDICT
  corba_response_t* aggregResp = new corba_response_t;
#else
  corba_response_t* aggregResp = this->aggregateLocationInfo(request);
  /** Update individual transfer costs */
  if (__estimateTransfers(request, aggregResp)) {
    /** All data locations known -- estimate total costs */
    __estimateTotalCosts(request, aggregResp);
  }
#endif  // ! HAVE_ALTPREDICT

  aggregResp->reqID = request->getRequest()->reqID;
  AGT_TRACE_FUNCTION(request->getRequest()->pb.path << ", " <<
      request->getResponsesSize() << " responses, " << max_srv);
  stat_in(this->myName,"aggregate");
  GS->aggregate(aggregResp, max_srv,
      request->getResponsesSize(), request->getResponses());

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

#if HAVE_ALTPREDICT
corba_response_t*
AgentImpl::aggregateLocationInfo(Request* request)
{
  corba_response_t* aggregResp = new corba_response_t;

  corba_pb_desc_t pb = request->getRequest()->pb;
  corba_response_t* responses = request->getResponses();
  /* Collect all available data locations */
  bool thisDataLocKnown = false;  // Do we know where current parameter is
  corba_data_loc_t* tmpDataLoc;
    
  aggregResp->dataLoc.length( responses[0].dataLoc.length() );
  for (unsigned int i = 0; i < responses[0].dataLoc.length(); i++) {
    aggregResp->dataLoc[i].bytes = responses[0].dataLoc[i].bytes;
    aggregResp->dataLoc[i].idNumber = 
          CORBA::string_dup(responses[0].dataLoc[i].idNumber);

    /* Check all responses for one that contains the data location */
    thisDataLocKnown = false;
    // i: parameter #, j: response #
    for (unsigned int j = 0; j < request->getResponsesSize(); j++) {
      if (strcmp(responses[j].dataLoc[i].hostName, "") != 0) {
        aggregResp->dataLoc[i].hostName = 
              CORBA::string_dup(responses[j].dataLoc[i].hostName);
        aggregResp->dataLoc[i].locationID =
              CORBA::string_dup(responses[j].dataLoc[i].locationID);
        thisDataLocKnown = true;
        break;
      }
    } // Check all responses for info on data location 

    if (thisDataLocKnown == false &&
        __isTransferWithClient(&pb, (int) i)) {
      /* Transfer involves client and server */
      aggregResp->dataLoc[i].hostName = 
            CORBA::string_dup(request->getRequest()->clientHostname);
      aggregResp->dataLoc[i].locationID = 
            CORBA::string_dup(request->getRequest()->clientLocationID);
      thisDataLocKnown = true;
    } // Does transfer involve client

    if ( thisDataLocKnown == false &&
         (int) i > pb.last_inout &&
         (int) i <= pb.last_out &&
        (pb.param_desc[i].mode == DIET_PERSISTENT ||
         pb.param_desc[i].mode == DIET_STICKY) ) {
      // OUT parameter, no return (zero transfer cost)
      aggregResp->dataLoc[i].hostName = CORBA::string_dup("__InPlace");
      aggregResp->dataLoc[i].locationID = CORBA::string_dup("__InPlace");
      thisDataLocKnown = true;
    }

    if (thisDataLocKnown == false) {
      /* Check if data exists in my subtree */
      tmpDataLoc =
          this->locMgr->getDataLocSubtree(aggregResp->dataLoc[i].idNumber);
      if (tmpDataLoc != NULL) {
        aggregResp->dataLoc[i].hostName =
                        CORBA::string_dup(tmpDataLoc->hostName);
        aggregResp->dataLoc[i].locationID =
                        CORBA::string_dup(tmpDataLoc->locationID);
        thisDataLocKnown = true;
      } else {
        /* This agent does not know where the data is */
      }
    }
  }   // Full loop for each parameter

  return aggregResp;
} // aggregateTransferInfo(Request* request)

/* Update the transfer predictions for each server in the responses.
 * Returns true if all data locations known, false otherwise.
 */
bool
__estimateTransfers(Request* request, 
                    corba_response_t* aggResponse)
{
  corba_pb_desc_t pb = request->getRequest()->pb;
  corba_response_t* responses = request->getResponses();
  bool allLocationsKnown = true;
  estVector_t ev;
  double tmpVal = 0.0;

  // i: parameter #, j: response #, k: server #
  for (unsigned int i = 0; i < responses[0].dataLoc.length(); i++) {
    if (strcmp(aggResponse->dataLoc[i].hostName, "") == 0) {
      // Unknown data location
      allLocationsKnown = false;
      continue;
    }

    for (unsigned int j = 0; j < request->getResponsesSize(); j++) {
      for (unsigned int k = 0; k < responses[j].servers.length(); k++) {
        ev = &(responses[j].servers[k].estim);

        if (!diet_est_array_defined_internal(ev, EST_COMMTIME, i)) {
          tmpVal = __predictTransferTime(&(aggResponse->dataLoc[i]),
                                         &(responses[j].servers[k].loc));
          diet_est_array_set_internal(ev, EST_COMMTIME, i, tmpVal);

          tmpVal = __estimateProximity(&(aggResponse->dataLoc[i]),
                                       &(responses[j].servers[k].loc));
          diet_est_array_set_internal(ev, EST_COMMPROXIMITY, i, tmpVal);
        } 
      } // Update all servers in response j
    }   // Update all responses
  }     // For each parameter
  return allLocationsKnown;
}

/** Estimate total transfer costs for moving all parameters.  
 * Requires that all data locations be known. */
void
__estimateTotalCosts(Request* request, 
                     corba_response_t* aggResponse) {
  corba_response_t* responses = request->getResponses();
  estVector_t ev;

  // j: response #, k: server number in response
  double transferEffort;
  for (unsigned int j = 0; j < request->getResponsesSize(); j++) {
    for (unsigned int k = 0; k < responses[j].servers.length(); k++) {
      ev = &(responses[j].servers[k].estim);
      if (! diet_est_defined_internal(ev, EST_TRANSFEREFFORT)) {
        // Overall transfer effort not yet predicted ... predict it
        transferEffort =
                __calculateTransferEffort(ev, aggResponse->dataLoc);
        diet_est_set_internal(ev, EST_TRANSFEREFFORT, transferEffort);
      }     /* End of if prediction needed */
    }       /* End search of server # k */
  }         /* End search of response # j */
}

/** Utility function to predict transfer times.  For now hard-coded 
 * for debugging purposes.  TODO: link with NWS. */
static double
__predictTransferTime(corba_data_loc_t *dataLoc, corba_server_t *server)
{
  double time = HUGE_VAL;

  // Error checking
  if (strcmp(dataLoc->hostName, "") == 0) {
    WARNING("predictTransferTime called with empty hostname for data location.  Predicting transfer time of inf.\n");
    time = HUGE_VAL;
  } else if (strcmp(server->hostName, "") == 0) {
    WARNING("predictTransferTime called with empty hostname for server.  Predicting transfer time of inf.\n");
    time = HUGE_VAL;
  } else if (strcmp(dataLoc->hostName, "__InPlace") == 0) {
    // No transfer required (e.g. persistent non-returned out argument)
    time = 0.0;
  } else {
    // Hostname given for data location and for server
    if (strcmp(dataLoc->hostName, "client") == 0) {
      // Transfer to or from client
      time = 1.11111;
    } else {
      time = 2.22222;
    }
  }

  return time;
}

static double
__estimateProximity(corba_data_loc_t *dataLoc, corba_server_t *server)
{
  double proximity = 0.0;

  // Error checking
  if (strcmp(dataLoc->locationID, "") == 0) {
    WARNING("estimateProximity called with empty data location.  Predicting proximity of 0.\n");
    proximity = 0.0;
  } else if (strcmp(server->locationID, "") == 0) {
    WARNING("estimateProximity called with empty server location.  Predicting proximity of 0.0.\n");
    proximity = 0.0;
  } else if (strcmp(dataLoc->locationID, "__InPlace") == 0) {
    // No transfer required (e.g. persistent non-returned out argument)
    proximity = HUGE_VAL;     // "Infinitely close"
  } else {
    // Calculate estimate of proximity
    char *dataID, *hostID;
    char *dataToken, *hostToken;
    size_t dataOffset = 0, hostOffset = 0;
    size_t dataTokenSize, hostTokenSize;

    dataID = strdup(dataLoc->locationID);
    hostID = strdup(server->locationID);

    proximity = 0.0;
    while ( (dataOffset < strlen(dataID)) &&
            (hostOffset < strlen(hostID)) ) {
      dataTokenSize = strcspn(
          dataID + dataOffset*sizeof(char), "_");
      hostTokenSize = strcspn(
          hostID + hostOffset*sizeof(char), "_");

      if (dataTokenSize != hostTokenSize) {
        break;                  // This token does not match
      }

      dataToken = (char *) strndup(
            (char *) ((int) dataID + dataOffset*sizeof(char)),
            dataTokenSize);
      hostToken = (char *) strndup(
             (char *) ((int) hostID + hostOffset*sizeof(char)),
             hostTokenSize);

      //printf("%s Token %f is %s (size %d), dataLoc is %d\n",
            //dataID, proximity, dataToken, dataTokenSize, dataOffset);
      //printf("%s Token %f is %s (size %d), hostLoc is %d\n",
            //hostID, proximity, hostToken, hostTokenSize, hostOffset);

      if (strcmp(dataToken, hostToken) != 0) {
        free(dataToken);
        free(hostToken);
        break;
      } else {
        proximity = proximity + 1.0;
        dataOffset += dataTokenSize + 1;
        hostOffset += hostTokenSize + 1;
        free(dataToken);
        free(hostToken);
      }
    }
    free(dataID);
    free(hostID);
  }
  /*printf("** Proximity of data %s and server %s = %f\n", 
    (char *) dataLoc->locationID, (char *) server->locationID, proximity);*/

  return proximity;
}

/** Calculate heuristic estimate for overall transfer effort based on 
 * proximity estimates */
double
__calculateTransferEffort(estVector_t ev, 
                          SeqDataLoc_t locations) {
  double transferEffort = 0.0;
  double proximity;

  for (int k = 0; k < (int) locations.length(); k++) {
    proximity = diet_est_array_get_internal(ev, EST_COMMPROXIMITY,
                  k, 0.0);

    if (proximity == HUGE_VAL) {
      // Transfer effort is 0 (in-place data).  Don't add to estimate.
    } else if (proximity < 0) {
      // Uknown proximity for this parameter. Can't estimate transferEffort.
      transferEffort = HUGE_VAL;
      break;
    } else if ((proximity >= 0) && (proximity < 5)) {
      transferEffort += (5 - proximity)*locations[k].bytes;
    } else {
      // For proximities > 4 and < inf, assume 'close' and use 1
      transferEffort += 1*locations[k].bytes;
    }
  } /* End search of all parameters */
  return (transferEffort);
}


bool
__isTransferWithClient(corba_pb_desc_t* pb, int idx) {
  bool isClientTransfer = false;

  // First find whether data will be transferred to or from client
  if (idx <= pb->last_in) {      // IN
    if ((pb->param_desc[idx].mode == DIET_VOLATILE) ||
        (strcmp(pb->param_desc[idx].id.idNumber, "") == 0)) {
      // IN parameter, either volatile or new persistent
      isClientTransfer = true;
    }
  } else if (idx <= pb->last_inout) { // INOUT
    if (pb->param_desc[idx].mode == DIET_VOLATILE) {
      // INOUT parameter, volatile  (transfer in and out)
      // Note that the cost of the double transfer should be 
      // accounted for by the SeD marking 2x the data size
      isClientTransfer = true;
    } else if (strcmp(pb->param_desc[idx].id.idNumber, "") == 0) {
      // INOUT parameter, new persistent (transfer in)
      isClientTransfer = true;
    } 
    /*else if (pb.param_desc[idx].mode == DIET_PERSISTENT_RETURN) {
      * PROBLEM: this type of parameter should include estimation of
      * both transfer from persistent storage and returned response to 
      * client =(
      *
      // INOUT parameter, old persistent (transfer out)
      clientData = true;
    }*/
  } else if (idx <= pb->last_out) {  // OUT
    if ( (pb->param_desc[idx].mode == DIET_VOLATILE) ||
         (pb->param_desc[idx].mode == DIET_PERSISTENT_RETURN)) {
      // OUT parameter, volatile or OUT parameter, persistent return
      isClientTransfer = true;
    }
  } else {
    WARNING("AgentImpl aggregate: don't know anything about parameter "
              << idx << "\n");
  }

  return isClientTransfer;
}

#endif // HAVE_ALTPREDICT

