/**
 * @file  MasterAgentImpl.cc
 *
 * @brief  DIET master agent implementation source code
 *
 * @author  Philippe COMBES (Philippe.Combes@ens-lyon.fr)
 *          Sylvain DAHAN (Sylvain.Dahan@lifc.univ-fcomte.fr)
 *          Frederic LOMBARD (Frederic.Lombard@lifc.univ-fcomte.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include <cstdio>
#include <iostream>

#include <boost/format.hpp>

#include "configuration.hh"
#include "MasterAgentImpl.hh"
#include "debug.hh"
#include "statistics.hh"
#include "ReferenceUpdateThread.hh"
#include "FloodRequestsList.hh"
#include "DIET_uuid.hh"

#ifdef HAVE_MULTI_MA
#include "ORBMgr.hh"
#endif


#ifdef HAVE_WORKFLOW
/*
   Workflow utilities header
 */

omni_mutex reqCount_mutex;

#endif /* HAVE_WORKFLOW */

// #define aff_val(x)
#define aff_val(x) cout << # x << " = " << x << "\n";

#define MA_TRACE_FUNCTION(formatted_text)               \
  TRACE_TEXT(TRACE_ALL_STEPS, "MA::");                  \
  TRACE_FUNCTION(TRACE_ALL_STEPS, formatted_text)

MasterAgentImpl::MasterAgentImpl(): AgentImpl() {
  this->reqIDCounter = 0;
  this->num_session = 0;
  this->num_data = 0;
#ifdef HAVE_MULTI_MA
  this->floodRequestsList = new FloodRequestsList();
#endif /* HAVE_MULTI_MA */
} // MasterAgentImpl


MasterAgentImpl::~MasterAgentImpl() {
#ifdef HAVE_MULTI_MA
  delete floodRequestsList;
  // MAList.emptyIt();
#endif /* HAVE_MULTI_MA */
} // MasterAgentImpl::~MasterAgentImpl()


/**
 * Launch this agent (initialization + registration in the hierarchy).
 */
int
MasterAgentImpl::run() {
  int res = this->AgentImpl::run();
  if (res) {
    return res;
  }

  this->reqIDCounter = 0;
  unsigned long tmpCounter;
  if (CONFIG_ULONG(diet::INITREQUESTID, tmpCounter)) {
    this->reqIDCounter = tmpCounter;
  }

#ifdef HAVE_MULTI_MA
  /* launch the bind service */
  unsigned long port;
  if (CONFIG_ULONG(diet::BINDSERVICEPORT, port)) {
    bindSrv = new BindService(this, port);
    char *bindName = ms_stralloc(strlen(localHostName) + 20);
    sprintf(bindName, "%s:%lu", localHostName, port);
    this->bindName = bindName;
    reqIDCounter = KeyString::hash(bindName);
  } else {
    reqIDCounter = KeyString::hash(localHostName);
  }
  reqIDCounter = ((reqIDCounter & 0xFFFFF) ^ ((reqIDCounter >> 12) & 0xFFF))
                 * 1000;

  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ..." << std::endl);

  /* get the list of neighbours */
  std::string neighbors = "";
  CONFIG_STRING(diet::NEIGHBOURS, neighbors);

  // FIXME: use std::string instead
  char *neighbours = ms_strdup(neighbors.c_str());
  char *comma, *begin_copy;
  begin_copy = neighbours;
  while ((comma = strchr(neighbours, ',')) != NULL) {
    comma[0] = '\0';
    if (neighbours[0] != '\0') {
      MAIds.insert(CORBA::string_dup(neighbours));
    }
    neighbours = comma + 1;
  }
  MAIds.insert(CORBA::string_dup(neighbours));
  free(begin_copy);

  /* initialize some variables */
  unsigned long conf;

  if (CONFIG_ULONG(diet::MINNEIGHBOURS, conf)) {
    minMAlinks = conf;
  } else {
    minMAlinks = 2;
  }

  if (CONFIG_ULONG(diet::MAXNEIGHBOURS, conf)) {
    maxMAlinks = conf;
  } else {
    maxMAlinks = 10;
  }

  if (CONFIG_ULONG(diet::UPDATELINKPERIOD, conf)) {
    new ReferenceUpdateThread(this, conf);
  } else {
    new ReferenceUpdateThread(this, 300);
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "Getting MAs references ... done." << std::endl);
#endif /* HAVE_MULTI_MA */

  /* num_session thread safe*/
  // NOTE: std::endl already flushes out output stream
  TRACE_TEXT(TRACE_MAIN_STEPS, std::endl
             << "Master Agent " << this->myName << " started." << std::endl);

  catalog = new MapDagdaCatalog();

  return 0;
} // run


