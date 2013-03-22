/**
 * @file MultiWfScheduler.cc
 *
 * @brief  The base class for multi-workflow schedule
 *
 * @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
 *          Benjamin Isnard (Benjamin.Isnard@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */


#include "marshalling.hh"
#include "est_internal.hh"
#include <cmath>
#include <iomanip>
#include "debug.hh"
#include "statistics.hh"
#ifdef USE_LOG_SERVICE
#include "DietLogComponent.hh"
#endif
#include "ORBMgr.hh"
#include "MaDag_impl.hh"
#include "MultiWfScheduler.hh"
#include "HEFTScheduler.hh"
#include "MaDagNodeLauncher.hh"
#include "Dag.hh"
#include "DagNode.hh"
#include "events/EventTypes.hh"
#include "DIET_compat.hh"

using namespace madag;
using namespace events;

/****************************************************************************/
/*                                                                          */
/*                      CLASS MultiWfScheduler                              */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfScheduler::MultiWfScheduler(MaDag_impl *maDag, nodePolicy_t nodePol)
  : nodePolicy(nodePol), platformType(PFM_ANY), mySem(0), interRoundDelay(100),
  myMaDag(maDag), keepOnRunning(true) {
  this->mySched = new HEFTScheduler();
  this->execQueue = NULL;  // must be initialized in derived class constructor
  gettimeofday(&this->refTime, NULL);  // init reference time
}

MultiWfScheduler::~MultiWfScheduler() {
  if (this->mySched != NULL) {
    delete this->mySched;
  }
}

std::string
MultiWfScheduler::toString() const {
  return "MultiWfScheduler";
}

/**
 * Change the intra-dag scheduler (by default it is HEFT)
 */
void
MultiWfScheduler::setSched(WfScheduler *sched) {
  this->mySched = sched;
}

/**
 * Change the platform type (by default it is PFM_ANY)
 */
void
MultiWfScheduler::setPlatformType(pfmType_t pfmType) {
  this->platformType = pfmType;
}

/**
 * Change the inter-round delay value (by default 100 ms)
 */
void
MultiWfScheduler::setInterRoundDelay(int IRD_value) {
  this->interRoundDelay = IRD_value;
}
/**
 * get the inter-round delay value
 */
const int
MultiWfScheduler::getInterRoundDelay() const {
  return this->interRoundDelay;
}
/**
 * get the MaDag object ref
 */
const MaDag_impl *
MultiWfScheduler::getMaDag() const {
  return this->myMaDag;
}

Dag *
MultiWfScheduler::getDag(const std::string &dagId) throw(MaDag::InvalidDag) {
  std::map<std::string, Dag *>::iterator iter = myDags.find(dagId);
  if (iter != myDags.end()) {
    return ((Dag *) iter->second);
  } else {
    throw(MaDag::InvalidDag(dagId.c_str()));
  }
}

/**
 * get the metadag of a dag
 */
MetaDag *
MultiWfScheduler::getMetaDag(Dag *dag) {
  std::map<std::string, MetaDag *>::iterator iter = myMetaDags.find(dag->getId());
  if (iter != myMetaDags.end()) {
    return ((MetaDag *) iter->second);
  } else {
    return NULL;
  }
}

/**
 * Process a new dag => when finished the dag is ready for execution
 */
