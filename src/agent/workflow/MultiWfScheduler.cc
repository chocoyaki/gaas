/****************************************************************************/
/* The base abstract class for multi-workflow scheduler                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
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
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

MultiWfScheduler::MultiWfScheduler(MaDag_impl* maDag) : mySem(1), myMaDag(maDag) {
  this->myMetaDag = new MultiDag();
  this->mySched   = new HEFTScheduler();
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
MaDag_impl*
MultiWfScheduler::getMaDag() {
  return this->myMaDag;
}

/**
 * Process a new dag => when finished the dag is ready for execution
 */
bool
MultiWfScheduler::scheduleNewDag(const corba_wf_desc_t& wf_desc, int wfReqId,
                                 MasterAgent_var MA)
{
  TRACE_TEXT (TRACE_ALL_STEPS, "The meta scheduler receives a new xml dag " << endl);
  // Dag XML Parsing
  Dag * newDag;
  try {
    newDag = this->parseNewDag(wf_desc);
  }
  catch (XMLParsingException& e) {
    // code to handle XML parsing exceptions
    return false;
  }

  // Dag internal scheduling
  std::vector<Node *> orderedNodes;
  TRACE_TEXT (TRACE_ALL_STEPS, "Making intra-dag schedule" << endl);
  try {
    orderedNodes = this->intraDagSchedule(newDag, MA);
  }
  catch (NodeException& e) {
    // code to handle node scheduling exceptions
    // destroy dag ?
    return false;
  }

  // Node queue creation (to manage ready nodes queueing)
  TRACE_TEXT (TRACE_ALL_STEPS, "Initializing new node queue" << endl);
  NodeQueue * nodeQ = this->createNodeQueue(orderedNodes, wfReqId);

  // Init queue by setting input nodes as ready
  newDag->setInputNodesReady();

  // Beginning of exclusion block
  this->myLock.lock();

  // Insert node queue into pool of node queues managed by the scheduler
  TRACE_TEXT (TRACE_ALL_STEPS, "Inserting new node queue into queue pool" << endl);
  this->insertNodeQueue(nodeQ);

  // Send signal to scheduler thread to inform there are new nodes
  this->wakeUp();

  // End of exclusion block
  this->myLock.unlock();

  return true;
}


/**
 * Execution method
 */
void*
MultiWfScheduler::run() {
  int nodeCount = 0;
  while (true) {
    cout << "\t ** Starting MultiWfScheduler" << endl;
    this->myLock.lock();
    nodeCount = 0;
    for (std::list<NodeQueue *>::iterator qp = myQueues.begin();
         qp != myQueues.end();
         qp++) {
      NodeQueue * q = *qp;
      Node * n = q->popFirstReadyNode();
      if (n != NULL) {
        cout << "Ready node " << n->getCompleteId()
            << " (request #" << q->getWfReqId() << ")" << endl;
        n->setAsRunning();
        runNode(n, n->getSeD());
        nodeCount++;
      }
    }
    this->myLock.unlock();
    if (nodeCount == 0) {
      cout << "No ready nodes" << endl;
      this->mySem.wait();
    }
  }
}

/**
 * Execute a post operation on synchronisation semaphore
 */
void
MultiWfScheduler::wakeUp() {
  cout << "Wake Up" << endl;
  this->mySem.post();
}

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

/**
 * Parse dag xml description and create a dag object
 */
Dag *
MultiWfScheduler::parseNewDag(const corba_wf_desc_t& wf_desc)
    throw (XMLParsingException) {
  DagWfParser reader(wf_desc.abstract_wf);
  reader.setup();
  Dag * newDag = reader.getDag();
  newDag->setId(itoa(dagIdCounter++));
  return newDag;
}

/**
 * Intra-dag scheduling
 */