/**
 * Returns the identifier of a data by agreggation of numsession and numdata
 */
char *
MasterAgentImpl::get_data_id() {
  boost::format id("DAGDA://id-%1%-%2%");
  boost::uuids::uuid uuid = diet_generate_uuid();
  id % uuid % myName;

  return CORBA::string_dup(id.str().c_str());
}

/****************************************************************************/
/* Available Service                                                        */
/****************************************************************************/

/**
 * Returns the list of Profile available
 */
SeqCorbaProfileDesc_t *
MasterAgentImpl::getProfiles(CORBA::Long &length) {
  TRACE_TEXT(TRACE_ALL_STEPS, "ask for list of services\n");
  return this->AgentImpl::SrvT->getProfiles(length);
}


/****************************************************************************/
/* Submission                                                               */
/****************************************************************************/

/**
 * Invoke Loc Manager method to get data presence information (call by client)
 * When using DAGDA instead of DTM, uses the search of data on the platform.
 */
CORBA::ULong
MasterAgentImpl::dataLookUp(const char *argID) {
  return dataManager->pfmIsDataPresent(argID);
} // dataLookUp(const char* argID)

/**
 * invoke loc Manager method to get data descriptor of the data identified by argID
 * When using DAGDA, the description is obtained from DAGDA instead of DTM.
 */
corba_data_desc_t *
MasterAgentImpl::get_data_arg(const char *argID) {
  return dataManager->pfmGetDataDesc(argID);
}

/** Problem submission : remotely called by client. */
#if !HAVE_ALTPREDICT
corba_response_t *
MasterAgentImpl::submit(const corba_pb_desc_t &pb_profile,
                        CORBA::ULong maxServers)
#else /* ! HAVE_ALTPREDICT */
corba_response_t *
MasterAgentImpl::submit(const corba_pb_desc_t &pb_profile,
                        CORBA::ULong maxServers,
                        const char * clientHostname,
                        const char * clientLocID)
#endif /* ! HAVE_ALTPREDICT */
{
  corba_request_t creq;
  corba_response_t *decision(0);
  char statMsg[128];
  MA_TRACE_FUNCTION(pb_profile.path << ", " << maxServers);

  /* Initialize statistics module */
  stat_init();

  try {
    /* Initialize the corba request structure */
    creq.reqID = reqIDCounter++;  // thread safe
    sprintf(statMsg, "start request %ld", (unsigned long) creq.reqID);
    stat_in(this->myName, statMsg);
    creq.pb = pb_profile;
    creq.max_srv = maxServers;
#if HAVE_ALTPREDICT
    creq.clientHostname = CORBA::string_dup(clientHostname);
    creq.clientLocationID = CORBA::string_dup(clientLocID);
#endif /* HAVE_ALTPREDICT */

#ifdef USE_LOG_SERVICE
    if (dietLogComponent) {
      dietLogComponent->logAskForSeD(&creq);
    }
#endif /* USE_LOG_SERVICE */

    decision = submit_local(creq);

#ifdef HAVE_MULTI_MA
    if (decision->servers.length() == 0) {
      sprintf(statMsg, "start floodRequest %ld",
              (unsigned long) creq.reqID);
      stat_in(this->myName, statMsg);

      FloodRequest &floodRequest =
        *(new FloodRequest(MADescription(),
                           MADescription(_this(), myName),
                           creq, knownMAs));

      while ((decision->servers.length() == 0) &&
             (!floodRequest.flooded())) {
        TRACE_TEXT(TRACE_ALL_STEPS, "multi-MAs search "
                   << creq.pb.path
                   << " request (" << creq.reqID << ")" << std::endl);
        int flooded = floodRequest.floodNextStep();
        if (!flooded) {
          bool requestAdded =
            floodRequestsList->put(floodRequest);
          assert(requestAdded);
          floodRequest.waitResponses();
          try {
            floodRequestsList->get(creq.reqID);
            *decision = floodRequest.getDecision();
            TRACE_TEXT(TRACE_ALL_STEPS, decision->servers.length()
                       << " SeD have been found for request ("
                       << creq.reqID << ")" << std::endl);
          } catch (FloodRequestNotFoundException &f) {
            WARNING("Can not found the requested decision in multi-MA search");
          }
        }
      }
      try {
        floodRequest.stopFlooding();
      } catch (FloodRequestNotFoundException &e) {
        WARNING(e);
      }
      delete &floodRequest;

      sprintf(statMsg, "stop floodRequest %ld",
              (unsigned long) creq.reqID);
      stat_out(this->myName, statMsg);
    }
#endif /* HAVE_MULTI_MA */
  } catch (...) {
    WARNING("An exception was caught\n");
  }

#ifdef USE_LOG_SERVICE
  if (dietLogComponent) {
    dietLogComponent->logSedChosen(&creq, decision);
  }
#endif /* USE_LOG_SERVICE */

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "**************************************************"
             << std::endl);
  sprintf(statMsg, "stop request %ld", (unsigned long) creq.reqID);
  stat_out(this->myName, statMsg);
  stat_flush();

  return decision;
}