void
MultiWfScheduler::scheduleNewDag(Dag *newDag, MetaDag *metaDag)
throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Beginning of exclusion block
  // TODO move exclusion lock later (need to make HEFTScheduler thread-safe)

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "\t ** New DAG to schedule (" << newDag->getId()
                                           << ") time=" <<
             this->getRelCurrTime() << "\n");

  // Dag internal scheduling
  TRACE_TEXT(TRACE_ALL_STEPS, "Making intra-dag schedule\n");
  myLock.lock();
  try {
    this->intraDagSchedule(newDag, myMaDag->getMA());
  } catch (...) {
    myLock.unlock();
    throw;
  }

  // Store Dag reference
  this->myDags[newDag->getId()] = newDag;

  // Store metaDag
  if (metaDag != NULL) {
    this->myMetaDags[newDag->getId()] = metaDag;
  }

  // Node queue creation (to manage ready nodes queueing)
  TRACE_TEXT(TRACE_ALL_STEPS, "Initializing new ready nodes queue\n");
  OrderedNodeQueue *readyNodeQ = this->createNodeQueue(newDag);

  // Init queue by setting input nodes as ready
  newDag->setInputNodesReady(this);

  // Insert node queue into pool of node queues managed by the scheduler
  TRACE_TEXT(TRACE_ALL_STEPS, "Inserting new node queue into queue pool\n");
  this->insertNodeQueue(readyNodeQ);

  // Set starting time of the DAG
  newDag->setStartTime(this->getRelCurrTime());

  // Send signal to scheduler thread to inform there are new nodes
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "%%%%% NEW DAG SUBMITTED: dag id = " << newDag->getId() << "\n");
  this->wakeUp(true);

  myLock.unlock();
} // scheduleNewDag

/**
 * Stop method
 */
void
MultiWfScheduler::stop(bool wait) {
  this->keepOnRunning = false;
  // post to mySem to unlock main loop
  this->mySem.post();
  if (wait) {
    this->join();
  }
}

/**
 * Execution method
 */
