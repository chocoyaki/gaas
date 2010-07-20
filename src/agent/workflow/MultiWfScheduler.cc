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
 * Revision 1.55  2010/07/20 16:41:30  bdepardo
 * Changed the initialization order of the attributes in
 * MultiWfScheduler::MultiWfScheduler to remove warnings.
 * Handle MULTIWF_NO_METRIC in a switch in run() to remove a warning.
 *
 * Revision 1.54  2010/07/20 08:59:36  bisnard
 * Added event generation
 *
 * Revision 1.53  2010/07/12 16:14:11  glemahec
 * DIET 2.5 beta 1 - Use the new ORB manager and allow the use of SSH-forwarders for all DIET CORBA objects
 *
 * Revision 1.52  2010/03/31 21:15:39  bdepardo
 * Changed C headers into C++ headers
 *
 * Revision 1.51  2010/03/08 13:43:23  bisnard
 * added new logged event when a wf node becomes ready
 *
 * Revision 1.50  2009/10/07 08:14:08  bisnard
 * modified trace messages
 *
 * Revision 1.49  2009/09/25 12:42:09  bisnard
 * - use new DagNodeLauncher classes to manage threads
 * - added dag cancellation method
 *
 * Revision 1.48  2009/06/23 09:25:38  bisnard
 * use new estimation vector entry (EST_EFT)
 *
 * Revision 1.47  2009/02/24 13:59:48  bisnard
 * modified trace messages
 *
 * Revision 1.46  2009/02/20 10:24:52  bisnard
 * use estimation class to reduce the nb of submit requests
 *
 * Revision 1.45  2009/02/06 14:50:35  bisnard
 * setup exceptions
 *
 * Revision 1.44  2009/01/16 13:41:22  bisnard
 * added common base class DagScheduler to simplify dag events handling
 * improved exception management
 *
 * Revision 1.43  2008/12/17 10:10:16  bisnard
 * corrected bug in handlerDagDone when using single dag submit
 *
 * Revision 1.42  2008/12/09 12:09:00  bisnard
 * added parameters to dag submit method to handle inter-dependent dags
 *
 * Revision 1.41  2008/12/02 14:17:44  bisnard
 * manage multi-dag cancellation when one dag fails
 *
 * Revision 1.40  2008/12/02 10:21:03  bisnard
 * use MetaDags to handle multi-dag submission and execution
 *
 * Revision 1.39  2008/10/20 07:56:43  bisnard
 * new classes XML parser (Dagparser,FWfParser)
 *
 * Revision 1.38  2008/10/14 13:24:49  bisnard
 * use new class structure for dags (DagNode,DagNodePort)
 *
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

#include "marshalling.hh"
#include "est_internal.hh"
#include <cmath>
#include <iomanip>
#include "debug.hh"
#include "statistics.hh"
#include "DietLogComponent.hh"

#include "MaDag_impl.hh"
#include "MultiWfScheduler.hh"
#include "HEFTScheduler.hh"
#include "MaDagNodeLauncher.hh"
#include "Dag.hh"
#include "DagNode.hh"
#include "events/EventTypes.hh"

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

MultiWfScheduler::MultiWfScheduler(MaDag_impl* maDag, nodePolicy_t nodePol)
  : nodePolicy(nodePol), platformType(PFM_ANY), mySem(0), interRoundDelay(100), 
    myMaDag(maDag) {
  this->mySched   = new HEFTScheduler();
  this->execQueue = NULL; // must be initialized in derived class constructor
  gettimeofday(&this->refTime, NULL); // init reference time
}

MultiWfScheduler::~MultiWfScheduler() {
  if (this->mySched != NULL)
    delete this->mySched;
}

string 
MultiWfScheduler::toString() const
{
  return "MultiWfScheduler";
}

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

Dag *
MultiWfScheduler::getDag(const string& dagId) throw (MaDag::InvalidDag) {
  map<string,Dag*>::iterator iter = myDags.find(dagId);
  if (iter != myDags.end())
    return ((Dag*)iter->second);
  else
    throw (MaDag::InvalidDag(dagId.c_str()));
}

/**
 * get the metadag of a dag
 */
MetaDag*
MultiWfScheduler::getMetaDag(Dag * dag) {
  map<string,MetaDag*>::iterator iter = myMetaDags.find(dag->getId());
  if (iter != myMetaDags.end())
    return ((MetaDag*)iter->second);
  else
    return NULL;
}