/** Problem submission. Looking for SeDs that can resolve the
    problem in the local domain. */
corba_response_t *
MasterAgentImpl::submit_local(const corba_request_t &creq) {
  corba_response_t *resp(0);
  Request *req(0);

  /* Initialize the request with a global scheduler */
  TRACE_TEXT(TRACE_ALL_STEPS, "Initialize the request "
             << creq.reqID << ".\n");
  /* Check that service exists */
  ServiceTable::ServiceReference_t sref;
  srvTMutex.lock();
  sref = this->SrvT->lookupService(&(creq.pb));

  if (sref == -1) { /* service does not exist */
    srvTMutex.unlock();
    /* Initialize the response */
    resp = new corba_response_t;
    resp->reqID = creq.reqID;
    resp->servers.length(0);
#if HAVE_ALTPREDICT
    resp->dataLoc.length(0);
#endif /* HAVE_ALTPREDICT */
  } else {
#if !defined HAVE_ALT_BATCH
    CORBA::Long numProfiles;
    SeqCorbaProfileDesc_t *profiles = this->SrvT->getProfiles(numProfiles);
    assert(sref < numProfiles);
    const corba_profile_desc_t profile = (*profiles)[sref];
#else /* ! defined HAVE_ALT_BATCH */
      /* I have defined, for batch cases, ServiceTable::getProfile(index)
         I use it here because of efficiency.
         Can we replace previous non batch code?

         TODO: we can only manipulate reference here... look if we can change
         chooseGlobalScheduler() prototype */
    corba_profile_desc_t profile = this->SrvT->getProfile(sref);
    /* Copy parallel flag of the client profile (reason why not const
       anymore) */
    profile.parallel_flag = creq.pb.parallel_flag;
#endif /* ! defined HAVE_ALT_BATCH */
    srvTMutex.unlock();
    req = new Request(&creq,
                      GlobalScheduler::chooseGlobalScheduler(&creq,
                                                             &profile));
    /** Forward request and schedule the responses */
    resp = findServer(req, creq.max_srv);
#if !defined HAVE_ALT_BATCH
    delete profiles;
#endif /* ! defined HAVE_ALT_BATCH*/
  }

  // Constructor initializes sequences with length == 0
  if ((resp) && (resp->servers.length() != 0)) {
    resp->servers.length(MIN(resp->servers.length(),
                             static_cast<size_t>(creq.max_srv)));
    TRACE_TEXT(TRACE_ALL_STEPS, "Decision signaled.\n");
    resp->myID = (ChildID) - 1;
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "No server found for problem "
               << creq.pb.path << ".\n");
  }

  reqList[creq.reqID] = 0;
  delete req;

  return resp;
} // submit_local(const corba_request_t& req, ...)


CORBA::Long
MasterAgentImpl::get_session_num() {
  (this->num_session)++;
  return num_session;
} // get_session_num()

/**
   invoked by client : frees persistent data identified by argID, if not exists return NULL
 */