void *
MultiWfScheduler::run() {
  int loopCount = 0;
  // the ressource availability matrix
  std::map<SeD_ptr, bool> ressAvail;
  // the service availability matrix
  std::map<std::string, bool> servAvail;
  // the nb of nodes to move from ready to exec at each round (policy dep.)
  int nodePolicyCount = 0;
  // use for statistic output
  char statMsg[64];

  switch (this->nodePolicy) {
  case MULTIWF_NODE_METRIC:
    nodePolicyCount = -1;   // ie no limit (take all ready nodes)
    break;
  case MULTIWF_DAG_METRIC:
    nodePolicyCount = 1;    // ie take only 1 ready node
    break;
  case MULTIWF_NO_METRIC:
    break;
  }

  TRACE_TEXT(TRACE_ALL_STEPS, "Multi-Workflow scheduler is running\n");
  std::cout << std::setiosflags(std::ios_base::fixed) << std::setprecision(0);
  // display double as int
  // / Start a ROUND of node ordering & mapping
  // / New rounds are started as long as some nodes can be mapped to ressources
  // / (if no more ressources then we wait until a node is finished or a new dag
  // / is submitted)
  while (this->keepOnRunning) {
    loopCount++;

    int queuedNodeCount = 0;   // nb nodes put in execQueue (check for new round)
    int mappedNodeCount = 0;   // nb nodes mapped to a ressource (check for new round)
    int servAvailCount = 0;  // nb available services (check for new round)

    int nodeReadyCount = 0;    // the nb of nodes ready (statistics)
    int dagCount = 0;          // the nb of dag (statistics)
    int nodeTodoCount = 0;     // the nb of nodes still to be executed (statistics)

    TRACE_TEXT(TRACE_MAIN_STEPS, "\t ** Starting Multi-Workflow scheduler ("
               << loopCount << ") time=" << this->getRelCurrTime() << "\n");

    myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    TRACE_TEXT(TRACE_ALL_STEPS, "PHASE 1: Move ready nodes to exec queue\n");


    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      OrderedNodeQueue *readyQ = *qp;
      int npc = nodePolicyCount;
      DagNode *n = NULL;
      nodeReadyCount += (int) readyQ->size();
      dagCount++;
      ChainedNodeQueue *waitQ = this->waitingQueues[readyQ];
      nodeTodoCount += (int) readyQ->size() + (int) waitQ->size();  // for stats only
      while ((npc) && (n = readyQ->popFirstNode())) {
        // save the address of the readyQ for this node (used if node pushed back)
        n->setLastQueue(readyQ);
        // set priority of node (depends on choosen algorithm)
        this->setExecPriority(n);
        // insert node into execution queue
        execQueue->pushNode(n);
        queuedNodeCount++;
        npc--;
      }
      ++qp;  // go to next queue
    }
    // only write stats when there is something to stats.
    if (dagCount > 0) {
      sprintf(statMsg, "dagCount %d", dagCount);
      stat_info("MA_DAG", statMsg);
      sprintf(statMsg, "nodeReadyCount %d", nodeReadyCount);
      stat_info("MA_DAG", statMsg);
      sprintf(statMsg, "queuedNodeCount %d", queuedNodeCount);
      stat_info("MA_DAG", statMsg);
      sprintf(statMsg, "nodeTodoCount %d", nodeTodoCount);
      stat_info("MA_DAG", statMsg);
    }
    if (queuedNodeCount > 0) {
      TRACE_TEXT(
        TRACE_ALL_STEPS, "Phase 2: Check ressources for nodes in exec queue ("
        << queuedNodeCount << " nodes)\n");
      int requestCount = 0;

      switch (this->platformType) {
      case PFM_ANY:
        // Initialize service availability matrix
        for (std::list<DagNode *>::iterator nodeIter = execQueue->begin();
             nodeIter != execQueue->end();
             ++nodeIter) {
          servAvail[(*nodeIter)->getPbName()] = true;
          servAvailCount = servAvail.size();
        }
        TRACE_TEXT(TRACE_ALL_STEPS, "Nb of distinct services in queue: "
                   << servAvailCount << "\n");
        break;
      case PFM_SAME_SERVICES:
        TRACE_TEXT(
          TRACE_ALL_STEPS,
          "Limiting check to one ressource (same services on all ress.)\n");
      } // switch

      while (!execQueue->isEmpty()) {
        DagNode *n = execQueue->popFirstNode();
        OrderedNodeQueue *readyQ =
          dynamic_cast<OrderedNodeQueue *>(n->getLastQueue());
        bool ressourceFound = false;
        corba_server_estimation_t *servEst;
        // Test to process node (depends on platform type)
        bool nodeSubmit = ((this->platformType == PFM_ANY)
                           && (servAvailCount) && (servAvail[n->getPbName()]))
                          || ((this->platformType == PFM_SAME_SERVICES) &&
                              (requestCount < 1));

        if (nodeSubmit) {
          int submitReqID = 0;   // store ReqID of submit to provide it for solve

          TRACE_TEXT(TRACE_MAIN_STEPS,
                     "Submit request for node " << n->getCompleteId()
                                                << "(" <<
                     n->getPbName() << ") / exec prio = " <<
                     n->getPriority() << "\n");

          // SEND REQUEST TO PLATFORM (FOR CURRENT NODE)
          wf_response_t *wf_response = NULL;
          try {
            wf_response = getProblemEstimates(n, myMaDag->getMA());
          } catch (...) {
            std::cerr << "ERROR during MA submission\n";
            n->setAsFailed();
            continue;
          }
          ++requestCount;
          // CHECK RESSOURCE AVAILABILITY
          for (unsigned int jx = 0;
               jx < wf_response->wfn_seq_resp[0].response.servers.length();
               jx++) { // loop over servers
            servEst = &wf_response->wfn_seq_resp[0].response.servers[jx];
            double compTime = diet_est_get_internal(&servEst->estim, EST_TCOMP,
                                                    0);
            double EFT = diet_est_get_internal(&servEst->estim, EST_EFT, 0);
            if ((compTime == HUGE_VAL) || (EFT == HUGE_VAL)) {
              WARNING(
                "SeD estimation function does not provide correct values for "
                << "computation time (EST_COMPTIME) and EFT (EST_EFT)");
            }

            std::string sedName(servEst->loc.SeDName);
            SeD_ptr curSeDPtr =
              ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, sedName);
            std::string hostname(CORBA::string_dup(servEst->loc.hostName));
            TRACE_TEXT(
              TRACE_ALL_STEPS, "  server " << hostname
                                           << ": compTime=" <<
              compTime <<
              ": EFT=" << EFT << "\n");
            if (EFT - compTime <= 0) {  // test if available right now
              // test if the server has not been already chosen for another node
              bool ressAvailable = true;
              for (std::map<SeD_ptr, bool>::iterator ressAvailIter =
                     ressAvail.begin();
                   ressAvailIter != ressAvail.end();
                   ++ressAvailIter) {
                if (curSeDPtr->_is_equivalent(ressAvailIter->first)) {
                  ressAvailable = false;
                  break;
                }
              }
              // server is free so it can be used for this node
              if (ressAvailable) {
                ressourceFound = true;
                ressAvail[curSeDPtr] = false;
                submitReqID = wf_response->wfn_seq_resp[0].response.reqID;
                TRACE_TEXT(TRACE_ALL_STEPS,
                           "  server found: " << hostname << "\n");
                break;
              } // end if
            } // end if
          } // end for jx

          // EXECUTE NODE (NEW THREAD)
          if (ressourceFound) {
            TRACE_TEXT(
              TRACE_MAIN_STEPS, "  $$$$ Exec node on " << servEst->loc.hostName
                                                       << " : " <<
              n->getCompleteId() << "\n");
            mappedNodeCount++;
            // create a node launcher
            DagNodeLauncher *launcher = new MaDagNodeLauncher(
              n, this,
              myMaDag->
              getCltMan(n->getDag()->getId()));
            // record chosen SeD information
            launcher->setSeD(servEst->loc.SeDName, submitReqID, servEst->estim);
            // start node execution
            n->setRealStartTime(this->getRelCurrTime());
            n->start(launcher);  // non-blocking
          } else {
            if (this->platformType == PFM_ANY) {
              servAvail[n->getPbName()] = false;
              servAvailCount--;
              TRACE_TEXT(TRACE_MAIN_STEPS,
                         "Service " << n->getPbName() << " is not available\n");
            }
          }
          delete wf_response;
        } // end if (nodeSubmit)

        // PUT THE NODE BACK IN THE READY QUEUE if no ressource available or node skipped
        if (!nodeSubmit || !ressourceFound) {
          // set the priority to the initial value (intra-dag)
          this->setWaitingPriority(n);
          readyQ->pushNode(n);
        }
      } // end loop execQueue

      // cleanup availability matrix
      ressAvail.clear();
      servAvail.clear();

      // Destroy ready/waiting queues if both are empty
      std::list<OrderedNodeQueue *>::iterator qp2 = readyQueues.begin();
      while (qp2 != readyQueues.end()) {
        OrderedNodeQueue *curReadyQ = *qp2;
        ChainedNodeQueue *curWaitQ = waitingQueues[curReadyQ];
        if (curWaitQ->isEmpty() && curReadyQ->isEmpty()) {
          TRACE_TEXT(TRACE_ALL_STEPS,
                     "Node Queues are empty: remove & destroy\n");
          qp2 = readyQueues.erase(qp2);       // removes from the list
          this->deleteNodeQueue(curReadyQ);   // deletes both queues
        } else {
          ++qp2;
        }
      } // end while
        // Round-robbin on remaining queues
      if (!readyQueues.empty()) {
        OrderedNodeQueue *firstReadyQueue = readyQueues.front();
        readyQueues.pop_front();
        readyQueues.push_back(firstReadyQueue);
      }
    } // end if queuedNodeCount > 0

    myLock.unlock();

    // The condition to go for a new round is the availability of ressources: under the
    // hypothesis that all ressources are identical (in terms of provided services) for
    // a given dag then there may be available ressources only if all nodes in the
    // execQueue were assigned a ressource.
    // if ((queuedNodeCount == 0) || (queuedNodeCount > mappedNodeCount)) {
    if ((queuedNodeCount == 0)
        || ((this->platformType == PFM_ANY) && (servAvailCount == 0))
        || ((this->platformType == PFM_SAME_SERVICES) &&
            (mappedNodeCount == 0))) {
      if (queuedNodeCount == 0) {
        TRACE_TEXT(TRACE_MAIN_STEPS, "No ready nodes - sleeping\n");
      } else {
        TRACE_TEXT(TRACE_MAIN_STEPS, "No ressource available - sleeping\n");
      }
      this->mySem.wait();
      if (this->keepOnRunning) {
        // WAIT UNTIL A NEW DAG IS SUBMITTED OR A NODE IS COMPLETED
        this->postWakeUp();
        this->checkDagsRelease();
      }
    } else {
      // DELAY between rounds (to avoid interference btw submits)
      diet::usleep(this->interRoundDelay * 1000);
    } // end if
  } // end while (this->keepOnRunning)
  return NULL;
} // run

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