/**
 * Process a new dag => when finished the dag is ready for execution
 */
void
MultiWfScheduler::scheduleNewDag(Dag * newDag, MetaDag * metaDag)
    throw (MaDag::ServiceNotFound, MaDag::CommProblem)
{
  // Beginning of exclusion block
  // TODO move exclusion lock later (need to make HEFTScheduler thread-safe)

  TRACE_TEXT(TRACE_MAIN_STEPS,"\t ** New DAG to schedule (" << newDag->getId()
              << ") time=" << this->getRelCurrTime() << endl);

  // Dag internal scheduling
  TRACE_TEXT (TRACE_ALL_STEPS, "Making intra-dag schedule" << endl);
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
  TRACE_TEXT (TRACE_ALL_STEPS, "Initializing new ready nodes queue" << endl);
  OrderedNodeQueue * readyNodeQ = this->createNodeQueue(newDag);

  // Init queue by setting input nodes as ready
  newDag->setInputNodesReady(this);

  // Insert node queue into pool of node queues managed by the scheduler
  TRACE_TEXT (TRACE_ALL_STEPS, "Inserting new node queue into queue pool" << endl);
  this->insertNodeQueue(readyNodeQ);

  // Set starting time of the DAG
  newDag->setStartTime(this->getRelCurrTime());

  // Send signal to scheduler thread to inform there are new nodes
  TRACE_TEXT( TRACE_MAIN_STEPS, "%%%%% NEW DAG SUBMITTED: dag id = " << newDag->getId() << endl);
  this->wakeUp(true);

  myLock.unlock();
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
  case MULTIWF_NO_METRIC:
    break;
  }

  TRACE_TEXT(TRACE_ALL_STEPS,"Multi-Workflow scheduler is running" << endl);
  cout << setiosflags(ios_base::fixed) << setprecision(0);  // display double as int
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

    myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    TRACE_TEXT(TRACE_ALL_STEPS,"PHASE 1: Move ready nodes to exec queue" << endl);


    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      OrderedNodeQueue * readyQ = *qp;
      int npc = nodePolicyCount;
      DagNode * n = NULL;
      nodeReadyCount+=(int)readyQ->size();
      dagCount++;
      ChainedNodeQueue * waitQ = this->waitingQueues[readyQ];
      nodeTodoCount+= (int)readyQ->size() + (int)waitQ->size(); // for stats only
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
          for (list<DagNode *>::iterator nodeIter = execQueue->begin();
               nodeIter != execQueue->end();
               ++nodeIter) {
            servAvail[(*nodeIter)->getPbName()] = true;
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
        DagNode *n = execQueue->popFirstNode();
        OrderedNodeQueue * readyQ = dynamic_cast<OrderedNodeQueue *>(n->getLastQueue());
        bool ressourceFound = false;
        int  submitReqID = 0;  // store ReqID of submit to provide it for solve
        corba_server_estimation_t* servEst;
        // Test to process node (depends on platform type)
        bool nodeSubmit = ((this->platformType == PFM_ANY)
                            && (servAvailCount) && (servAvail[n->getPbName()]))
            || ((this->platformType == PFM_SAME_SERVICES) && (requestCount < 1));

        if (nodeSubmit) {
          TRACE_TEXT(TRACE_MAIN_STEPS,"Submit request for node " << n->getCompleteId()
            << "(" << n->getPbName() << ") / exec prio = " << n->getPriority() << endl);

          // SEND REQUEST TO PLATFORM (FOR CURRENT NODE)
          wf_response_t *  wf_response = NULL;
          try {
            wf_response= getProblemEstimates(n, myMaDag->getMA());
          } catch (...) {
            cerr << "ERROR during MA submission" << endl;
            n->setAsFailed();
            continue;
          }
          ++requestCount;
          // CHECK RESSOURCE AVAILABILITY
          for (unsigned int jx=0;
               jx < wf_response->wfn_seq_resp[0].response.servers.length();
               jx++) { // loop over servers
            servEst = &wf_response->wfn_seq_resp[0].response.servers[jx];
            double compTime = diet_est_get_internal(&servEst->estim, EST_TCOMP, 0);
            double EFT = diet_est_get_internal(&servEst->estim, EST_EFT, 0);
            if ((compTime == HUGE_VAL) || (EFT == HUGE_VAL)) {
              WARNING("SeD estimation function does not provide correct values for "
                      << "computation time (EST_COMPTIME) and EFT (EST_EFT)");
            }

            string sedName = string(servEst->loc.SeDName);
	    SeD_ptr curSeDPtr = ORBMgr::getMgr()->resolve<SeD, SeD_ptr>(SEDCTXT, sedName);
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
	    TRACE_TEXT(TRACE_MAIN_STEPS,"  $$$$ Exec node on " << servEst->loc.hostName
              << " : " << n->getCompleteId() << endl);
            mappedNodeCount++;
            // create a node launcher
            DagNodeLauncher *launcher = new MaDagNodeLauncher( n, this,
                                              myMaDag->getCltMan(n->getDag()->getId()));
            // record chosen SeD information
            launcher->setSeD(servEst->loc.SeDName, submitReqID, servEst->estim);
            // start node execution
            n->setRealStartTime(this->getRelCurrTime());
            n->start(launcher); // non-blocking
          } else {
            if (this->platformType == PFM_ANY) {
              servAvail[n->getPbName()] = false;
              servAvailCount--;
              TRACE_TEXT(TRACE_MAIN_STEPS,"Service " << n->getPbName() << " is not available" << endl);
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

    myLock.unlock();

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

      // WAIT UNTIL A NEW DAG IS SUBMITTED OR A NODE IS COMPLETED

      this->postWakeUp();
      this->checkDagsRelease();

    } else {
      // DELAY between rounds (to avoid interference btw submits)
      usleep(this->interRoundDelay * 1000);
    } // end if
  } // end while (true)
}

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
    throw (MaDag::ServiceNotFound, MaDag::CommProblem) {
  // Check that all services are available and get the estimations (with MA)
  TRACE_TEXT (TRACE_ALL_STEPS,"MultiWfScheduler: Marshalling the profiles" << endl);
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  wf_response_t * wf_response = NULL;
  pbs_seq->length(dag->size());
  // Create a mapping table (estimation class => submission index)
  map<string,int> estimClassMap;
  // Current submit index is ix
  int ix = 0;
  for (map<std::string, DagNode *>::iterator iter = dag->begin();
       iter != dag->end();
       ++iter) {
    DagNode * dagNode = (DagNode *) iter->second;
    // creates the diet profile
    dagNode->initProfileSubmit();

    // set the submit index and decide if this node must be added to the request
    string nodeEstimClass = dagNode->getEstimationClass();
    bool submit = true;
    int submitIdx;
    if (nodeEstimClass.empty()) {
      // default case (no estimation class) => node is added to request
      submitIdx = ix++;
    } else {
      // estimation class defined => check if already added to request or not
      map<string,int>::iterator estimClassIter = estimClassMap.find(nodeEstimClass);
      if (estimClassIter != estimClassMap.end()) {
        submitIdx = (int) estimClassIter->second;
        submit = false;
        TRACE_TEXT (TRACE_ALL_STEPS,"Node " << dagNode->getId()
                     << " using submit index=" << submitIdx << endl);
      } else {
        submitIdx = ix++;
        estimClassMap.insert(make_pair(nodeEstimClass,submitIdx));
      }
    }
    dagNode->setSubmitIndex(submitIdx);

    // add the current node's profile to the sequence (if part of the request)
    if (submit) {
      mrsh_pb_desc(&(*pbs_seq)[submitIdx], dagNode->getProfile());
    }

  }
  TRACE_TEXT (TRACE_ALL_STEPS,
              "MultiWfScheduler: send " << ix << " profile(s) to the MA  ... "
                  << endl);
  // resize the sequence to the final length
  pbs_seq->length(ix);

  bool failed = false;
  string failureMsg;
  try {
    wf_response = MA->submit_pb_set(*pbs_seq);
  } catch(CORBA::SystemException& e) {
    failureMsg = " MultiWfScheduler: Got a CORBA " + string(e._name()) + " exception ("
                 + string(e.NP_minorString()) + ")";
    failed = true;
    WARNING(failureMsg << endl);
  }
  TRACE_TEXT (TRACE_ALL_STEPS, "... done" << endl);
  delete pbs_seq;
  if (failed) {
    throw (MaDag::CommProblem(failureMsg.c_str()));
  }
  if (!wf_response->complete) {
    // get the faulty node using the submission index
    DagNode * failedDagNode = NULL;
    for (map<std::string, DagNode *>::iterator iter = dag->begin();
         iter != dag->end();
         ++iter) {
      DagNode *dagNode = (DagNode *) iter->second;
      if (dagNode->getSubmitIndex() == wf_response->idxError) {
        failedDagNode = dagNode;
        break;
      }
    }
    // throw corba exception with node details
    if (failedDagNode)
      throw (MaDag::ServiceNotFound(failedDagNode->getId().c_str(),
                                    failedDagNode->getPbName().c_str(),
                                    failedDagNode->getPortsDescr().c_str()));
    else
      throw (MaDag::ServiceNotFound(NULL,NULL,NULL));
  }
  return wf_response;
}

/**
 * Call MA to get server estimations for one node
 * Note: the profile for the node is supposed to be already created
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(DagNode *node, MasterAgent_var MA)
    throw (MaDag::ServiceNotFound, MaDag::CommProblem) {
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  wf_response_t * wf_response = NULL;
  pbs_seq->length(1);
  node->setSubmitIndex(0);
  mrsh_pb_desc(&(*pbs_seq)[0], node->getProfile());
  TRACE_TEXT (TRACE_ALL_STEPS, "MultiWfScheduler: send 1 profile to the MA  ... "
                  << endl);
  bool failed = false;
  string failureMsg;
  try {
    wf_response = MA->submit_pb_set(*pbs_seq);
  } catch(CORBA::SystemException& e) {
    failureMsg = " MultiWfScheduler: Got a CORBA " + string(e._name()) + " exception ("
                 + string(e.NP_minorString()) + ")";
    WARNING(failureMsg << endl) ;
    failed = true;
  }
  delete pbs_seq;
  TRACE_TEXT (TRACE_ALL_STEPS, "... done" << endl);
  if (failed)
    throw (MaDag::CommProblem(failureMsg.c_str()));
  if (!wf_response->complete)
    throw (MaDag::ServiceNotFound(node->getId().c_str(),
                                  node->getPbName().c_str(),
                                  node->getPortsDescr().c_str()));
  return wf_response;
}

/**
 * Intra-dag scheduling
 */
void
MultiWfScheduler::intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw (MaDag::ServiceNotFound, MaDag::CommProblem) {
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
  for (std::map <std::string, DagNode *>::iterator nodeIt = dag->begin();
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
  delete waitQ;
  delete nodeQ;
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
MultiWfScheduler::setExecPriority(DagNode * node) {
  // by default does nothing
}
/**
 * set node priority before inserting back in the ready queue
 */
void
MultiWfScheduler::setWaitingPriority(DagNode * node) {
  // by default does nothing
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

/**
 * Execute a post operation on synchronisation semaphore
 */
void
MultiWfScheduler::wakeUp(bool newDag, DagNode *node) {
  // store information in the FIFO list
  wakeUpInfo_t  info;
  info.isNewDag = newDag;
  info.nodeRef = node;
  myWakeUpLock.lock();
  myWakeUpList.push_back(info);
  myWakeUpLock.unlock();
  // call POST on semaphore
  this->mySem.post();
}

/**
 * Call the LogService when a node becomes ready for queueing
 * (happens when node has no more pending dependencies)
 */
void
MultiWfScheduler::handlerNodeReady(DagNode *node) {
//   DietLogComponent* LC = myMaDag->getDietLogComponent();
//   if (LC) {
//     LC->logWfNodeReady(node->getDag()->getId().c_str(),
//                        node->getId().c_str());
//   }
  sendEventFrom<DagNode, DagNode::READY>(node, "Node ready", "" , EventBase::INFO);
}

/**
 * manage dag end of execution
 * Can handle several calls for the same dag (may happen if several nodes from
 * the same dag are cancelled within a short timeframe)
 */
void
MultiWfScheduler::handlerDagDone(Dag * dag) {
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
  if (myWakeUpList.size() > 0) {
    wakeUpInfo_t& info = myWakeUpList.front();
    myWakeUpLock.unlock();
    if (info.isNewDag)
    {
      TRACE_TEXT (TRACE_MAIN_STEPS,"Scheduler waking up (NEW DAG)" << endl);
    } else
    {
      TRACE_TEXT (TRACE_MAIN_STEPS,"Scheduler waking up (END OF NODE)" << endl);
      if (info.nodeRef)
      {
        info.nodeRef->terminate();
      } else
      {
        INTERNAL_ERROR(__FUNCTION__ << "Invalid terminating node reference" << endl,1);
      }
    }
    myWakeUpLock.lock();
    myWakeUpList.pop_front();
  }
  myWakeUpLock.unlock();
}

/**
 * manage dags termination
 */
void
MultiWfScheduler::checkDagsRelease() {
  // MANAGE DAG TERMINATION
  myWakeUpLock.lock();
  if (myDagsTermList.size() > 0)
  {
    myDagsTermList.sort();
    myDagsTermList.unique();  // removes consecutive duplicates
    list<string>::iterator it = myDagsTermList.begin();
    while (it != myDagsTermList.end())
    {
      Dag * currDag = getDag(*it);
      TRACE_TEXT (TRACE_ALL_STEPS,"Dag " << currDag->getId() << " : try to release" << endl);
      if (!currDag->isRunning())
      {
        myWakeUpLock.unlock();
        releaseDag(currDag);
        myWakeUpLock.lock();
        it = myDagsTermList.erase(it);
      } else
      {
        TRACE_TEXT (TRACE_ALL_STEPS,"Dag " << currDag->getId() << " : cannot release now" << endl);
        ++it;
      }
    }
  }
  myWakeUpLock.unlock();
}

/**
 * release dag on client mgr
 */
void
MultiWfScheduler::releaseDag(Dag * dag) {
  typedef size_t comm_failure_t;
  string dagId = dag->getId();
  char * message;
  bool clientFailure;
  MetaDag * metaDag = this->getMetaDag(dag);

  // RELEASE THE CLIENT MANAGER

  TRACE_TEXT(TRACE_MAIN_STEPS, "[Dag " << dagId << "] : calling client for release" << endl);
  CltMan_ptr cltMan = myMaDag->getCltMan(dag->getId());
  try {
      message = cltMan->release(dagId.c_str(), !dag->isCancelled());
      TRACE_TEXT (TRACE_ALL_STEPS," Release message : " << message << endl);
       // INFORM LOGMANAGER
      if (myMaDag->dietLogComponent != NULL) {
        myMaDag->dietLogComponent->logDag(message);
      }
      stat_flush();
      delete message;
  } catch(CORBA::SystemException& e) {
      cout << "Caught a CORBA " << e._name() << " exception ("
           << e.NP_minorString() << ")" << endl ;
      WARNING("Connection problems with Client occured - Release cancelled");
      clientFailure = true;
      dag->setAsCancelled(NULL);
      if (metaDag)
        metaDag->setReleaseFlag(true);  // allow MaDag to destroy the metadag
  }

  // DISPLAY DEBUG INFO
  if (!dag->isCancelled()) {
    TRACE_TEXT (TRACE_MAIN_STEPS,"############### DAG "
                                  << dagId << " IS DONE #########" << endl);
  } else {
    TRACE_TEXT (TRACE_MAIN_STEPS,"############### DAG "
                                  << dagId << " IS CANCELLED #########" << endl);
    // Display list of failed nodes
    const std::list<string>& failedNodes = dag->getNodeFailureList();
    for (std::list<string>::const_iterator iter = failedNodes.begin();
         iter != failedNodes.end();
         ++iter) {
      TRACE_TEXT (TRACE_MAIN_STEPS, "Dag " << dagId << " FAILED NODE : " << *iter << endl);
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
    TRACE_TEXT (TRACE_ALL_STEPS, "Trigger end-of-dag event to MetaDag" << endl);
    metaDag->handlerDagDone(dag);
    myMetaDags.erase(dag->getId());
    if (metaDag->isDone()) {
      TRACE_TEXT (TRACE_ALL_STEPS,"######## META-DAG "
                                << metaDag->getId() << " IS COMPLETED #########" << endl);
      delete metaDag;
    }
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS, "Deleting dag" << endl);
    delete dag;
  }
}

/**
 * Cancel dag (without stopping running tasks)
 */
void
MultiWfScheduler::cancelDag(const string& dagId) {
  TRACE_TEXT (TRACE_ALL_STEPS,"######## RECEIVED DAG CANCELLATION FOR DAG '"
                                << dagId << "' #########" << endl);
  myLock.lock();
  try {
    getDag(dagId)->setAsCancelled(this);
  } catch (...) {
    myLock.unlock();
    throw;
  }
  myLock.unlock();
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