CORBA::Long
MasterAgentImpl::diet_free_pdata(const char *argID) {
  if (!dataManager->pfmIsDataPresent(argID)) {
    return 0;
  }
  dataManager->pfmRemData(argID);
  return 1;
} // diet_free_pdata(const char* argID)


#ifdef HAVE_MULTI_MA
char *
MasterAgentImpl::getBindName() {
  return CORBA::string_dup(bindName);
}


/* Update MAs references */
void
MasterAgentImpl::updateRefs() {
  MAIds.lock();
  MasterAgent_var ma;
  int loopCpt = 0;

  for (StrList::iterator iter = MAIds.begin();
       iter != MAIds.end(); ++iter) {
    if (loopCpt < maxMAlinks) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Resolving " << *iter << "...");
      ma = bindSrv->lookup(*iter);
      if (CORBA::is_nil(ma)) {
        TRACE_TEXT(TRACE_ALL_STEPS, "not found" << std::endl);
      } else {
        TRACE_TEXT(TRACE_ALL_STEPS, "found" << std::endl);
        try {
          bool result = ma->handShake(myName, bindName);
          if (result) {
            TRACE_TEXT(TRACE_ALL_STEPS,
                       "connection accepted" << std::endl);
            knownMAs[*iter] = MADescription(ma, ma->getHostname());
            loopCpt++;
          } else {
            TRACE_TEXT(TRACE_ALL_STEPS,
                       "connection refused" << std::endl);
            knownMAs.erase(*iter);
          }
        } catch (CORBA::SystemException &ex) {
          TRACE_TEXT(TRACE_ALL_STEPS,
                     "obsolete reference" << std::endl);
          knownMAs.erase(*iter);
        }
      }
    } else {
      knownMAs.erase(*iter);
    }
  }

  MAIds.unlock();
  logNeighbors();
} // updateRefs()




/****************************************************************************/
/* MAs handshake                                                            */
/****************************************************************************/

CORBA::Boolean
MasterAgentImpl::handShake(const char *maName, const char *myName) {
  TRACE_TEXT(TRACE_ALL_STEPS, myName
             << " is shaking my hand ("
             << knownMAs.size() << "/" << maxMAlinks << ")" << std::endl);
  MasterAgent_ptr me =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_ptr>(AGENTCTXT, maName);
  /* FIXME: There is probably a cleaner way to find if two IOR are equal */
  std::string myior = ORBMgr::getMgr()->getIOR(_this());
  std::string hisior = ORBMgr::getMgr()->getIOR(me);
  if (myior == hisior) {
    TRACE_TEXT(TRACE_ALL_STEPS,
               "I refuse to handshake with myself\n");
    /* we need to return now, because the knownMA locker is already
       taken by the updateRefs function which call the handshake
       one. */
    /*    free(myior);
          free(hisior);*/
    return false;
  }

  /*  free(myior);
      free(hisior);*/
  knownMAs.erase(myName);

  // there is to much links to accept a new one.
  if (knownMAs.size() >= static_cast<size_t>(maxMAlinks)) {
    return false;
  }

  MAIds.insert(myName);
  knownMAs[myName] = MADescription(me, me->getHostname());

  logNeighbors();
  return true;
} // handShake(MasterAgent_ptr me, const char* myName)

/****************************************************************************/
/*                           Flooding Algorithm                             */
/****************************************************************************/

