/**
* @file Dag.cc
* 
* @brief  Dag description 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/

#include "debug.hh"

#include "Dag.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"
#include "DagWfParser.hh"
#include "DagScheduler.hh"
#include "EventTypes.hh"

using namespace std;
using namespace events;

Dag::Dag(string id)
  : myId(id), myWf(NULL), myExecAgent(MasterAgent::_nil()), startTime(0),
    finishTime(0), estDelay(0), tmpDag(false), cancelled(false) {
  if (id.empty()) {
    WARNING("Dag created with empty id - may cause errors");
  }
}

Dag::Dag(string id, MasterAgent_var& MA)
  : myId(id), myWf(NULL), startTime(0), finishTime(0),
    estDelay(0), tmpDag(false), cancelled(false) {
  if (id.empty()) {
    WARNING("Dag created with empty id - may cause errors");
  }
  if (MA != MasterAgent::_nil())
    myExecAgent = MA;
  else {
    INTERNAL_ERROR("Null MASTER AGENT in Dag constructor", 1);
  }
}

/**
 * DAG destructor: will delete all the nodes of the dag (if not a temp dag)
 */
Dag::~Dag() {
  if (! this->tmpDag) {
    while (! nodes.empty()) {
      DagNode * p = begin()->second;
      nodes.erase(begin());
      delete p;
    }
    nodes.clear();
  }
}

void
Dag::setId(const string& id) {
  if (!myId.empty())
    sendEventFrom<Dag, MODID>(this, "Dag id modified", id, EventBase::INFO);
  this->myId = id;
}

const string&
Dag::getId() const {
  return this->myId;
}

void
Dag::setWorkflow(FWorkflow * wf) {
  this->myWf = wf;
}

FWorkflow *
Dag::getWorkflow() const throw(WfStructException) {
  if (myWf != NULL)
    return myWf;
  throw WfStructException(WfStructException::eWF_UNDEF, "dag_id=" + getId());
}

MasterAgent_var&
Dag::getExecutionAgent() {
  if (myExecAgent == MasterAgent::_nil()) {
    INTERNAL_ERROR(__FUNCTION__<<"No MA defined for dag execution"<<endl, 1);
  }
  return myExecAgent;
}

void
Dag::setExecutionAgent(MasterAgent_var& MA) {
  myExecAgent = MA;
}

/**
 * Get the node with given identifier (only node id, not the complete id)
 */
WfNode *
Dag::getNode(const string& nodeId) throw(WfStructException) {
  return getDagNode(nodeId);
}

/**
 * Allocates a new node and add it to the dag
 */
DagNode*
Dag::createDagNode(const string& id, FWorkflow* wf) throw(WfStructException) {
  map<string, DagNode*>::iterator p = this->nodes.find(id);
  if (p != this->nodes.end())
    throw WfStructException(WfStructException::eDUPLICATE_NODE, "node id="+id);
  DagNode* newDagNode = new DagNode(id, this, wf);
  this->nodes[id] = newDagNode;
  EventManager::getEventMgr()->sendEvent(
    new EventCreateObject<DagNode, Dag>(newDagNode, this));
  return newDagNode;
}

/**
 * Get the dag node with given identifier (only node id, not the complete id)
 */
DagNode *
Dag::getDagNode(const string& nodeId) throw(WfStructException) {
  map<string, DagNode*>::iterator p = this->nodes.find(nodeId);
  if (p != this->nodes.end())
    return p->second;
  else
    throw WfStructException(WfStructException::eUNKNOWN_NODE, "node id="+nodeId);
}

void
Dag::removeNode(const string& nodeId) throw(WfStructException) {
  map<string, DagNode*>::iterator p = this->nodes.find(nodeId);
  if (p != this->nodes.end()) {
    // remove node from the list of nodes
    this->nodes.erase(p);
    // clean parent attribute of the node
    ((DagNode*) p->second)->setDag(NULL);

  } else
    throw WfStructException(WfStructException::eUNKNOWN_NODE, "node id="+nodeId);
}

/**
 * check the precedence between node *
 * this function checks the precedence between node; it doesn't *
 * link the WfPorts but it creates the list of predecessors of each node *
 */
void
Dag::checkPrec(NodeSet* contextNodeSet) throw(WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING DAG STRUCTURE START" << endl);
  for (map<string, DagNode * >::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    DagNode *node = (DagNode*) p->second;
    node->setNodePrecedence(contextNodeSet);  // throw WfStructException
  }
  // TODO use DFS to check there is no cycle
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING DAG STRUCTURE END" << endl);
}

/**
 * link all ports of the dag *
 */