/**
 * Call MA to get server estimations for all services for nodes of a Dag
 * Note: creates the node profiles, uses the estimation class to optimize
 * the request ie reduce the nb of estimations requested to the MA
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(Dag *dag, MasterAgent_var MA)
throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Check that all services are available and get the estimations (with MA)
  TRACE_TEXT(TRACE_ALL_STEPS, "MultiWfScheduler: Marshalling the profiles\n");
  corba_pb_desc_seq_t *pbs_seq = new corba_pb_desc_seq_t();
  wf_response_t *wf_response = NULL;
  pbs_seq->length(dag->size());
  // Create a mapping table (estimation class => submission index)
  std::map<std::string, int> estimClassMap;
  // Current submit index is ix
  int ix = 0;
  for (std::map<std::string, DagNode *>::iterator iter = dag->begin();
       iter != dag->end();
       ++iter) {
    DagNode *dagNode = (DagNode *) iter->second;
    // creates the diet profile
    dagNode->initProfileSubmit();

    // set the submit index and decide if this node must be added to the request
    std::string nodeEstimClass = dagNode->getEstimationClass();
    bool submit = true;
    int submitIdx;
    if (nodeEstimClass.empty()) {
      // default case (no estimation class) => node is added to request
      submitIdx = ix++;
    } else {
      // estimation class defined => check if already added to request or not
      std::map<std::string, int>::iterator estimClassIter =
        estimClassMap.find(nodeEstimClass);
      if (estimClassIter != estimClassMap.end()) {
        submitIdx = (int) estimClassIter->second;
        submit = false;
        TRACE_TEXT(TRACE_ALL_STEPS,
                   "Node " << dagNode->getId()
                           << " using submit index=" <<
                   submitIdx << "\n");
      } else {
        submitIdx = ix++;
        estimClassMap.insert(make_pair(nodeEstimClass, submitIdx));
      }
    }
    dagNode->setSubmitIndex(submitIdx);

    // add the current node's profile to the sequence (if part of the request)
    if (submit) {
      mrsh_pb_desc(&(*pbs_seq)[submitIdx], dagNode->getProfile());
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
             "MultiWfScheduler: send " << ix << " profile(s) to the MA  ... "
                                       << "\n");
  // resize the sequence to the final length
  pbs_seq->length(ix);

  bool failed = false;
  std::string failureMsg;
  try {
    wf_response = MA->submit_pb_set(*pbs_seq);
  } catch (CORBA::SystemException &e) {
    failureMsg = " MultiWfScheduler: Got a CORBA " +
                 std::string(e._name()) + " exception (" +
                 std::string(e.NP_minorString()) + ")";
    failed = true;
    WARNING(failureMsg << "\n");
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "... done\n");
  delete pbs_seq;
  if (failed) {
    throw(MaDag::CommProblem(failureMsg.c_str()));
  }
  if (!wf_response->complete) {
    // get the faulty node using the submission index
    DagNode *failedDagNode = NULL;
    for (std::map<std::string, DagNode *>::iterator iter = dag->begin();
         iter != dag->end();
         ++iter) {
      DagNode *dagNode = (DagNode *) iter->second;
      if (dagNode->getSubmitIndex() == wf_response->idxError) {
        failedDagNode = dagNode;
        break;
      }
    }
    // throw corba exception with node details
    if (failedDagNode) {
      throw(MaDag::ServiceNotFound(failedDagNode->getId().c_str(),
                                   failedDagNode->getPbName().c_str(),
                                   failedDagNode->getPortsDescr().c_str()));
    } else {
      throw(MaDag::ServiceNotFound(NULL, NULL, NULL));
    }
  }
  return wf_response;
} // getProblemEstimates

/**
 * Call MA to get server estimations for one node
 * Note: the profile for the node is supposed to be already created
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(DagNode *node, MasterAgent_var MA)
throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  corba_pb_desc_seq_t *pbs_seq = new corba_pb_desc_seq_t();
  wf_response_t *wf_response = NULL;
  pbs_seq->length(1);
  node->setSubmitIndex(0);
  mrsh_pb_desc(&(*pbs_seq)[0], node->getProfile());
  TRACE_TEXT(TRACE_ALL_STEPS,
             "MultiWfScheduler: send 1 profile to the MA  ... "
             << "\n");
  bool failed = false;
  std::string failureMsg;
  try {
    wf_response = MA->submit_pb_set(*pbs_seq);
  } catch (CORBA::SystemException &e) {
    failureMsg = " MultiWfScheduler: Got a CORBA " +
                 std::string(e._name()) + " exception (" +
                 std::string(e.NP_minorString()) + ")";
    WARNING(failureMsg << "\n");
    failed = true;
  }
  delete pbs_seq;
  TRACE_TEXT(TRACE_ALL_STEPS, "... done\n");
  if (failed) {
    throw(MaDag::CommProblem(failureMsg.c_str()));
  }
  if (!wf_response->complete) {
    throw(MaDag::ServiceNotFound(node->getId().c_str(),
                                 node->getPbName().c_str(),
                                 node->getPortsDescr().c_str()));
  }
  return wf_response;
} // getProblemEstimates

/**
 * Intra-dag scheduling
 */