void
MasterAgentImpl::searchService(const char *predecessorStr,
                               const char *predecessorId,
                               const corba_request_t &request) {
  char statMsg[128];
  MasterAgent_ptr predecessor =
    ORBMgr::getMgr()->resolve<MasterAgent, MasterAgent_ptr>(AGENTCTXT,
                                                            predecessorStr);
  TRACE_TEXT(
    TRACE_ALL_STEPS, predecessorId << " search "
                                   << request.pb.path <<
    " request (" << request.reqID << ")"
                                   << std::endl);

  /* Initialize statistics module */
  stat_init();

  sprintf(statMsg, "start searchService %ld", (unsigned long) request.reqID);
  stat_in(this->myName, statMsg);

  reqIdList.lock();
  ReqIdList::iterator pos = reqIdList.find(request.reqID);
  bool found = (pos != reqIdList.end());
  if (!found) {
    reqIdList.insert(pos, request.reqID);
  }
  reqIdList.unlock();

  if (found) {
    predecessor->alreadyContacted(request.reqID, bindName);
    TRACE_TEXT(TRACE_ALL_STEPS, "already contacted for request (" <<
               request.reqID << ")" << std::endl);
  } else {
    FloodRequest &floodRequest =
      *(new FloodRequest(MADescription(predecessor, predecessorId),
                         MADescription(_this(), bindName),
                         request, knownMAs));

    floodRequestsList->put(floodRequest);

    corba_response_t *decision = submit_local(request);

    if (decision->servers.length() == 0) {
      predecessor->serviceNotFound(request.reqID, bindName);
      TRACE_TEXT(TRACE_ALL_STEPS, "no server for request (" <<
                 request.reqID << ")" << std::endl);
    } else {
      predecessor->serviceFound(request.reqID, *decision);
      TRACE_TEXT(TRACE_ALL_STEPS, decision->servers.length()
                 << " server(s) found for request (" <<
                 request.reqID << ")" << std::endl);
    }
  }

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "**************************************************\n");
  sprintf(statMsg, "stop searchService %ld", (unsigned long) request.reqID);
  stat_out(this->myName, statMsg);
  stat_flush();
} // searchService(...)


void
MasterAgentImpl::stopFlooding(CORBA::Long reqId, const char *senderId) {
  try {
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    floodRequest.stopFlooding();
    delete &floodRequest;
    reqIdList.erase(reqId);
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  }
} // stopFlooding


void
MasterAgentImpl::serviceNotFound(CORBA::Long reqId, const char *senderId) {
  // fprintf(stderr, "serviceNotFound from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName);
  try {
    TRACE_TEXT(
      TRACE_ALL_STEPS, "service not found by " << senderId
                                               << " for request (" <<
      reqId << ")" << std::endl);
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    floodRequest.addResponseNotFound();
    floodRequestsList->put(floodRequest);
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  }
} // serviceNotFound


void
MasterAgentImpl::newFlood(CORBA::Long reqId, const char *senderId) {
  // fprintf(stderr, "newFlood from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName);
  TRACE_TEXT(
    TRACE_ALL_STEPS, senderId << " continue the search for "
                              << " request (" << reqId << ")" <<
    std::endl);
  try {
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    bool flooded = floodRequest.floodNextStep();
    floodRequestsList->put(floodRequest);
    if (flooded) {
      floodRequest.getPredecessor()->floodedArea(reqId, bindName);
    } else {
      floodRequest.waitResponses();
      floodRequestsList->get(reqId);
      try {
        corba_response_t decisions = floodRequest.getDecision();
        if (decisions.servers.length() != 0) {
          floodRequest.getPredecessor()->serviceFound(reqId,
                                                      decisions);
          decisions.servers.length(0);
        } else if (floodRequest.flooded()) {
          floodRequest.getPredecessor()->floodedArea(reqId, bindName);
        } else {
          floodRequest.getPredecessor()->serviceNotFound(reqId,
                                                         bindName);
        }
      } catch (FloodRequestNotFoundException &e) {
        WARNING(e);
      } catch (CORBA::SystemException &ex) {
        // does nothing
      }
      floodRequestsList->put(floodRequest);
    }
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  } catch (CORBA::SystemException &ex) {
    // does nothing
  }
} // newFlood

void
MasterAgentImpl::floodedArea(CORBA::Long reqId,
                             const char *senderId) {
  // fprintf(stderr, "floodedArea from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName);
  TRACE_TEXT(
    TRACE_ALL_STEPS, "stop the flood of " << senderId
                                          << " for request (" <<
    reqId << ")" << std::endl);
  try {
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    floodRequest.addResponseFloodedArea(senderId);
    floodRequestsList->put(floodRequest);
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  }
} // floodedArea


void
MasterAgentImpl::alreadyContacted(CORBA::Long reqId,
                                  const char *senderId) {
  // fprintf(stderr, "alreadyContacted from %s, %s:%d, %s\n", senderId, (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName);
  TRACE_TEXT(TRACE_ALL_STEPS, "already contacted for request ("
             << reqId << ")");
  try {
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    floodRequest.addResponseAlreadyContacted(senderId);
    floodRequestsList->put(floodRequest);
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  }
} // alreadyContacted