void
Dag::linkAllPorts() throw(WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, "LINKING NODES START" << endl);
  for (map<string, DagNode*>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    DagNode * n = (DagNode*)(p->second);
    n->connectNodePorts();
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "LINKING NODES END" << endl);
}

/**
 * return the size of the Dag (the nodes number)
 */
unsigned int
Dag::size() {
  return nodes.size();
}

/**
 * return an iterator on the first node *
 * (according to the map and not to the dag structure) *
 */
map <string, DagNode *>::iterator
Dag::begin() {
  return nodes.begin();
}

/**
 * return an iterator on the last node *
 * (according to the map and not to the dag structure) *
 */
map <string, DagNode *>::iterator
Dag::end() {
  return nodes.end();
}

string
Dag::toString() const {
  return "DAG " + getId();
}

/**
 * returns the XML description of the dag
 */
void
Dag::toXML(ostream& output) const {
  output << "<dag>" << endl;
  for (map<string, DagNode*>::const_iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    ((DagNode*) p->second)->toXML(output);
  }
  output << "</dag>" << endl;
}

/**
 * printDietReqID the dag execution is completed
 *
 * this methods loops through all the nodes of the dag and getReqID of each node
 * @return reqID[]
 */
void
Dag::showDietReqID() {
  DagNode * dagNode = NULL;
  TRACE_TEXT(TRACE_MAIN_STEPS,
             "@@@@ BEGIN Dag::" <<__FUNCTION__  << "()" << endl
             << "dag_id =" << this->myId << endl);
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (DagNode*)p->second;
    //if ((dagNode) && !(dagNode->isDone()))
    TRACE_TEXT(TRACE_MAIN_STEPS,
               " dagNode->getPb() = "  << dagNode->getPbName() <<endl
               << " dagNode->getProfile()->dietReqID =" << dagNode->getProfile()->dietReqID << endl);
  }
  TRACE_TEXT(TRACE_MAIN_STEPS, "@@@@ END Dag::" <<__FUNCTION__  << "()" << endl);
}
/**
 * printDietReqID the dag execution is completed
 *
 * this methods loops through all the nodes of the dag and getReqID of each node
 * @return diet_reqID_t[]
 */
vector<diet_reqID_t>
Dag::getAllDietReqID() {
  DagNode * dagNode = NULL;
  vector<diet_reqID_t> request_ids;
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end(); ++p) {
    dagNode = (DagNode*)p->second;
    request_ids.push_back(dagNode->getReqID());
  }
  return request_ids;
}

bool
Dag::isDone() {
  DagNode * dagNode = NULL;
  bool res = true;
  this->myLock.lock();  /** LOCK */
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (DagNode*) p->second;
    if ((dagNode) && !(dagNode->isDone())) {
      res = false;
      break;
    }
  }
  this->myLock.unlock(); /** UNLOCK */
  return res;
}

/**
 * check if the dag execution is ongoing
 * (checks all nodes status and returns true if at least one node is running)
 */
bool
Dag::isRunning() {
  bool res = false;
  DagNode * dagNode = NULL;
  this->myLock.lock();  /** LOCK */
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    dagNode = (DagNode*)p->second;
    if ((dagNode) && (dagNode->isRunning())) {
      res = true;
    }
  }
  this->myLock.unlock(); /** UNLOCK */
  return res;
}

/**
 * check if the dag execution is cancelled *
 */
bool
Dag::isCancelled() {
  return cancelled;
}

/**
 * local function to extract node and port name from composite
 * id (node#port) used by API
 */

DagNodeOutPort *
Dag::getOutputPort(const char* id) throw(WfStructException) {
  string strId(id);
  string::size_type portSep = strId.find("#");
  string nodeId = strId.substr(0, portSep);
  string portId = strId.substr(portSep+1, strId.length()-portSep-1);
  DagNode * n = getDagNode(nodeId);
  return dynamic_cast<DagNodeOutPort*>(n->getPort(portId));
}

/**
 * Get a scalar result of the workflow *
 *
 * this methods loops through all dag nodes to find the output port identified
 * by id
 * @param id identifier of the node
 * @param value pointer to pointer to the value of the output port
 *
 * @todo unify the different methods used to retrieve an output for the different types
 */
int
Dag::get_scalar_output(const char * id,
                       void** value) {
  DagNodeOutPort * outp = getOutputPort(id);
  return diet_scalar_get(diet_parameter(outp->profile(), outp->getIndex()),
                         value, NULL);
}

/**
 * get a string result of the workflow *
 */
int
Dag::get_string_output(const char * id,
                       char** value) {
  DagNodeOutPort * outp = getOutputPort(id);
  return diet_string_get(diet_parameter(outp->profile(), outp->getIndex()),
                         value, NULL);
}