std::vector<Node *>
MultiWfScheduler::intraDagSchedule(Dag * dag, MasterAgent_var MA)
    throw (NodeException) {
  // Check that all services are available and get the estimations (with MA)
  vector<diet_profile_t*> v = dag->getAllProfiles();
  unsigned int len = v.size();
  corba_pb_desc_seq_t pbs_seq;
  pbs_seq.length(len);
  for (unsigned int ix=0; ix< len; ix++) {
    cout << "marshalling pb " << ix << endl
        << "pb_name = " << v[ix]->pb_name << endl
        << "last_in = " << v[ix]->last_in << endl
        << "last_inout = " << v[ix]->last_inout << endl
        << "last_out = " << v[ix]->last_out << endl;

    mrsh_pb_desc(&pbs_seq[ix], v[ix]);
  }
  cout << "MultiWfScheduler: send the problems sequence to the MA  ... "
      << endl;
  wf_response_t * wf_response = MA->submit_pb_set(pbs_seq, dag->size());
  cout << "... done" << endl;
  if ( ! wf_response->complete)
    throw (NodeException(NodeException::eSERVICE_NOT_FOUND));

  // Prioritize the nodes (with intra-dag scheduler)
  return this->mySched->prioritizeNodes(wf_response, dag);
}

/**
 * Create a new node queue based on dag
 */
NodeQueue *
MultiWfScheduler::createNodeQueue(std::vector<Node *> nodes, const int wfReqId)  {
  NodeQueue * nodeQ = new NodeQueue(wfReqId);
  nodeQ->pushNodes(nodes);
  return nodeQ;
}

/**
 * Insert new node queue into pool
 */
void
MultiWfScheduler::insertNodeQueue(NodeQueue * nodeQ) {
  myQueues.push_back(nodeQ);
}

/****************************************************************************/
/*                            CLASS NodeRun                                 */
/****************************************************************************/

class NodeRun: public Thread {
public:
  NodeRun(Node * node, SeD_var sed, MultiWfScheduler * scheduler, CltMan_ptr cltMan) {
    this->myNode = node;
    this->mySeD = sed;
    this->myScheduler = scheduler;
    this->myCltMan = cltMan;
  }

  virtual void*
  run() {
    string dag_id = this->myNode->getDag()->getId();
    cout << "NodeRun: running node " << this->myNode->getCompleteId() << endl;
    if (myCltMan != CltMan::_nil()) {
      cout << "NodeRun: try to call client manager (ping)" << endl;
      myCltMan->ping();

      if (!CORBA::is_nil(this->mySeD)) {
        cout << "NodeRun: try to call client manager (exec on sed)" << endl;
        myCltMan->execNodeOnSed(this->myNode->getId().c_str(),
                              this->myNode->getDag()->getId().c_str(),
                              this->mySeD);
      } else {
        cout << "NodeRun: try to call client manager (exec without sed)" << endl;
        myCltMan->execNode(this->myNode->getId().c_str(),
                         this->myNode->getDag()->getId().c_str());
      }
      cout << "NodeRun: setting node as done" << endl;
      this->myNode->setAsDone();
      if ((this->myNode->getDag() != NULL) &&
          (this->myNode->getDag()->isDone())
          ) {
        cout << "NodeRun: The Dag " << this->myNode->getDag()->getId().c_str() << "is done " << endl;
        myCltMan->release(this->myNode->getDag()->getId().c_str());
      }
    }
    else {
      cout << "NodeRun: ERROR!! cannot contact the Client Wf Mgr" << endl;
    }
    this->myScheduler->wakeUp();
  }

private:
  Node *  myNode;
  SeD_var mySeD;
  MultiWfScheduler * myScheduler;
  CltMan_ptr myCltMan;
};

/**
 * Execute a node
 */
Thread *
MultiWfScheduler::runNode(Node * node, SeD_var sed) {
  Thread * thread = new NodeRun(node, sed, this, this->myMaDag->getCltMan(node->getWfReqId()));
  thread->start();
  return thread;
}