void
MasterAgentImpl::serviceFound(CORBA::Long reqId,
                              const corba_response_t &decision) {
  // printTime();
  // fprintf(stderr, "%d serviceFound, %s:%d, %s\n", (int)decision.length(), (const char*)reqId.maId, (int)reqId.idNumber, (const char*)myName);
  try {
    FloodRequest &floodRequest =
      floodRequestsList->get(reqId);
    floodRequest.addResponseServiceFound(decision);
    floodRequestsList->put(floodRequest);
  } catch (FloodRequestNotFoundException &e) {
    WARNING(e);
  }
  // printf("<<<<<%d service found\n", (int)decision.length());
} // serviceFound

void
MasterAgentImpl::logNeighbors() {
  char *str;
  size_t str_len = 1;

  knownMAs.lock();
  for (MasterAgentImpl::MAList::iterator iter = knownMAs.begin();
       iter != knownMAs.end(); ++iter)
    str_len += strlen(iter->first) + 1;

  str = new char[str_len];
  str[0] = 0;

  for (MasterAgentImpl::MAList::iterator iter = knownMAs.begin();
       iter != knownMAs.end(); ++iter) {
    strcat(str, iter->first);
    strcat(str, " ");
  }
  knownMAs.unlock();
  TRACE_TEXT(TRACE_MAIN_STEPS, "Multi-MAs neighbors " << str << std::endl);

#ifdef USE_LOG_SERVICE
  if (dietLogComponent) {
    dietLogComponent->logNeighbors(str);
  }
#endif /* USE_LOG_SERVICE */
} // logNeighbors

#endif /* HAVE_MULTI_MA */

#ifdef HAVE_WORKFLOW

/**
 * Workflow submission function.
 */
wf_response_t *
MasterAgentImpl::submit_pb_set(const corba_pb_desc_seq_t &seq_pb) {
  wf_response_t *wf_response = new wf_response_t;
  unsigned int len = seq_pb.length();
  unsigned int failureIdx;
  wf_response->wfn_seq_resp.length(0);
  corba_response_t *corba_response = 0;
  bool missingService = false;

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "The MasterAgent receives a set of "
             << len << " problems" << std::endl);
  // LOOP for MA submissions (keeps the order of problems sequence)
  for (unsigned int ix = 0; ix < len; ix++) {
    corba_response = this->submit(seq_pb[ix], 1024);
    if ((!corba_response) || (corba_response->servers.length() == 0)) {
      missingService = true;
      failureIdx = ix;
      break;
    } else {
      wf_response->wfn_seq_resp.length(ix + 1);
      wf_response->wfn_seq_resp[ix].node_id =
        CORBA::string_dup(seq_pb[ix].path);
      wf_response->wfn_seq_resp[ix].response = *corba_response;
    }
    delete corba_response;
  }

  // Handle exception of missing service
  if (!missingService) {
    wf_response->complete = true;
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               "The problem set can't be solved (one or more services are "
               << "missing) " << std::endl);
    wf_response->complete = false;
    wf_response->idxError = failureIdx;
  }

  return wf_response;
} // submit_pb_set
/**
 * A submission function used to submit a set of problem to the MA (OBSOLETE)
 *
 * @param pb_seq     sequence of problems
 * @param reqCount   number of requests of the client. The request number is at least
 *                   equal to sequence problem length but it can be greater if a problem
 *                   has to be executed more than one time
 * @param complete   indicates if the response is complete. The function return at the first problem
 *                   that cannot be solved
 * @param firstReqId the first request identifier to be used by the client
 * @param seqReqId   an identifier to the submission (each sequence submission
 *                   has a unique identifier)
 */