/**
 * Get a file result of the workflow
 */
int
Dag::get_file_output (const char * id,
                      size_t* size, char** path) {
  DagNodeOutPort * outp = getOutputPort(id);
  return diet_file_get(diet_parameter(outp->profile(), outp->getIndex()), NULL,
                       size, path);
}

/**
 * Get a matrix result of the workflow
 */
int
Dag::get_matrix_output (const char * id, void** value,
                        size_t* nb_rows, size_t *nb_cols,
                        diet_matrix_order_t* order) {
  DagNodeOutPort * outp = getOutputPort(id);
  return diet_matrix_get(diet_parameter(outp->profile(), outp->getIndex()),
                         value, NULL,
                         nb_rows, nb_cols, order);
}

/**
 * Get a container result of the workflow
 */
int
Dag::get_container_output (const char * id, char** dataID) {
  DagNodeOutPort * outp = getOutputPort(id);
  if (dataID != NULL) {
    *dataID = strdup(outp->profile()->parameters[outp->getIndex()].desc.id);
    return 0;
  } else return 1;
}

/**
 * Get the nodes sorted according to their priority using insertion sort
 * (output vector must be deleted)
 */
std::vector<DagNode*>&
Dag::getNodesByPriority() {
  std::vector<DagNode*> * sorted_list = new std::vector<DagNode*>;
  DagNode * n1 = NULL;
  TRACE_TEXT(TRACE_ALL_STEPS, "Sorting dag nodes by priority" << endl);
  for (std::map <std::string, DagNode *>::iterator p = this->begin();
       p != this->end();
       ++p) {
    n1 = (DagNode*)(p->second);
    // found where insert the node
    std::vector<DagNode*>::iterator q = sorted_list->begin();
    bool b = false;
    DagNode * n2 = NULL;
    while ((q != sorted_list->end()) && (!b)) {
      n2 = *q;
      if (n2->getPriority() < n1->getPriority())
        b = true;
      else
        ++q;
    }
    sorted_list->insert(q, n1);
  }
  return *sorted_list;
}

/**
 * get all the results
 */
void
Dag::displayAllResults(ostream& output) {
  output << "** RESULTS OF DAG " << getId() << " **" << endl;
  DagNode * n = NULL;
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (DagNode *)(p->second);
    if ((n != NULL) && (n->isAnExit())) {
      n->displayResults(output);
    }
  }
}

/**
 * free all persistent data used by the dag (includes intermediate and
 * final results)
 */
void
Dag::deleteAllResults() {
  DagNode * n = NULL;
  for (map<string, DagNode *>::iterator p = nodes.begin();
       p != nodes.end();
       ++p) {
    n = (DagNode *)(p->second);
    if (n != NULL) {
      n->freeProfileAndData(myExecAgent);
    }
  }
}

/**
 * get the input nodes
 */
vector<DagNode *>
Dag::getInputNodes() {
  vector<DagNode *> v;
  DagNode * node = NULL;
  for (map<string, DagNode *>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       ++p) {
    node = (DagNode *)(p->second);
    if ((node != NULL) && (node->isAnInput()))
      v.push_back(node);
  }
  return v;
} // end getInputNodes

/**
 * set all input nodes as ready
 */
void
Dag::setInputNodesReady(DagScheduler* scheduler) {
  vector<DagNode *> inputs = this->getInputNodes();
  for (vector<DagNode *>::iterator p = inputs.begin();
       p != inputs.end();
       ++p) {
    DagNode * node = (DagNode *) *p;
    node->setAsReady(scheduler);
  }
}

/**
 * get the estimated makespan of the DAG
 * @deprecated
 */
// double
// Dag::getEstMakespan() {
//   double makespan = -1;
//   WfNode * n = NULL;
//   for (map<string, WfNode*>::iterator p = this->nodes.begin();
//        p != this->nodes.end();
//        ++p) {
//     n = (WfNode*)(p->second);
//     if ((n != NULL) &&
//       (n->getEstCompTime() > makespan))
//       makespan = n->getEstCompTime();
//   }
//   return makespan;
// }

/**
 * get the estimated earliest finish time of the DAG
 */
double
Dag::getEFT() {
  double EFT = -1;
  DagNode * n = NULL;
  for (map<string, DagNode*>::iterator p = this->nodes.begin();
       p != this->nodes.end();
       ++p) {
    n = (DagNode*)(p->second);
    if ((n != NULL) &&
         (n->getEstCompTime() > EFT))
      EFT = n->getEstCompTime();
  }
  return EFT;
}