void
MultiWfScheduler::intraDagSchedule(Dag *dag, MasterAgent_var MA)
throw(MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Call the MA to get estimations for all services
  wf_response_t *wf_response = this->getProblemEstimates(dag, MA);
  // Prioritize the nodes (with intra-dag scheduler)
  this->mySched->setNodesPriority(wf_response, dag);
  // Cleanup
  delete wf_response;
}


/**
 * Create two chained node queues and return the ready queue
 * (uses the priority-based nodequeue)
 *  - WAITING queue => READY queue
 */

OrderedNodeQueue *
MultiWfScheduler::createNodeQueue(Dag *dag) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating new node queues (priority-based)\n");
  OrderedNodeQueue *readyQ = new PriorityNodeQueue();
  ChainedNodeQueue *waitQ = new ChainedNodeQueue(readyQ);
  for (std::map <std::string, DagNode *>::iterator nodeIt = dag->begin();
       nodeIt != dag->end();
       ++nodeIt) {
    waitQ->pushNode(&(*nodeIt->second));
  }
  this->waitingQueues[readyQ] = waitQ;  // used to destroy waiting queue
  return readyQ;
} // createNodeQueue

/**
 * Delete the two chained node queues
 *  - WAITING queue => READY queue
 */
void
MultiWfScheduler::deleteNodeQueue(OrderedNodeQueue *nodeQ) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Deleting node queues\n");
  ChainedNodeQueue *waitQ = waitingQueues[nodeQ];
  waitingQueues.erase(nodeQ);     // removes from the map
  delete waitQ;
  delete nodeQ;
}