response_seq_t *
MasterAgentImpl::submit_pb_seq(const corba_pb_desc_seq_t &pb_seq,
                               CORBA::Long reqCount,
                               CORBA::Boolean &complete,
                               CORBA::Long &firstReqId,
                               CORBA::Long &seqReqId) {
  struct timeval start, end;
  gettimeofday(&start, 0);
  static CORBA::Long mySeqReqId = 0;
  response_seq_t *response_seq = new response_seq_t;
  corba_response_t *corba_response = 0;
  complete = false;

  for (unsigned int ix = 0; ix < pb_seq.length(); ix++) {
    corba_response = this->submit(pb_seq[ix], reqCount);
    if ((!corba_response) || (corba_response->servers.length() == 0)) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Problem sequence can't be solved: service "
                 << pb_seq[ix].path
                 << " missing) " << std::endl);
      return response_seq;
    } else {
      response_seq->length(ix + 1);
      (*response_seq)[ix].problem = pb_seq[ix];
      (*response_seq)[ix].servers = corba_response->servers;
    } // end if
  } // end for

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "Problem sequence can be solved (all services available) "
             << std::endl);
  complete = true;
  // Update request identifiers
  reqCount_mutex.lock();
  seqReqId = mySeqReqId++;
  reqIDCounter = reqIDCounter + reqCount - pb_seq.length();
  reqCount_mutex.unlock();

  gettimeofday(&end, 0);
  /*  time_t ptime = (end.tv_sec - start.tv_sec)* 1000 +
      (end.tv_usec - start.tv_usec)/1000;*/
#ifdef USE_LOG_SERVICE
  if (dietLogComponent) {
    // FIXME: update dietLogComponent with the new data structure
    // dietLogComponent->logDagSubmit(wf_response, ptime);
  }
#endif /* USE_LOG_SERVICE */
  return response_seq;
} // submit_pb_seq
#endif /* HAVE_WORKFLOW */

SeqString *
MasterAgentImpl::searchData(const char *request) {
  SeqString *ret = new SeqString();
  attributes_t attr = catalog->request(request);
  attributes_t::iterator it;
  int i = 0;

  ret->length(attr.size());
  for (it = attr.begin(); it != attr.end(); ++it)
    (*ret)[i++] = CORBA::string_dup(it->c_str());

  return ret;
} // searchData

CORBA::Long
MasterAgentImpl::insertData(const char *key,
                            const SeqString &values) {
  attributes_t attr;
  if (catalog->exists(key)) {
    return 1;
  }

  for (unsigned int i = 0; i < values.length(); ++i) {
    attr.push_back(std::string(values[i]));
  }
  catalog->insert(key, attr);
  return 0;
} // insertData

MasterAgentFwdrImpl::MasterAgentFwdrImpl(Forwarder_ptr fwdr,
                                         const char *objName) {
  this->forwarder = Forwarder::_duplicate(fwdr);
  this->objName = CORBA::string_dup(objName);
}

corba_response_t *
MasterAgentFwdrImpl::submit(const corba_pb_desc_t &pb_profile,
                            CORBA::ULong maxServers) {
  TRACE_TEXT(
    TRACE_MAIN_STEPS, __FILE__ << ": l." << __LINE__
                               << " (" << __FUNCTION__ << ")" <<
    std::endl
                               << "submit(pb_profile, " <<
    maxServers << ", "
                               << objName << ")" << std::endl);
  return forwarder->submit(pb_profile, maxServers, objName);
}

CORBA::Long
MasterAgentFwdrImpl::get_session_num() {
  return forwarder->get_session_num(objName);
}

char *
MasterAgentFwdrImpl::get_data_id() {
  return forwarder->get_data_id(objName);
}

CORBA::ULong
MasterAgentFwdrImpl::dataLookUp(const char *argID) {
  return forwarder->dataLookUp(argID, objName);
}

corba_data_desc_t *
MasterAgentFwdrImpl::get_data_arg(const char *argID) {
  return forwarder->get_data_arg(argID, objName);
}

CORBA::Long
MasterAgentFwdrImpl::diet_free_pdata(const char *argID) {
  return forwarder->diet_free_pdata(argID, objName);
}

SeqCorbaProfileDesc_t *
MasterAgentFwdrImpl::getProfiles(CORBA::Long &length) {
  return forwarder->getProfiles(length, objName);
}

#ifdef HAVE_MULTI_MA
CORBA::Boolean
MasterAgentFwdrImpl::handShake(const char *name,
                               const char *myName) {
  return forwarder->handShake(name, myName, objName);
}

char *
MasterAgentFwdrImpl::getBindName() {
  return forwarder->getBindName(objName);
}

void
MasterAgentFwdrImpl::searchService(const char *predecessor,
                                   const char *predecessorId,
                                   const corba_request_t &request) {
  forwarder->searchService(predecessor, predecessorId, request, objName);
}

