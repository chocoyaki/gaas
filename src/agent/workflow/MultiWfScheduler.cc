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

using namespace madag;

/****************************************************************************/
/*                                                                          */
/*                      CLASS MultiWfScheduler                              */
/*                                                                          */
/****************************************************************************/

long MultiWfScheduler::interRoundDelay = 100; // in milliseconds

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfScheduler::MultiWfScheduler(MaDag_impl* maDag, nodePolicy_t nodePol)
  : mySem(0), myMaDag(maDag), nodePolicy(nodePol) {
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
  map<SeD_ptr, bool> avail;
  // the counters of executed nodes (to check if new round must be started)
  int queuedNodeCount = 0;
  int mappedNodeCount = 0;
  // the nb of nodes to move from ready to exec at each round (policy dep.)
  int nodePolicyCount = 0;
  if (this->nodePolicy == MULTIWF_NODE_METRIC)
    nodePolicyCount = -1;   // ie no limit (take all ready nodes)
  else if (this->nodePolicy == MULTIWF_DAG_METRIC)
    nodePolicyCount = 1;    // ie take only 1 ready node

  TRACE_TEXT(TRACE_ALL_STEPS,"Multi-Workflow scheduler is running" << endl);
  /// Start a ROUND of node ordering & mapping
  /// New rounds are started as long as some nodes can be mapped to ressources
  /// (if no more ressources then we wait until a node is finished or a new dag
  /// is submitted)
  while (true) {
    loopCount++;
    TRACE_TEXT(TRACE_MAIN_STEPS,"\t ** Starting Multi-Workflow scheduler ("
        << loopCount << ") time=" << this->getRelCurrTime() << endl);
    this->myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    TRACE_TEXT(TRACE_ALL_STEPS,"PHASE 1: Move ready nodes to exec queue" << endl);
    queuedNodeCount = 0;
    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      OrderedNodeQueue * readyQ = *qp;
      int npc = nodePolicyCount;
      Node * n = NULL;
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

    if (queuedNodeCount > 0) {
      TRACE_TEXT(TRACE_ALL_STEPS,"Phase 2: Check ressources for nodes in exec queue ("
          << queuedNodeCount << " nodes)" << endl);
      // Build list of services for all nodes in the exec queue
      // and ask MA for list of ressources available
      wf_response_t * wf_response = getProblemEstimates(execQueue, myMaDag->getMA());
      if (wf_response == NULL) {
        cout << "ERROR during MA submission" << endl;
        continue;
      }
      mappedNodeCount = 0;
      // Assign available ressources to nodes and start node execution
      int ix = 0; // index of the node response in wf_response
      while (!execQueue->isEmpty()) {
        Node *n = execQueue->popFirstNode();
        OrderedNodeQueue * readyQ = dynamic_cast<OrderedNodeQueue *>(n->getLastQueue());

        // CHECK RESSOURCE AVAILABILITY
        bool ressourceFound = false;
        int submitReqID = 0;  /* used to store the ReqID of submit that must be provided
        to the client for the solve request */
        corba_server_estimation_t* servEst; // will contain the chosenServer estimation
        ix = n->getSubmitIndex();
        if (!strcmp(n->getPb().c_str(), wf_response->wfn_seq_resp[ix].node_id)) {
          for (unsigned int jx=0;
               jx < wf_response->wfn_seq_resp[ix].response.servers.length();
               jx++) { // loop over servers
            servEst = &wf_response->wfn_seq_resp[ix].response.servers[jx];
            double compTime = diet_est_get_internal(&servEst->estim, EST_TCOMP, 0);
            double EFT = diet_est_get_internal(&servEst->estim, EST_USERDEFINED, 0);
            SeD_ptr curSeDPtr = servEst->loc.ior;
            string hostname(CORBA::string_dup(servEst->loc.hostName));
            TRACE_TEXT(TRACE_ALL_STEPS,"  server " << hostname << ": compTime="
                << compTime << ": EFT=" << EFT << endl);
            if (EFT - compTime <= 0) {  // test if available right now
              // test if the server has not been already chosen for another node
              bool available = true;
              for (map<SeD_ptr, bool>::iterator availIter=avail.begin();
                   availIter!=avail.end();
                   availIter++) {
                if (curSeDPtr->_is_equivalent(availIter->first)) {
                  available = false;
                  break;
                }
              }
              // server is free so it can be used for this node
              if (available) {
                ressourceFound = true;
                avail[curSeDPtr] = false;
                submitReqID = wf_response->wfn_seq_resp[ix].response.reqID;
                TRACE_TEXT(TRACE_ALL_STEPS,"  server found: " << hostname << endl);
                break;
              } // end if
            } // end if
          } // end for jx
        } else {  // problem name in node list & in wf response do not match
          TRACE_TEXT(TRACE_MAIN_STEPS,"WARNING: mismatch btw queue node & MA response"
              << "(node pb=" << n->getPb() << ")" << endl); // should not happen!
        }
        // EXECUTE NODE (NEW THREAD)
        if (ressourceFound) {
          n->setAsRunning();
	  TRACE_TEXT(TRACE_MAIN_STEPS,"  $$$$ Exec node on " << servEst->loc.hostName
            << " : " << n->getCompleteId()
            << " / exec prio = " << n->getPriority() << endl);
          mappedNodeCount++;
          runNode(n, servEst->loc.ior, submitReqID, servEst->estim);
        }
        // PUT THE NODE BACK IN THE READY QUEUE if no ressource available
        else {
          // set the priority to the initial value (intra-dag)
          this->setWaitingPriority(n);
          readyQ->pushNode(n);
        }

      } // end loop execQueue

      // cleanup availability matrix
      avail.clear();

      // destroy wf_response
      delete wf_response;

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
      }
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
    if ((queuedNodeCount == 0) || (queuedNodeCount > mappedNodeCount)) {
      if (queuedNodeCount == 0) {
        TRACE_TEXT(TRACE_MAIN_STEPS,"No ready nodes - sleeping" << endl);
      } else {
        TRACE_TEXT(TRACE_MAIN_STEPS,"No ressource available - sleeping" << endl);
      }
      this->mySem.wait();
      if (this->termNode) {
        TRACE_TEXT(TRACE_ALL_STEPS,"Joining RunNode thread" << endl);
        this->termNodeThread->join();
        delete this->termNodeThread;
        this->termNode = false;
      }
    } else {
      // DELAY between rounds (to avoid interference btw submits)
      usleep(this->interRoundDelay * 1000);
    }
  }
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
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(Dag *dag, MasterAgent_var MA)
    throw (NodeException) {
  // Check that all services are available and get the estimations (with MA)
  corba_pb_desc_seq_t* pbs_seq = new corba_pb_desc_seq_t();
  pbs_seq->length(dag->size());
  int ix = 0;
  for (map<std::string, Node *>::iterator iter = dag->begin();
       iter != dag->end(); iter++) {
         Node * node = (Node *) iter->second;
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
 */
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
	char* message = myCltMan->release(this->myNode->getDag()->getId().c_str());
	TRACE_TEXT (TRACE_ALL_STEPS," message : "<< message << endl);
	if (this->myScheduler->getMaDag()->dietLogComponent != NULL) {
		this->myScheduler->getMaDag()->dietLogComponent->logDag(message);
	}
        delete message;
	TRACE_TEXT (TRACE_MAIN_STEPS,"############### DAG "
            << this->myNode->getDag()->getId().c_str() <<" IS DONE #########"<<endl);
        delete this->myNode->getDag();
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
      TRACE_TEXT (TRACE_MAIN_STEPS, "############## DAG "
          << this->myNode->getDag()->getId().c_str() << " IS CANCELLED! #########" << endl);
      // Release the client manager (if still alive)
      if (!clientFailure) {
        char* message = myCltMan->release(this->myNode->getDag()->getId().c_str());
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