/**
 * Insert new node queue into ready queues list
 */
void
MultiWfScheduler::insertNodeQueue(OrderedNodeQueue *nodeQ) {
  this->readyQueues.push_back(nodeQ);
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfScheduler::setExecPriority(DagNode *node) {
  // by default does nothing
}
/**
 * set node priority before inserting back in the ready queue
 */
void
MultiWfScheduler::setWaitingPriority(DagNode *node) {
  // by default does nothing
}

/**
 * Get the current time from scheduler reference clock
 */
double
MultiWfScheduler::getRelCurrTime() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return (double) ((current_time.tv_sec - refTime.tv_sec) * 1000
                   + (current_time.tv_usec - refTime.tv_usec) / 1000);
}

/**
 * Execute a post operation on synchronisation semaphore
 */
void
MultiWfScheduler::wakeUp(bool newDag, DagNode *node) {
  // store information in the FIFO list
  wakeUpInfo_t info;
  info.isNewDag = newDag;
  info.nodeRef = node;
  myWakeUpLock.lock();
  myWakeUpList.push_back(info);
  myWakeUpLock.unlock();
  // call POST on semaphore
  this->mySem.post();
} // wakeUp

/**
 * Call the LogService when a node becomes ready for queueing
 * (happens when node has no more pending dependencies)
 */