void
MasterAgentFwdrImpl::stopFlooding(CORBA::Long reqId,
                                  const char *senderId) {
  forwarder->stopFlooding(reqId, senderId, objName);
}

void
MasterAgentFwdrImpl::serviceNotFound(CORBA::Long reqId,
                                     const char *senderId) {
  forwarder->serviceNotFound(reqId, senderId, objName);
}

void
MasterAgentFwdrImpl::newFlood(CORBA::Long reqId,
                              const char *senderId) {
  forwarder->newFlood(reqId, senderId, objName);
}

void
MasterAgentFwdrImpl::floodedArea(CORBA::Long reqId,
                                 const char *senderId) {
  forwarder->floodedArea(reqId, senderId, objName);
}

void
MasterAgentFwdrImpl::alreadyContacted(CORBA::Long reqId,
                                      const char *senderId) {
  forwarder->alreadyContacted(reqId, senderId, objName);
}

void
MasterAgentFwdrImpl::serviceFound(CORBA::Long reqId,
                                  const corba_response_t &decision) {
  forwarder->serviceFound(reqId, decision, objName);
}
#endif /* HAVE_MULTI_MA */
#ifdef HAVE_WORKFLOW
wf_response_t *
MasterAgentFwdrImpl::submit_pb_set(const corba_pb_desc_seq_t &seq_pb) {
  return forwarder->submit_pb_set(seq_pb, objName);
}

response_seq_t *
MasterAgentFwdrImpl::submit_pb_seq(const corba_pb_desc_seq_t &pb_seq,
                                   CORBA::Long reqCount,
                                   CORBA::Boolean &complete,
                                   CORBA::Long &firstReqId,
                                   CORBA::Long &seqReqId) {
  return forwarder->submit_pb_seq(pb_seq, reqCount, complete,
                                  firstReqId, seqReqId, objName);
}
#endif /* HAVE_WORKFLOW */

SeqString *
MasterAgentFwdrImpl::searchData(const char *request) {
  return forwarder->searchData(request, objName);
}

CORBA::Long
MasterAgentFwdrImpl::insertData(const char *key,
                                const SeqString &values) {
  return forwarder->insertData(key, values, objName);
}

CORBA::Long
MasterAgentFwdrImpl::agentSubscribe(const char *me, const char *hostName,
                                    const SeqCorbaProfileDesc_t &services) {
  return forwarder->agentSubscribe(me, hostName, services, objName);
}

CORBA::Long
MasterAgentFwdrImpl::serverSubscribe(const char *me, const char *hostName,
                                     const SeqCorbaProfileDesc_t &services) {
  return forwarder->serverSubscribe(me, hostName, services, objName);
}

CORBA::Long
MasterAgentFwdrImpl::childUnsubscribe(CORBA::ULong childID,
                                      const SeqCorbaProfileDesc_t &services) {
  return forwarder->childUnsubscribe(childID, services, objName);
}

CORBA::Long
MasterAgentFwdrImpl::bindParent(const char *parentName) {
  return forwarder->bindParent(parentName, objName);
}

CORBA::Long
MasterAgentFwdrImpl::disconnect() {
  return forwarder->disconnect(objName);
}

CORBA::Long
MasterAgentFwdrImpl::removeElement(bool recursive) {
  return forwarder->removeElement(recursive, objName);
}

CORBA::Long
MasterAgentFwdrImpl::addServices(CORBA::ULong myID,
                                 const SeqCorbaProfileDesc_t &services) {
  return forwarder->addServices(myID, services, objName);
}

CORBA::Long
MasterAgentFwdrImpl::childRemoveService(CORBA::ULong childID,
                                        const corba_profile_desc_t &profile) {
  return forwarder->childRemoveService(childID, profile, objName);
}

char *
MasterAgentFwdrImpl::getDataManager() {
  return forwarder->getDataManager(objName);
}

void
MasterAgentFwdrImpl::getResponse(const corba_response_t &resp) {
  forwarder->getResponse(resp, objName);
}

CORBA::Long
MasterAgentFwdrImpl::ping() {
  return forwarder->ping(objName);
}

char *
MasterAgentFwdrImpl::getHostname() {
  return forwarder->getHostname(objName);
}
