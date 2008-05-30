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
#include "debug.hh"

using namespace madag;

/****************************************************************************/
/*                                                                          */
/*                      CLASS MultiWfScheduler                              */
/*                                                                          */
/****************************************************************************/

long MultiWfScheduler::interNodeDelay = 200; // in milliseconds

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfScheduler::MultiWfScheduler(MaDag_impl* maDag) : mySem(1), myMaDag(maDag) {
//   this->myMetaDag = new MultiDag();
  this->mySched   = new HEFTScheduler();
  this->execQueue = NULL; // must be initialized in derived class constructor
  // init reference time
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  this->refTime = current_time.tv_sec;
}

MultiWfScheduler::~MultiWfScheduler() {
  if (this->myMetaDag != NULL)
    delete this->myMetaDag;
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

  // Beginning of exclusion block
  this->myLock.lock();

  // Insert node queue into pool of node queues managed by the scheduler
  TRACE_TEXT (TRACE_ALL_STEPS, "Inserting new node queue into queue pool" << endl);
  this->insertNodeQueue(readyNodeQ);

  // Send signal to scheduler thread to inform there are new nodes
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
  int nodeCount = 0;
  while (true) {
    cout << "\t ** Starting Multi-Workflow scheduler" << endl;
    this->myLock.lock();
    // Loop over all nodeQueues and run the first ready node
    // for each queue
    nodeCount = 0;
    std::list<OrderedNodeQueue *>::iterator qp = readyQueues.begin();
    while (qp != readyQueues.end()) {
      cout << "Checking ready nodes queue:" << endl;
      OrderedNodeQueue * readyQ = *qp;
      Node * n = readyQ->popFirstNode();
      if (n != NULL) {
        cout << "  #### Ready node : " << n->getCompleteId()
            << " / request #" << n->getWfReqId()
            << " / prio = " << n->getPriority() << endl;

        // set priority of node (depends on choosen algorithm)
        this->setExecPriority(n);

        // insert node into execution queue
        execQueue->pushNode(n);

        nodeCount++;
        // Destroy queues if both are empty
        ChainedNodeQueue * waitQ = waitingQueues[readyQ];
        if (waitQ->isEmpty() && readyQ->isEmpty()) {
          cout << "Node Queues are empty: remove & destroy" << endl;
          qp = readyQueues.erase(qp);      // removes from the list
          this->deleteNodeQueue(readyQ);  // deletes both queues
          continue;
        }
      }
      ++qp; // go to next queue
    }

    if (nodeCount > 0) {
      cout << "Executing nodes in priority order:" << endl;
      while (!execQueue->isEmpty()) {
        Node *n = execQueue->popFirstNode();

        // EXECUTE NODE (NEW THREAD)
        n->setAsRunning();
        cout << "  $$$$ Exec node : " << n->getCompleteId()
            << " / request #" << n->getWfReqId()
            << " / prio = " << n->getPriority() << endl;
        runNode(n, n->getSeD());

        // DELAY between NODES (to avoid interference btw submits)
        usleep(this->interNodeDelay * 1000);
      }
    }

    this->myLock.unlock();

    if (nodeCount == 0) {
      cout << "No ready nodes" << endl;
      this->mySem.wait();
      if (this->termNode) {
        this->termNodeThread->join();
        delete this->termNodeThread;
      }
    }
  }
}

/**
 * Execute a post operation on synchronisation semaphore
 */
void
MultiWfScheduler::wakeUp() {
  cout << "Wake Up" << endl;
  this->termNode = false; // no thread to join
  this->mySem.post();
}

/**
 * Execute a post operation on synchronisation semaphore
 * and joins the node thread given as parameter
 */
void
MultiWfScheduler::wakeUp(NodeRun * nodeThread) {
  cout << "Wake Up & Join" << endl;
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
  return (current_time.tv_sec - this->refTime);
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
  DagWfParser reader(wfReqId, wf_desc.abstract_wf);
  reader.setup();
  Dag * newDag = reader.getDag();
  newDag->setId(itoa(dagIdCounter++));
  return newDag;
}