void
MultiWfScheduler::handlerNodeReady(DagNode *node) {
  // DietLogComponent* LC = myMaDag->getDietLogComponent();
  // if (LC) {
  // LC->logWfNodeReady(node->getDag()->getId().c_str(),
  // node->getId().c_str());
  // }
  sendEventFrom<DagNode, DagNode::READY>(node, "Node ready", "",
                                         EventBase::INFO);
}

/**
 * manage dag end of execution
 * Can handle several calls for the same dag (may happen if several nodes from
 * the same dag are cancelled within a short timeframe)
 */
void
MultiWfScheduler::handlerDagDone(Dag *dag) {
  myWakeUpLock.lock();
  myDagsTermList.push_back(dag->getId());
  myWakeUpLock.unlock();
}

/**
 * manage threads (nodes) termination
 */
void
MultiWfScheduler::postWakeUp() {
  // MANAGE NODE TERMINATION (if not waking up on new dag submission)
  myWakeUpLock.lock();
  if (!myWakeUpList.empty()) {
    wakeUpInfo_t &info = myWakeUpList.front();
    myWakeUpLock.unlock();
    if (info.isNewDag) {
      TRACE_TEXT(TRACE_MAIN_STEPS, "Scheduler waking up (NEW DAG)\n");
    } else {
      TRACE_TEXT(TRACE_MAIN_STEPS, "Scheduler waking up (END OF NODE)\n");
      if (info.nodeRef) {
        info.nodeRef->terminate();
      } else {
        INTERNAL_ERROR(__FUNCTION__ << "Invalid terminating node reference\n",
                       1);
      }
    }
    myWakeUpLock.lock();
    myWakeUpList.pop_front();
  }
  myWakeUpLock.unlock();
} // postWakeUp

/**
 * manage dags termination
 */
void
MultiWfScheduler::checkDagsRelease() {
  // MANAGE DAG TERMINATION
  myWakeUpLock.lock();
  if (!myDagsTermList.empty()) {
    myDagsTermList.sort();
    myDagsTermList.unique();  // removes consecutive duplicates
    std::list<std::string>::iterator it = myDagsTermList.begin();
    while (it != myDagsTermList.end()) {
      Dag *currDag = getDag(*it);
      TRACE_TEXT(TRACE_ALL_STEPS,
                 "Dag " << currDag->getId() << " : try to release\n");
      if (!currDag->isRunning()) {
        myWakeUpLock.unlock();
        releaseDag(currDag);
        myWakeUpLock.lock();
        it = myDagsTermList.erase(it);
      } else {
        TRACE_TEXT(TRACE_ALL_STEPS,
                   "Dag " << currDag->getId() << " : cannot release now\n");
        ++it;
      }
    }
  }
  myWakeUpLock.unlock();
} // checkDagsRelease

