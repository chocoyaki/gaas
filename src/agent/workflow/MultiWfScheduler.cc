/****************************************************************************/
/* The base class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.37  2008/09/30 09:25:34  bisnard
 * use Node::initProfileSubmit to create the diet profile before requesting node estimation to MA
 *
 * Revision 1.36  2008/09/19 13:12:48  bisnard
 * add debug info
 *
 * Revision 1.35  2008/09/04 14:33:55  bisnard
 * - New option for MaDag to select platform type (servers
 * with same service list or not)
 * - Optimization of the multiwfscheduler to avoid requests to
 * MA for server availability
 *
 * Revision 1.34  2008/09/03 09:27:54  bisnard
 * Temporary fix to reduce nb of submits
 *
 * Revision 1.33  2008/07/18 12:21:33  rbolze
 * correct nodeTodoCount value
 *
 * Revision 1.32  2008/07/17 15:01:43  rbolze
 * change position of the stat_info call
 *
 * Revision 1.31  2008/07/17 13:33:09  bisnard
 * New multi-wf heuristic SRPT
 *
 * Revision 1.30  2008/07/17 12:13:39  bisnard
 * Added stats on nb of total nodes to execute
 * Added CORBA exceptions handling for client release
 *
 * Revision 1.29  2008/07/17 10:49:14  rbolze
 * change fflush(stdout) by stat_flush()
 *
 * Revision 1.28  2008/07/17 10:14:36  rbolze
 * add some stat_info
 *
 * Revision 1.27  2008/07/12 00:22:28  rbolze
 * add function getInterRoundDelay()
 * use this function when the maDag start to display this value.
 * display the dag_id when compute the ageFactor in AgingHEFT
 * add some stats info :
 * 	queuedNodeCount
 * 	change MA DAG to MA_DAG
 *
 * Revision 1.26  2008/07/11 07:56:05  bisnard
 * provide list of failed nodes in case of cancelled dag
 *
 * Revision 1.25  2008/07/10 11:42:20  bisnard
 * Fix bug 68 memory loss during workflow execution
 *
 * Revision 1.24  2008/07/08 15:52:03  bisnard
 * Set interRoundDelay as parameter of workflow scheduler
 *
 * Revision 1.23  2008/07/08 11:13:53  bisnard
 * Dag cancellation cleanup
 * Raise exception when xml incorrect
 * Delete dag when completed
 *
 * Revision 1.22  2008/07/07 16:18:46  bisnard
 * Removed clientmgr ping before node execution
 *
 * Revision 1.21  2008/07/07 09:40:44  bisnard
 * use SeD CORBA ref instead of hostname to check ressource availability
 *
 * Revision 1.20  2008/07/04 13:30:15  bisnard
 * Avoid crash when connection to client is lost
 *
 * Revision 1.19  2008/06/25 10:05:44  bisnard
 * - Waiting priority set when node is put back in waiting queue
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 * - HEFT scheduler uses SeD ref instead of hostname
 * - Estimation vector and ReqID passed to client when SeD chosen by MaDag
 * - New params in execNodeOnSeD to provide ReqId and estimation vector
 * to client for solve request
 *
 * Revision 1.18  2008/06/19 10:18:54  bisnard
 * new heuristic AgingHEFT for multi-workflow scheduling
 *
 * Revision 1.17  2008/06/18 15:03:09  bisnard
 * use milliseconds instead of seconds in timestamps
 * new handler method when node is waiting in queue
 * set NodeRun class as friend to protect handler methods
 *
 * Revision 1.16  2008/06/17 10:15:36  bisnard
 * Corrected bug in execution loop
 *
 * Revision 1.15  2008/06/04 07:52:36  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.14  2008/06/03 13:37:09  bisnard
 * Multi-workflow sched now keeps nodes in the ready nodes queue
 * until a ressource is available to ensure comparison is done btw
 * nodes of different workflows (using sched-specific metric).
 *
 * Revision 1.13  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.12  2008/06/01 14:06:57  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.11  2008/06/01 09:21:35  rbolze
 * the remote method release now return a string which contains
 * feedback on the dag execution
 * it is send through logservice via the logDag method
 *
 * Revision 1.10  2008/05/30 14:22:48  bisnard
 * obsolete MultiDag
 *
 * Revision 1.9  2008/05/30 14:16:25  bisnard
 * obsolete MultiDag (not used anymore for multi-wf)
 *
 * Revision 1.8  2008/05/30 13:22:19  bisnard
 * added micro-delay between workflow node executions to avoid interf
 *
 * Revision 1.7  2008/05/16 12:30:20  bisnard
 * MaDag returns dagID to client after dag submission
 * (used for node execution)
 *
 * Revision 1.6  2008/04/30 07:37:01  bisnard
 * use relative timestamps for estimated and real completion time
 * make MultiWfScheduler abstract and add HEFT MultiWf scheduler
 *
 * Revision 1.5  2008/04/28 12:12:44  bisnard
 * new NodeQueue implementation for FOFT
 * manage thread join after node execution
 * compute slowdown for FOFT
 *
 * Revision 1.4  2008/04/21 14:31:45  bisnard
 * moved common multiwf routines from derived classes to MultiWfScheduler
 * use wf request identifer instead of dagid to reference client
 * use nodeQueue to manage multiwf scheduling
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/15 14:20:19  bisnard
 * - Postpone sed mapping until wf node is executed
 *
 * Revision 1.2  2008/04/14 09:10:37  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 09:13:29  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#include "MultiWfScheduler.hh"
#include "HEFTScheduler.hh"
#include "MaDag_impl.hh"
#include "marshalling.hh"
#include "est_internal.hh"
#include "debug.hh"
#include "statistics.hh"

using namespace madag;

/****************************************************************************/
/*                                                                          */
/*                      CLASS MultiWfScheduler                              */
/*                                                                          */
/****************************************************************************/