/**
 * Call MA to get server estimations for all services
 */
wf_response_t *
MultiWfScheduler::getProblemEstimates(Dag *dag, MasterAgent_var MA)
    throw (NodeException) {
  // Check that all services are available and get the estimations (with MA)
  vector<diet_profile_t*> v = dag->getAllProfiles();
  unsigned int len = v.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    TRACE_TEXT (TRACE_ALL_STEPS, "marshalling pb " << ix << endl
        << "pb_name = " << v[ix]->pb_name << endl
        << "last_in = " << v[ix]->last_in << endl
        << "last_inout = " << v[ix]->last_inout << endl
        << "last_out = " << v[ix]->last_out << endl);

    mrsh_pb_desc(&pbs_seq[ix], v[ix]);
  }
  TRACE_TEXT (TRACE_ALL_STEPS,
              "MultiWfScheduler: send the problems sequence to the MA  ... "
                  << endl);
  wf_response_t * wf_response = MA->submit_pb_set(pbs_seq, dag->size());
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
  delete waitQ;
  PriorityNodeQueue * readyQ  = dynamic_cast<PriorityNodeQueue *>(nodeQ);
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
 * get reference time
 */
double
MultiWfScheduler::getRefTime() {
  return this->refTime;
}

/**
 * Execute a node
 */
Thread *
MultiWfScheduler::runNode(Node * node, SeD_var sed) {
  Thread * thread = new NodeRun(node, sed, this,
                                this->myMaDag->getCltMan(node->getWfReqId()));
  thread->start();
  return thread;
}

/****************************************************************************/
/*                            CLASS NodeRun                                 */
/****************************************************************************/

NodeRun::NodeRun(Node * node, SeD_var sed, MultiWfScheduler * scheduler,
                 CltMan_ptr cltMan) {
  this->myNode = node;
  this->mySeD = sed;
  this->myScheduler = scheduler;
  this->myCltMan = cltMan;
}

/**
 * The execution method
 */
void*
NodeRun::run() {
  string dag_id = this->myNode->getDag()->getId();
  TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: running node "
      << this->myNode->getCompleteId() << endl);
  if (myCltMan != CltMan::_nil()) {
    TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: try to call client manager (ping)"
        << endl);
    myCltMan->ping();

    TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: try to call client manager ");
    if (!CORBA::is_nil(this->mySeD)) {
      TRACE_TEXT (TRACE_ALL_STEPS, "(exec on sed)" << endl);
      myCltMan->execNodeOnSed(this->myNode->getId().c_str(),
                              this->myNode->getDag()->getId().c_str(),
                                  this->mySeD);
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "(exec without sed)" << endl);
      myCltMan->execNode(this->myNode->getId().c_str(),
                         this->myNode->getDag()->getId().c_str());
    }
    TRACE_TEXT (TRACE_ALL_STEPS, "NodeRun: node is done" << endl);
    // update node status (must be called before handlerNodeDone to update realCompTime)
    this->myNode->setAsDone(this->myScheduler->getRelCurrTime());
    // inform scheduler that node is done (depending on the scheduler, this may trigger
    //  a recursive update of the realCompTime in other nodes)
    this->myScheduler->handlerNodeDone(myNode);
      // check if dag is completed and release client if yes
    if ((this->myNode->getDag() != NULL) &&
         (this->myNode->getDag()->isDone())
       ) {
      cout << "NodeRun: The Dag " << this->myNode->getDag()->getId().c_str()
          << " is done " << endl;
      myCltMan->release(this->myNode->getDag()->getId().c_str());
       }
  }
  else {
    cout << "NodeRun: ERROR!! cannot contact the Client Wf Mgr" << endl;
  }
  this->myScheduler->wakeUp(this);
}