/**
 * release dag on client mgr
 */
void
MultiWfScheduler::releaseDag(Dag *dag) {
  typedef size_t comm_failure_t;
  std::string dagId = dag->getId();
  char *message;
  MetaDag *metaDag = this->getMetaDag(dag);

  // RELEASE THE CLIENT MANAGER

  TRACE_TEXT(TRACE_MAIN_STEPS,
             "[Dag " << dagId << "] : calling client for release\n");
  CltMan_ptr cltMan = myMaDag->getCltMan(dag->getId());
  try {
    message = cltMan->release(dagId.c_str(), !dag->isCancelled());
    TRACE_TEXT(TRACE_ALL_STEPS, " Release message : " << message << "\n");
    // INFORM LOGMANAGER
#ifdef USE_LOG_SERIVCE
    if (myMaDag->dietLogComponent != NULL) {
      myMaDag->dietLogComponent->logDag(message);
    }
#endif
    stat_flush();
    delete message;
  } catch (CORBA::SystemException &e) {
    std::cout << "Caught a CORBA " << e._name() << " exception ("
              << e.NP_minorString() << ")\n";
    WARNING("Connection problems with Client occured - Release cancelled");
    dag->setAsCancelled(NULL);
    if (metaDag) {
      metaDag->setReleaseFlag(true);  // allow MaDag to destroy the metadag
    }
  }

  // DISPLAY DEBUG INFO
  if (!dag->isCancelled()) {
    TRACE_TEXT(TRACE_MAIN_STEPS, "############### DAG "
               << dagId << " IS DONE #########\n");
  } else {
    TRACE_TEXT(TRACE_MAIN_STEPS, "############### DAG "
               << dagId << " IS CANCELLED #########\n");
    // Display list of failed nodes
    const std::list<std::string> &failedNodes = dag->getNodeFailureList();
    for (std::list<std::string>::const_iterator iter = failedNodes.begin();
         iter != failedNodes.end();
         ++iter) {
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "Dag " << dagId << " FAILED NODE : " << *iter << "\n");
    }
  }

  // REMOVE from dag list
  myDags.erase(dagId);

  // IF DAG IS CANCELLED, PROPAGATE CANCELLATION TO THE METADAG
  if ((metaDag != NULL) && dag->isCancelled()) {
    metaDag->cancelAllDags(this);
  }

  // DELETE DAG or METADAG
  if (metaDag != NULL) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Trigger end-of-dag event to MetaDag\n");
    metaDag->handlerDagDone(dag);
    myMetaDags.erase(dag->getId());
    if (metaDag->isDone()) {
      TRACE_TEXT(TRACE_ALL_STEPS, "######## META-DAG "
                 << metaDag->getId() << " IS COMPLETED #########\n");
      delete metaDag;
    }
  } else {
    TRACE_TEXT(TRACE_ALL_STEPS, "Deleting dag\n");
    delete dag;
  }
} // releaseDag

/**
 * Cancel dag (without stopping running tasks)
 */
void
MultiWfScheduler::cancelDag(const std::string &dagId) {
  TRACE_TEXT(TRACE_ALL_STEPS, "######## RECEIVED DAG CANCELLATION FOR DAG '"
             << dagId << "' #########\n");
  myLock.lock();
  try {
    getDag(dagId)->setAsCancelled(this);
  } catch (...) {
    myLock.unlock();
    throw;
  }
  myLock.unlock();
} // cancelDag

/****************************************************************************/
/*                            CLASS DagState                                */
/****************************************************************************/

/**
 * DagState default constructor
 */
DagState::DagState() {
  this->EFT = -1;
  this->makespan = -1;
  this->estimatedDelay = 0;
  this->slowdown = 0;
}