/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfScheduler::MultiWfScheduler(MaDag_impl* maDag, nodePolicy_t nodePol)
  : mySem(0), myMaDag(maDag), nodePolicy(nodePol), interRoundDelay(100),
    platformType(PFM_ANY) {
  this->mySched   = new HEFTScheduler();
  this->execQueue = NULL; // must be initialized in derived class constructor
  gettimeofday(&this->refTime, NULL); // init reference time
}

MultiWfScheduler::~MultiWfScheduler() {
  if (this->mySched != NULL)
    delete this->mySched;
}

long MultiWfScheduler::dagIdCounter = 0;

/**
 * Change the intra-dag scheduler (by default it is HEFT)
 */
void
MultiWfScheduler::setSched(WfScheduler * sched) {
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
const MaDag_impl*
MultiWfScheduler::getMaDag() const {
  return this->myMaDag;
}

/**
 * Process a new dag => when finished the dag is ready for execution
 */
double
MultiWfScheduler::scheduleNewDag(const corba_wf_desc_t& wf_desc, int wfReqId,
                                 MasterAgent_var MA)
    throw (XMLParsingException, NodeException)
{
  // Beginning of exclusion block
  // TODO move exclusion lock later (need to make HEFTScheduler thread-safe)
  this->myLock.lock();

  TRACE_TEXT (TRACE_ALL_STEPS, "The meta scheduler receives a new xml dag "
      << endl);
  // Dag XML Parsing
  Dag * newDag;
  newDag = this->parseNewDag(wfReqId, wf_desc); // dagId is set here

  // Dag internal scheduling
  TRACE_TEXT (TRACE_ALL_STEPS, "Making intra-dag schedule" << endl);
  this->intraDagSchedule(newDag, MA);

  // Node queue creation (to manage ready nodes queueing)
  TRACE_TEXT (TRACE_ALL_STEPS, "Initializing new ready nodes queue" << endl);
  OrderedNodeQueue * readyNodeQ = this->createNodeQueue(newDag);

  // Init queue by setting input nodes as ready
  newDag->setInputNodesReady();

  // Insert node queue into pool of node queues managed by the scheduler
  TRACE_TEXT (TRACE_ALL_STEPS, "Inserting new node queue into queue pool" << endl);
  this->insertNodeQueue(readyNodeQ);

  // Set starting time of the DAG
  newDag->setStartTime(this->getRelCurrTime());

  // Send signal to scheduler thread to inform there are new nodes
  TRACE_TEXT( TRACE_MAIN_STEPS, "%%%%% NEW DAG SUBMITTED: dag id = " << newDag->getId() << endl);
  this->wakeUp();

  // End of exclusion block
  this->myLock.unlock();

  return atof(newDag->getId().c_str());
}

/**
 * Execution method
 */
void*
MultiWfScheduler::run() {
  int loopCount = 0;
  // the ressource availability matrix
  map<SeD_ptr, bool> ressAvail;
  // the service availability matrix
  map<string, bool> servAvail;
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
  }

  TRACE_TEXT(TRACE_ALL_STEPS,"Multi-Workflow scheduler is running" << endl);
  /// Start a ROUND of node ordering & mapping
  /// New rounds are started as long as some nodes can be mapped to ressources
  /// (if no more ressources then we wait until a node is finished or a new dag
  /// is submitted)
  while (true) {
    loopCount++;

    int queuedNodeCount = 0;   // nb nodes put in execQueue (check for new round)
    int mappedNodeCount = 0;   // nb nodes mapped to a ressource (check for new round)
    int servAvailCount = 0; // nb available services (check for new round)

    int nodeReadyCount = 0;    // the nb of nodes ready (statistics)
    int dagCount = 0;          // the nb of dag (statistics)
    int nodeTodoCount = 0;     // the nb of nodes still to be executed (statistics)

    TRACE_TEXT(TRACE_MAIN_STEPS,"\t ** Starting Multi-Workflow scheduler ("
        << loopCount << ") time=" << this->getRelCurrTime() << endl);
    this->myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    TRACE_TEXT(TRACE_ALL_STEPS,"PHASE 1: Move ready nodes to exec queue" << endl);


    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      OrderedNodeQueue * readyQ = *qp;
      int npc = nodePolicyCount;
      Node * n = NULL;
      nodeReadyCount+=(int)readyQ->size();
      dagCount++;
      ChainedNodeQueue * waitQ = this->waitingQueues[readyQ];
      nodeTodoCount+= (int)readyQ->size() + (int)waitQ->size(); // for stats only
      while ((npc) && (n = readyQ->popFirstNode())) {
        TRACE_TEXT(TRACE_MAIN_STEPS,"  #### Ready node : " << n->getCompleteId()
            << " / request #" << n->getWfReqId()
            << " / wait prio = " << n->getPriority() << endl);
        // save the address of the readyQ for this node (used if node pushed back)
        n->setLastQueue(readyQ);
        // set priority of node (depends on choosen algorithm)
        this->setExecPriority(n);
        // insert node into execution queue
        execQueue->pushNode(n);
        queuedNodeCount++;
        npc--;
      }
      ++qp; // go to next queue
    }
    //only write stats when there is something to stats.
    if(dagCount>0){
	sprintf(statMsg, "dagCount %d", dagCount);
	stat_info("MA_DAG",statMsg);
	sprintf(statMsg, "nodeReadyCount %d", nodeReadyCount);
	stat_info("MA_DAG",statMsg);
	sprintf(statMsg, "queuedNodeCount %d", queuedNodeCount);
        stat_info("MA_DAG",statMsg);
        sprintf(statMsg, "nodeTodoCount %d", nodeTodoCount);
	stat_info("MA_DAG",statMsg);
    }
    if (queuedNodeCount > 0) {
      TRACE_TEXT(TRACE_ALL_STEPS,"Phase 2: Check ressources for nodes in exec queue ("
          << queuedNodeCount << " nodes)" << endl);
      int requestCount = 0;

      switch(this->platformType) {
        case PFM_ANY:
          // Initialize service availability matrix
          for (list<Node *>::iterator nodeIter = execQueue->begin();
               nodeIter != execQueue->end();
               ++nodeIter) {
            servAvail[(*nodeIter)->getPb()] = true;
            servAvailCount = servAvail.size();
          }
          TRACE_TEXT(TRACE_ALL_STEPS, "Nb of distinct services in queue: "
              << servAvailCount << endl);
          break;
        case PFM_SAME_SERVICES:
          TRACE_TEXT(TRACE_ALL_STEPS,
            "Limiting check to one ressource (same services on all ress.)" << endl);
      }

      while (!execQueue->isEmpty()) {
        Node *n = execQueue->popFirstNode();
        OrderedNodeQueue * readyQ = dynamic_cast<OrderedNodeQueue *>(n->getLastQueue());
        bool ressourceFound = false;
        int  submitReqID = 0;  // store ReqID of submit to provide it for solve
        corba_server_estimation_t* servEst;
        // Test to process node (depends on platform type)
        bool nodeSubmit = ((this->platformType == PFM_ANY)
                            && (servAvailCount) && (servAvail[n->getPb()]))
            || ((this->platformType == PFM_SAME_SERVICES) && (requestCount < 1));

        if (nodeSubmit) {
          TRACE_TEXT(TRACE_MAIN_STEPS,"Submit request for node " << n->getCompleteId()
            << "(" << n->getPb() << ") / exec prio = " << n->getPriority() << endl);

          // SEND REQUEST TO PLATFORM (FOR CURRENT NODE)
          wf_response_t *  wf_response = getProblemEstimates(n, myMaDag->getMA());
          if (wf_response == NULL) {
            cout << "ERROR during MA submission" << endl;
            continue;
          }
          ++requestCount;
          // CHECK RESSOURCE AVAILABILITY
          for (unsigned int jx=0;
               jx < wf_response->wfn_seq_resp[0].response.servers.length();
               jx++) { // loop over servers
            servEst = &wf_response->wfn_seq_resp[0].response.servers[jx];
            double compTime = diet_est_get_internal(&servEst->estim, EST_TCOMP, 0);
            double EFT = diet_est_get_internal(&servEst->estim, EST_USERDEFINED, 0);
            SeD_ptr curSeDPtr = servEst->loc.ior;
            string hostname(CORBA::string_dup(servEst->loc.hostName));
            TRACE_TEXT(TRACE_ALL_STEPS,"  server " << hostname << ": compTime="
                       << compTime << ": EFT=" << EFT << endl);
            if (EFT - compTime <= 0) {  // test if available right now
              // test if the server has not been already chosen for another node
              bool ressAvailable = true;
              for (map<SeD_ptr, bool>::iterator ressAvailIter=ressAvail.begin();
                   ressAvailIter!=ressAvail.end();
                   ressAvailIter++) {
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
                TRACE_TEXT(TRACE_ALL_STEPS,"  server found: " << hostname << endl);
                break;
              } // end if
            } // end if
          } // end for jx

          // EXECUTE NODE (NEW THREAD)
          if (ressourceFound) {
            n->setAsRunning();
            n->setRealStartTime(this->getRelCurrTime());
	    TRACE_TEXT(TRACE_MAIN_STEPS,"  $$$$ Exec node on " << servEst->loc.hostName
              << " : " << n->getCompleteId() << endl);
            mappedNodeCount++;
            runNode(n, servEst->loc.ior, submitReqID, servEst->estim);
          } else {
            if (this->platformType == PFM_ANY) {
              servAvail[n->getPb()] = false;
              servAvailCount--;
              TRACE_TEXT(TRACE_MAIN_STEPS,"Service " << n->getPb() << " is not available" << endl);
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
        OrderedNodeQueue * curReadyQ = *qp2;
        ChainedNodeQueue * curWaitQ = waitingQueues[curReadyQ];
        if (curWaitQ->isEmpty() && curReadyQ->isEmpty()) {
          TRACE_TEXT(TRACE_ALL_STEPS,"Node Queues are empty: remove & destroy" << endl);
          qp2 = readyQueues.erase(qp2);       // removes from the list
          this->deleteNodeQueue(curReadyQ);   // deletes both queues
        } else {
          qp2++;
        }
      } // end while
      // Round-robbin on remaining queues
      if (readyQueues.size() > 0) {
        OrderedNodeQueue * firstReadyQueue = readyQueues.front();
        readyQueues.pop_front();
        readyQueues.push_back(firstReadyQueue);
      }
    } // end if queuedNodeCount > 0

    this->myLock.unlock();

    // The condition to go for a new round is the availability of ressources: under the
    // hypothesis that all ressources are identical (in terms of provided services) for
    // a given dag then there may be available ressources only if all nodes in the
    // execQueue were assigned a ressource.
//     if ((queuedNodeCount == 0) || (queuedNodeCount > mappedNodeCount)) {
    if ((queuedNodeCount == 0)
         || ((this->platformType == PFM_ANY) && (servAvailCount == 0))
         || ((this->platformType == PFM_SAME_SERVICES) && (mappedNodeCount == 0))) {
      if (queuedNodeCount == 0) {
        TRACE_TEXT(TRACE_MAIN_STEPS,"No ready nodes - sleeping" << endl);
      } else {
        TRACE_TEXT(TRACE_MAIN_STEPS,"No ressource available - sleeping" << endl);
      }
      this->mySem.wait();
      if (this->termNode) {
        TRACE_TEXT(TRACE_ALL_STEPS,"Joining RunNode thread ("
            << this->termNodeThread << ")" << endl);
        this->termNodeThread->join();
        delete this->termNodeThread;
        this->termNode = false;
      }
    } else {
      // DELAY between rounds (to avoid interference btw submits)
      usleep(this->interRoundDelay * 1000);
    } // end if
  } // end while (true)
}

/**
 * Execute a post operation on synchronisation semaphore
 */
void
MultiWfScheduler::wakeUp() {
  TRACE_TEXT(TRACE_ALL_STEPS,"Wake Up" << endl);
  this->termNode = false; // no thread to join
  this->mySem.post();
}

/**
 * Execute a post operation on synchronisation semaphore
 * and joins the node thread given as parameter
 */
void
MultiWfScheduler::wakeUp(NodeRun * nodeThread) {
  TRACE_TEXT(TRACE_ALL_STEPS,"Wake Up & Join" << endl);
  this->termNodeThread = nodeThread;
  this->termNode = true;
  this->mySem.post();
}

/**
 * Get the current time from scheduler reference clock
 */
double
MultiWfScheduler::getRelCurrTime() {
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return (double) ((current_time.tv_sec - refTime.tv_sec)*1000
      + (current_time.tv_usec - refTime.tv_usec)/1000);
}

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

/**
 * Parse dag xml description and create a dag object
 */
Dag *
MultiWfScheduler::parseNewDag(int wfReqId, const corba_wf_desc_t& wf_desc)
    throw (XMLParsingException) {
  DagWfParser* reader = new DagWfParser(wfReqId, wf_desc.abstract_wf);
  if (!reader->setup()) {
      throw XMLParsingException(XMLParsingException::eBAD_STRUCT);
  }
  Dag * newDag = reader->getDag();
  delete reader;
  newDag->setId(itoa(dagIdCounter++));
  return newDag;
}

/**
 * Call MA to get server estimations for all services for nodes of a Dag
 * Note: creates the node profiles
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(Dag *dag, MasterAgent_var MA)
    throw (NodeException) {
  // Check that all services are available and get the estimations (with MA)
  TRACE_TEXT (TRACE_ALL_STEPS,"MultiWfScheduler: Marshalling the profiles" << endl);
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  pbs_seq->length(dag->size());
  int ix = 0;
  for (map<std::string, Node *>::iterator iter = dag->begin();
       iter != dag->end(); iter++) {
         Node * node = (Node *) iter->second;
         cout << "Node #" << ix << " (" << node->getCompleteId() << ")" << endl;
         node->initProfileSubmit(); // creates the diet profile
         node->setSubmitIndex(ix); // used to find response
         mrsh_pb_desc(&(*pbs_seq)[ix++], node->getProfile());
  }
  TRACE_TEXT (TRACE_ALL_STEPS,
              "MultiWfScheduler: send " << ix << " profile(s) to the MA  ... "
                  << endl);
  wf_response_t * wf_response = MA->submit_pb_set(*pbs_seq);
  delete pbs_seq;
  TRACE_TEXT (TRACE_ALL_STEPS, "... done" << endl);
  if ( ! wf_response->complete)
    throw (NodeException(NodeException::eSERVICE_NOT_FOUND));
  return wf_response;
}

/**
 * Call MA to get server estimations for all services for nodes of a NodeQueue
 * @deprecated
 */
/*
wf_response_t *
MultiWfScheduler::getProblemEstimates(OrderedNodeQueue* queue, MasterAgent_var MA)
    throw (NodeException) {
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  pbs_seq->length(queue->size());
  int ix = 0;
  for (list<Node *>::iterator iter = queue->begin();
       iter != queue->end(); iter++) {
         Node * node = (Node *) *iter;
         node->setSubmitIndex(ix); // used to find response
         mrsh_pb_desc(&(*pbs_seq)[ix++], node->getProfile());
  }
  TRACE_TEXT (TRACE_ALL_STEPS,
              "MultiWfScheduler: send " << ix << " profile(s) to the MA  ... "
                  << endl);
  wf_response_t * wf_response = MA->submit_pb_set(*pbs_seq);
  delete pbs_seq;
  TRACE_TEXT (TRACE_ALL_STEPS, "... done" << endl);
  if ( ! wf_response->complete)
    throw (NodeException(NodeException::eSERVICE_NOT_FOUND));
  return wf_response;
} */

/**
 * Call MA to get server estimations for one node
 * Note: the profile for the node is supposed to be already created
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(Node *node, MasterAgent_var MA)
    throw (NodeException) {
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  pbs_seq->length(1);
  node->setSubmitIndex(0);
  mrsh_pb_desc(&(*pbs_seq)[0], node->getProfile());
  TRACE_TEXT (TRACE_ALL_STEPS, "MultiWfScheduler: send 1 profile to the MA  ... "
                  << endl);
  wf_response_t * wf_response = MA->submit_pb_set(*pbs_seq);
  delete pbs_seq;
  TRACE_TEXT (TRACE_ALL_STEPS, "... done" << endl);
  if ( ! wf_response->complete)
    throw (NodeException(NodeException::eSERVICE_NOT_FOUND));
  return wf_response;
}

/**
 * Intra-dag scheduling
 */
void
MultiWfScheduler::intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw (NodeException) {
  // Call the MA to get estimations for all services
  wf_response_t * wf_response = this->getProblemEstimates(dag, MA);
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
MultiWfScheduler::createNodeQueue(Dag * dag)  {
  TRACE_TEXT (TRACE_ALL_STEPS, "Creating new node queues (priority-based)" << endl);
  OrderedNodeQueue *  readyQ  = new PriorityNodeQueue();
  ChainedNodeQueue *  waitQ   = new ChainedNodeQueue(readyQ);
  for (std::map <std::string, Node *>::iterator nodeIt = dag->begin();
       nodeIt != dag->end();
       nodeIt++) {
    waitQ->pushNode(&(*nodeIt->second));
  }
  this->waitingQueues[readyQ] = waitQ; // used to destroy waiting queue
  return readyQ;
}

/**
 * Delete the two chained node queues
 *  - WAITING queue => READY queue
 */
void
MultiWfScheduler::deleteNodeQueue(OrderedNodeQueue * nodeQ) {
  TRACE_TEXT (TRACE_ALL_STEPS, "Deleting node queues" << endl);
  ChainedNodeQueue *  waitQ = waitingQueues[nodeQ];
  waitingQueues.erase(nodeQ);     // removes from the map
  PriorityNodeQueue * readyQ  = dynamic_cast<PriorityNodeQueue *>(nodeQ);
  delete waitQ;
  delete readyQ;
}

/**
 * Insert new node queue into ready queues list
 */
void
MultiWfScheduler::insertNodeQueue(OrderedNodeQueue * nodeQ) {
  this->readyQueues.push_back(nodeQ);
}

/**
 * set node priority before inserting into execution queue
 */
void
MultiWfScheduler::setExecPriority(Node * node) {
  // by default does nothing
}
/**
 * set node priority before inserting back in the ready queue
 */
void
MultiWfScheduler::setWaitingPriority(Node * node) {
  // by default does nothing
}

/**
 * Execute a node on a given SeD
 */
Thread *
MultiWfScheduler::runNode(Node * node, SeD_var sed,
                          int reqID, corba_estimation_t& ev) {
  Thread * thread = new NodeRun(node, sed, reqID, ev, this,
                                this->myMaDag->getCltMan(node->getWfReqId()));
  thread->start();
  return thread;
}

/**
 * Execute a node without specifying a SeD
 */
Thread *
MultiWfScheduler::runNode(Node * node) {
  Thread * thread = new NodeRun(node, this,
                                this->myMaDag->getCltMan(node->getWfReqId()));
  thread->start();
  return thread;
}

/****************************************************************************/
/*                            CLASS NodeRun                                 */
/****************************************************************************/

NodeRun::NodeRun(Node * node, SeD_var sed, int reqID, corba_estimation_t ev,
                 MultiWfScheduler * scheduler, CltMan_ptr cltMan) {
  this->myNode = node;
  this->mySeD = sed;
  this->myReqID = reqID;
  this->myEstVect = ev;
  this->myScheduler = scheduler;
  this->myCltMan = cltMan;
}

NodeRun::NodeRun(Node * node,
                 MultiWfScheduler * scheduler, CltMan_ptr cltMan) {
  this->myNode = node;
  this->mySeD = SeD::_nil();
  this->myScheduler = scheduler;
  this->myCltMan = cltMan;
}

/**
 * The execution method
 */
void*
NodeRun::run() {
  typedef size_t comm_failure_t;
  string dag_id = this->myNode->getDag()->getId();
  TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: running node "
      << this->myNode->getCompleteId() << endl);
  if (myCltMan != CltMan::_nil()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: try to call client manager ");
    CORBA::Long res;
    bool clientFailure = false;

    // NODE EXECUTION ON CLIENT
    try {
      try {
        if (!CORBA::is_nil(this->mySeD)) {
          TRACE_TEXT (TRACE_ALL_STEPS, "(exec on sed - request #"
            << this->myReqID << ")" << endl);
          res = myCltMan->execNodeOnSed(this->myNode->getId().c_str(),
                                    this->myNode->getDag()->getId().c_str(),
                                    this->mySeD,
                                    this->myReqID,
                                    this->myEstVect);
        } else {
          TRACE_TEXT (TRACE_ALL_STEPS, "(exec without sed)" << endl);
          res = myCltMan->execNode(this->myNode->getId().c_str(),
                         this->myNode->getDag()->getId().c_str());
        }
      } catch (CORBA::COMM_FAILURE& e) {
        throw (comm_failure_t)1;
      } catch (CORBA::TRANSIENT& e) {
        throw (comm_failure_t)1;
      }
    } catch (comm_failure_t& e) {
      if (e == 0 || e == 1) {
        WARNING("Connection problems with Client occured - Dag cancelled");
        clientFailure = true;
        res = 1;
      }
    }
    // POST-PROCESSING

    if (res == 0) {
      TRACE_TEXT (TRACE_MAIN_STEPS, "NodeRun: node " << this->myNode->getCompleteId()
          << " is done" << endl);
      // update node status (must be called before handlerNodeDone to update realCompTime)
      this->myNode->setAsDone(this->myScheduler->getRelCurrTime());
      // inform scheduler that node is done (depending on the scheduler, this may trigger
      //  a recursive update of the realCompTime in other nodes)
      this->myScheduler->handlerNodeDone(myNode);

      // check if dag is completed and release client if yes
      if ((this->myNode->getDag() != NULL) &&
         (this->myNode->getDag()->isDone())) {
      	//this->myNode->getDag()->showDietReqID();
        // CALLING CLIENT MANAGER (CORBA AGENT) FOR RELEASE
        TRACE_TEXT (TRACE_MAIN_STEPS,"Dag " << this->myNode->getDag()->getId()
            << " completed - calling client for release");
        char * message;
        try {
          try {
            message = myCltMan->release(this->myNode->getDag()->getId().c_str());
          } catch (CORBA::COMM_FAILURE& e) {
            throw (comm_failure_t)1;
          } catch (CORBA::TRANSIENT& e) {
            throw (comm_failure_t)1;
          }
        } catch (comm_failure_t& e) {
          if (e == 0 || e == 1) {
            WARNING("Connection problems with Client occured - Release cancelled");
            clientFailure = true;
            this->myNode->getDag()->setAsCancelled();
          }
        }
        if (!clientFailure) {
          TRACE_TEXT (TRACE_ALL_STEPS," message : "<< message << endl);
          if (this->myScheduler->getMaDag()->dietLogComponent != NULL) {
            this->myScheduler->getMaDag()->dietLogComponent->logDag(message);
          }
	  // flush all stat
	  stat_flush();
          delete message;
          TRACE_TEXT (TRACE_MAIN_STEPS,"############### DAG "
              << this->myNode->getDag()->getId().c_str() <<" IS DONE #########"<<endl);
          delete this->myNode->getDag();
        }
      }
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: node " << this->myNode->getCompleteId()
          << " execution failed! " << endl
          << " ==> Cancelling DAG execution" << endl);
      this->myNode->setAsFailed(); // set the dag as cancelled
    }

    // Manage dag termination if a node failed (following code is executed
    // by the last running node)
    if (this->myNode->getDag()->isCancelled() && !this->myNode->getDag()->isRunning()) {
      string dagId = this->myNode->getDag()->getId();
      TRACE_TEXT (TRACE_MAIN_STEPS, "############## DAG " << dagId
          << " IS CANCELLED! #########" << endl);
      // Display list of failed nodes
      const std::list<string>& failedNodes = this->myNode->getDag()->getNodeFailureList();
      for (std::list<string>::const_iterator iter = failedNodes.begin();
           iter != failedNodes.end(); iter++) {
        TRACE_TEXT (TRACE_MAIN_STEPS, "DAG " << dagId << " FAILED NODE : " << *iter << endl);
      }
      // Release the client manager (if still alive)
      char* message;
      if (!clientFailure) {
        try {
          try {
            message = myCltMan->release(this->myNode->getDag()->getId().c_str());
          } catch (CORBA::COMM_FAILURE& e) {
            throw (comm_failure_t)1;
          } catch (CORBA::TRANSIENT& e) {
            throw (comm_failure_t)1;
          }
        } catch (comm_failure_t& e) {
          if (e == 0 || e == 1) {
            WARNING("Connection problems with Client occured - Release cancelled");
            clientFailure = true;
          }
        }
      }
      if (!clientFailure) {
        TRACE_TEXT (TRACE_ALL_STEPS," message : "<< message << endl);
	if (this->myScheduler->getMaDag()->dietLogComponent != NULL) {
		this->myScheduler->getMaDag()->dietLogComponent->logDag(message);
	}
        delete message;
      }
      // Delete dag
      delete this->myNode->getDag();
    }
  }
  else {
       TRACE_TEXT (TRACE_ALL_STEPS,"NodeRun: ERROR!! cannot contact the Client Wf Mgr" << endl);
  }
  fflush(stdout);
  this->myScheduler->wakeUp(this);
}

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