/**
 * set the start time of the DAG
 */
void
Dag::setStartTime(double time) {
  this->startTime = time;
}

/**
 * get the start time of the DAG
 */
double
Dag::getStartTime() {
  return this->startTime;
}

/**
 * set the finish time of the DAG
 */
void
Dag::setFinishTime(double time) {
  this->finishTime = time;
}

/**
 * get the makespan of the DAG (finish - start)
 */
double
Dag::getMakespan() {
  return finishTime - startTime;
}

/**
 * get the estimated delay of the DAG
 */
double
Dag::getEstDelay() {
  return this->estDelay;
}

/**
 * set the estimated delay of the DAG
 * (updated by an exit node)
 * (can be increased or decreased)
 */
void
Dag::setEstDelay(double delay) {
  this->estDelay = delay;
  TRACE_TEXT(TRACE_ALL_STEPS, "Updated est. delay on DAG "
              << this->getId() << " : delay = " << delay << endl);
}

bool
Dag::updateDelayRec(DagNode * node, double newDelay) {
  bool res = true;
  this->myLock.lock();  /** LOCK */
  res = _updateDelayRec(node, newDelay);
  this->myLock.unlock();  /** UNLOCK */
  return res;
}

/**
 * RECURSIVE
 * updates the estDelay of node and propagates it to the successors
 * returns true if propagation worked well (no pb in getting delay info)
 */
bool
Dag::_updateDelayRec(DagNode * node, double newDelay) {
  bool res = true;
  if (newDelay > node->getEstDelay()) {
    // the node is/will be late compared to the last estimated delay
    // so the new delay must be propagated to the successors
    node->setEstDelay(newDelay);
    for (list<WfNode*>::iterator nextIter = node->nextNodesBegin();
         nextIter != node->nextNodesEnd();
         ++nextIter) {
      res = res && this->_updateDelayRec(dynamic_cast<DagNode*>(*nextIter), newDelay);
    }
  }
  else {
    TRACE_TEXT(TRACE_ALL_STEPS, "Delay estimate for node "
                << node->getCompleteId() << " unchanged (newDelay = "
                << newDelay << " / previous delay = " << node->getEstDelay()
                << ")" << endl);
  }
  return res;
}

/**
 * Notify the dag of node end of execution (when successful)
 */
void
Dag::setNodeDone(DagNode* node, DagScheduler* scheduler) {
  // the following applies only to MaDag
  if (scheduler) {
    // trigger next nodes (even if dag finished , because nodes belonging to other dags
    // can depend on the current one)
    if (!isCancelled() && (node->nextNodesNb() > 0)) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Dag " << getId() << " : Calling next nodes of "
                  << node->getId() << endl);
      for (list<WfNode*>::iterator nextIter = node->nextNodesBegin();
           nextIter != node->nextNodesEnd();
           ++nextIter) {
        (dynamic_cast<DagNode*>(*nextIter))->prevNodeHasDone(scheduler);
      }
    }
    // manage dag termination when the current node is the last
    // one finishing (either dag is complete or is cancelled)
    if (isDone() || isCancelled()) {
      TRACE_TEXT(TRACE_ALL_STEPS, "Dag " << getId() << " : End of execution" << endl);
      scheduler->handlerDagDone(this);
    }
  }
}

/**
 * notify the dag of node execution failure (MADAG & CLIENT-SIDE)
 */
void
Dag::setNodeFailure(string nodeId, DagScheduler* scheduler) {
  this->myLock.lock();  /** LOCK */
  setAsCancelled(scheduler);
  this->failedNodes.push_front(nodeId);
  this->myLock.unlock();  /** UNLOCK */
}

/**
 * get the list of failed nodes
 */
const std::list<string>&
Dag::getNodeFailureList() {
  return this->failedNodes;
}
/**
 * set the dag as cancelled
 */
void
Dag::setAsCancelled(DagScheduler* scheduler) {
  if (cancelled) return;
  cancelled = true;
  // the following is used only by MaDag
  if (scheduler) {
    // remove all nodes from their queues
    TRACE_TEXT(TRACE_ALL_STEPS, "Dag " << getId()
                << " : removing all nodes from nodeQueues" << endl);
    for (map<string, DagNode*>::iterator p = this->nodes.begin();
         p != this->nodes.end();
         ++p) {
      ((DagNode*) p->second)->removeFromNodeQueue();
    }
    // trigger dag termination (will remove all dag nodes from the execution
    // queues and will inform the client of cancellation)
    scheduler->handlerDagDone(this);
  }
  sendEventFrom<Dag, STATE>(this, "Dag cancelled", "", EventBase::INFO);
}

