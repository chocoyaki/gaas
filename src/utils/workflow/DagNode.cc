/**
* @file DagNode.cc
*
* @brief  The node class used for dag execution
*
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
*
* @section Licence
*   |LICENSE|
*/
/****************************************************************************/
/* The node class used for dag execution                                    */
/* Manages the execution profile, the data, the execution status and time   */
/****************************************************************************/


#include "debug.hh"
// #include "marshalling.hh"
#include "MasterAgent.hh" // for persistent data deletion

#include "Dag.hh"
#include "DagNode.hh"
#include "DagNodePort.hh"
#include "EventTypes.hh"

using namespace events;

/****************************************************************************/
/*                                                                          */
/*                          class WfDataException                           */
/*                                                                          */
/****************************************************************************/
std::string
WfDataException::ErrorMsg() const {
  std::string errorMsg;
  switch (Type()) {
  case eNOTFOUND:
    errorMsg = "Data not found (" + Info() + ")"; break;
  case eNOTAVAIL:
    errorMsg = "Data not available (" + Info() + ")"; break;
  case eWRONGTYPE:
    errorMsg = "Wrong data type (" + Info() + ")"; break;
  case eINVALID_CONTAINER:
    errorMsg = "Invalid container (" + Info() + ")"; break;
  case eID_UNDEF:
    errorMsg = "Undefined data ID (" + Info() + ")"; break;
  case eINVALID_VALUE:
    errorMsg = "Invalid init value (" + Info() + ")"; break;
  case eREADFILERROR:
    errorMsg = "Cannot read file (" + Info() + ")"; break;
  case eVOID_DATA:
    errorMsg = "Void data (" + Info() + ")"; break;
  }
  return errorMsg;
}

/****************************************************************************/
/*                                                                          */
/*                            class DagNode                                 */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*                       Constructors/Destructor                            */
/****************************************************************************/

DagNode::DagNode(const std::string& id, Dag *dag, FWorkflow* wf)
  : WfNode(id), myDag(dag), myWf(wf), isStarted(false), isTerminated(false) {
  if (id.empty()) {
    WARNING("Creating dagNode with empty id - may cause errors");
  }
  this->prevNodesTodoCount = 0;
  this->task_done = false;
  this->profile = NULL;
  this->myLauncher = NULL;
  this->taskExecFailed = false;
  this->nextDone = 0;
  this->priority = 0;
  this->myQueue = NULL;
  this->realCompTime = -1;
  this->realStartTime = -1;
  this->estCompTime = -1;
  this->estDelay = 0;
  this->submitIndex = 0;
}

/**
 * Node destructor
 */
DagNode::~DagNode() {
  if (profile != NULL) {
    diet_profile_free(profile);
  }

  if (myQueue) {
    myQueue->removeNode(this);
  }

  // free the ports map ()
  // free the parameters vectors
  {
    // char vector
    char * cx;
    while (charParams.size() != 0) {
      cx = charParams.back();
      charParams.pop_back();
      delete (cx);
    }
  }
  {
    // short vector
    short * sx = NULL;
    while (shortParams.size() != 0) {
      sx = shortParams.back();
      shortParams.pop_back();
      if (sx) {
        delete(sx);
      }
    }
  }
  {
    // int vector
    int * ix = NULL;
    while (intParams.size() != 0) {
      ix = intParams.back();
      intParams.pop_back();
      if (ix)
        delete (ix);
    }
  }
  {
    // long vector
    long * lx = NULL;
    while (longParams.size() != 0) {
      lx = longParams.back();
      longParams.pop_back();
      if (lx)
        delete (lx);
    }
  }
  {
    // std::string
    char * cx = NULL;
    while (stringParams.size() != 0) {
      cx = stringParams.back();
      stringParams.pop_back();
      if (cx) {
        delete [] cx;
      }
    }
  }

  {
    // file name vector
    char * cx = NULL;
    while (fileParams.size() != 0) {
      cx = fileParams.back();
      fileParams.pop_back();
      if (cx)
        delete [] cx;
    }
  }

  {
    // float vector
    float * fx = NULL;
    while (floatParams.size() != 0) {
      fx = floatParams.back();
      floatParams.pop_back();
      if (fx)
        delete(fx);
    }
  }
  {
    // double vector
    double * dx = NULL;
    while (doubleParams.size() != 0) {
      dx = doubleParams.back();
      doubleParams.pop_back();
      if (dx)
        delete(dx);
    }
  }
}

std::string DagNode::getCompleteId() {
  if (this->myDag != NULL)
    return this->myDag->getId() + "-" + this->myId;
  return this->myId;
}

void
DagNode::setDag(Dag *dag) {
  this->myDag = dag;
}

Dag *
DagNode::getDag() const {
  if (this->myDag != NULL)
    return this->myDag;
  else {
    INTERNAL_ERROR("ERROR: calling getDag() on a node not linked to a dag\n",
                   1);
  }
}

FWorkflow *
DagNode::getWorkflow() const {
  return myWf;
}

void
DagNode::setPbName(const std::string& pbName) {
  this->myPb = pbName;
}

const std::string&
DagNode::getPbName() const {
  return this->myPb;
}

// void
// DagNode::setFNode(FProcNode * fNode) {
//   this->myFNode = fNode;
// }
//
// FProcNode *
// DagNode::getFNode() {
//   return this->myFNode;
// }

/******************************/
/* Base methods override      */
/******************************/

/**
 * (public) Add a new predecessor
 * (may check some constraints before adding the predecessor effectively)
 */
void
DagNode::addNodePredecessor(WfNode * node, const std::string& fullNodeId) {
  // check if predecessor is not already done
  DagNode *predNode = dynamic_cast<DagNode*>(node);
  if (predNode == NULL) {
    INTERNAL_ERROR(__FUNCTION__ << " : Invalid predecessor node type or null ptr", 0);
  }
  // predecessor is added only if not already finished
  if (!predNode->isDone()) {
    addPrevId(fullNodeId);
  }
}

/**
 * (private)
 * Set a new previous node *
 */
void
DagNode::setPrev(int index, WfNode * node) {
  WfNode::setPrev(index, node);
  prevNodesTodoCount++;
}

/**
 * create and add a new port to the node *
 */
WfPort *
DagNode::newPort(std::string portId, unsigned int ind,
                 WfPort::WfPortType portType, WfCst::WfDataType dataType,
                 unsigned int depth) throw(WfStructException) {
  if (isPortDefined(portId))
    throw WfStructException(WfStructException::eDUPLICATE_PORT, "port id="+portId);
  DagNodePort * p = NULL;
  switch (portType) {
  case WfPort::PORT_IN:
    p = new DagNodeInPort(this, portId, dataType, depth, ind);
    break;
  case WfPort::PORT_ARG:
    p = new DagNodeArgPort(this, portId, dataType, depth, ind);
    break;
  case WfPort::PORT_INOUT:
    p = new DagNodeInOutPort(this, portId, dataType, depth, ind);
    break;
  case WfPort::PORT_OUT:
    p = new DagNodeOutPort(this, portId, dataType, depth, ind);
    break;
  default:
    INTERNAL_ERROR("Invalid port type for DagNode", 1);
  }
  this->ports[portId] = p;
  EventManager::getEventMgr()->sendEvent(
    new EventCreateObject<DagNodePort, DagNode>(p, this));
  return (WfPort*) p;
}

std::string
DagNode::toString() const {
  return "DAG_NODE " + getId();
}

void
DagNode::toXML(std::ostream& output) const {
  output << "<node id=\"" << myId << "\" path=\"" << myPb << "\" ";
  if (!estimationClass.empty())
    output << "est-class=\"" << estimationClass << "\" ";
  std::string currState = this->getStatusAsString();
  if (!currState.empty())
    output << "status=\"" << currState << "\" ";
  output << ">\n";
  for (unsigned int ix = 0; ix < getPortNb(); ++ix) {
    const DagNodePort * port = dynamic_cast<const DagNodePort*>(getPortByIndex(ix));
    port->toXML(output);
  }
  output << "</node>\n";
}

/******************************/
/* DIET Profile Mgmt          */
/******************************/

/**
 * return the node profile
 */
diet_profile_t *
DagNode::getProfile() {
  return this->profile;
}

/**
 * return the reqID of the node
 */
diet_reqID_t
DagNode::getReqID(){
  if (this->isDone())
    return this->getProfile()->dietReqID;
  return -1;
}

/**
 * Set the estimation class
 */
void
DagNode::setEstimationClass(const std::string& estimClassId) {
  estimationClass = estimClassId;
}

/**
 * Get the estimation class
 */
const std::string&
DagNode::getEstimationClass() {
  return estimationClass;
}

/**
 * set the submit index
 */
void
DagNode::setSubmitIndex(int idx) {
  this->submitIndex = idx;
}

/**
 * get the submit index
 */
int
DagNode::getSubmitIndex() {
  return this->submitIndex;
}


/**
 * Creates the base profile without the parameters (COMMON)
 */
void
DagNode::createProfile() {
  if (this->profile) {
    WARNING("DagNode::createProfile : profile already defined!!\n");
  }
  int nbIn = 0, nbOut = 0, nbInOut = 0;
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    WfPort * port = (WfPort *) p->second;
    switch (port->getPortType()) {
    case WfPort::PORT_IN:
    case WfPort::PORT_ARG:
      ++nbIn;
      break;
    case WfPort::PORT_INOUT:
      ++nbInOut;
      break;
    case WfPort::PORT_OUT:
      ++nbOut;
      break;
    default:
      INTERNAL_ERROR("Invalid port type for profile creation", 1);
    }
  } // end for all ports
  int last_in    = nbIn - 1;
  int last_inout = last_in + nbInOut;
  int last_out   = last_inout + nbOut;
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating DIET profile: pb=" << myPb
             << " / last_in=" << last_in
             << " / last_inout=" << last_inout
             << " / last_out=" << last_out << "\n");
  this->profile =  diet_profile_alloc((char*)(myPb.c_str()),
                                      last_in, last_inout, last_out);

}

/**
 * Creates the profile before submission of the node (MADAG SIDE)
 */
void
DagNode::initProfileSubmit() {
  createProfile();
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(p->second);
    dagPort->initProfileSubmit();
  }
}

/**
 * Creates the profile before execution of the node (CLIENT SIDE)
 */
void
DagNode::initProfileExec() throw(WfDataException) {
  TRACE_TEXT(TRACE_ALL_STEPS, "Creating profile for Execution\n");
  createProfile();
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(p->second);
    dagPort->initProfileExec();
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "Profile for Execution done (" << myId << ")\n");
}

/**
 * Store the persistent data of the node profile *
 */
void
DagNode::storeProfileData() {
  // store the data IDs for each port
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    if (((WfPort *)p->second)->isOutput()) {
      DagNodeOutPort * dagPort = dynamic_cast<DagNodeOutPort*>(p->second);
      dagPort->storeProfileData();
    }
  }
}

/**
 * Free the profile and the persistent data of the node
 */
void
DagNode::freeProfileAndData(MasterAgent_var& MA) {
  TRACE_TEXT(TRACE_ALL_STEPS,
              myId << " Free profile and release persistent data\n");
  // Free persistent data
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    DagNodePort * dagPort = dynamic_cast<DagNodePort*>(p->second);
    dagPort->freePersistentData(MA);
  }
  if (profile) {
    diet_profile_free(profile);
    profile = NULL;
  }
}

/**
 * Allocate a new char *
 */
char *
DagNode::newChar(const std::string& value) {
  if (value != "") {
    char * cx = new char;
    *cx = value.c_str()[0];
    charParams.push_back(cx);
  }
  else {
    charParams.push_back(NULL);
  }
  return charParams[charParams.size() - 1];
}

/**
 * Allocate a new short *
 */
short *
DagNode::newShort(const std::string& value) {
  if (value != "") {
    short * sx = new short;
    *sx = atoi(value.c_str());
    shortParams.push_back(sx);
  }
  else {
    shortParams.push_back(NULL);
  }
  return shortParams[shortParams.size() - 1];
}

/**
 * Allocate a new int  *
 */
int *
DagNode::newInt(const std::string& value) {
  if (value != "") {
    int * ix = new int;
    *ix = atoi(value.c_str());
    intParams.push_back(ix);
  }
  else {
    intParams.push_back(NULL);
  }
  return intParams[intParams.size() - 1];
}

/**
 * Allocate a new long *
 */
long *
DagNode::newLong(const std::string& value) {
  if (value != "") {
    long * lx = new long;
    *lx = atoi(value.c_str());
    longParams.push_back(lx);
  }
  else {
    longParams.push_back(NULL);
  }
  return longParams[longParams.size() - 1];
}

/**
 * Allocate a new string *
 */
char *
DagNode::newString(const std::string& value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());
  stringParams.push_back(str);
  return str;
}

/**
 * Allocate a new file *
 */
char *
DagNode::newFile(const std::string& value) {
  char * str = new char[value.size()+1];
  strcpy(str, value.c_str());

  //   TRACE_TEXT(TRACE_ALL_STEPS,
  //            "----> new file; value = " << value << ", " << str << "\n");
  fileParams.push_back(str);
  return str;
}

/**
 * Allocate a new float  *
 */
float *
DagNode::newFloat(const std::string& value) {
  float * fx = new float;
  *fx = (float)atof(value.c_str());
  floatParams.push_back(fx);
  return fx;
}

/**
 * Allocate a new double  *
 */
double *
DagNode::newDouble(const std::string& value) {
  double * dx = new double;
  *dx = atof(value.c_str());
  doubleParams.push_back(dx);
  return dx;
}

/**
 * Display the results of the node
 */
void
DagNode::displayResults(std::ostream& output) {
  for (std::map<std::string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    if (((WfPort *)p->second)->isOutput()) {
      DagNodeOutPort * outp = dynamic_cast<DagNodeOutPort *>(p->second);
      std::string currPortFullId =
        outp->getParent()->getId() + "#" + outp->getId();
      if (!outp->isConnected()) {
        output << "## DAG OUTPUT (" << currPortFullId << ") = ";
        if (!hasFailed()) {
          WfListDataWriter  dataWriter(output);
          outp->writeData(&dataWriter);
        } else {
          output << "<error>";
        }
        output << "\n";
      }
    }
  }
}


/******************************/
/* Scheduling                 */
/******************************/

/**
 * set the node priority *
 */
void
DagNode::setPriority(double priority) {
  this->priority = priority;
}

/**
 * get the node priority *
 */
double
DagNode::getPriority() {
  return this->priority;
}

/**
 * Set the NodeQueue ref when the node is inserted into it *
 * (This is used to notify the NodeQueue when the node state changes *
 */
void DagNode::setNodeQueue(NodeQueue * nodeQ) {
  this->myQueue = nodeQ;
}

/**
 * Remove node from the NodeQueue it belongs to (if applicable)
 * This is used in case of dag cancellation
 */
void
DagNode::removeFromNodeQueue() {
  if (myQueue) {
    myQueue->removeNode(this);
  }
}

/**
 * set the ref to the last nodeQueue occupied by the node
 */
void
DagNode::setLastQueue(NodeQueue * queue) {
  this->lastQueue = queue;
}

/**
 * get the ref to the last nodeQueue occupied by the node
 */
NodeQueue *
DagNode::getLastQueue() {
  return this->lastQueue;
}

/******************************/
/* Timestamps (MaDag)         */
/******************************/

/**
 * set the estimated duration
 */
void
DagNode::setEstDuration(double time) {
  this->estDuration = time;
}

/**
 * get the estimated duration
 */
double
DagNode::getEstDuration() {
  return this->estDuration;
}

/**
 * set the estimated completion time
 */
void
DagNode::setEstCompTime(double time) {
  this->estCompTime = time;
}

/**
 * get the estimated completion time
 */
double
DagNode::getEstCompTime() {
  return this->estCompTime;
}

/**
 * set the estimated delay
 * (can be increased or decreased)
 */
void
DagNode::setEstDelay(double delay) {
  std::string traceHeader = "[" + getId() + "] setEstDelay() : ";
  // if this is an exit node then eventually updates the dag delay
  if (this->isAnExit()
      && (delay > this->estDelay)
      && (this->getDag() != NULL)) {
    this->getDag()->setEstDelay(delay);
  }
  this->estDelay = delay;
  TRACE_TEXT(TRACE_ALL_STEPS, traceHeader << "delay = " << delay << "\n");
}

/**
 * set the real start time
 */
void
DagNode::setRealStartTime(double time) {
  this->realStartTime = time;
}

/**
 * get the real start time
 */
double
DagNode::getRealStartTime() {
  return this->realStartTime;
}

/**
 * get the estimated delay
 */
double
DagNode::getEstDelay() {
  return this->estDelay;
}

/**
 * set the real completion time
 */
void
DagNode::setRealCompTime(double time) {
  this->realCompTime = time;
}

/**
 * get the real completion time
 */
double
DagNode::getRealCompTime() {
  return this->realCompTime;
}

/**
 * get the real delay (positive) or 0 if no delay
 * or -1 if needed timestamps not set
 */
double
DagNode::getRealDelay() {
  if ((this->realCompTime != -1) && (this->estCompTime != -1)) {
    double delay = this->realCompTime - this->estCompTime;
    if (delay > 0) return delay;
    else return 0;
  } else return -1;
}

/**********************************/
/* Execution                      */
/**********************************/

/**
 * test if the node is ready for execution (Madag side)
 * (check the counter of dependencies)
 */
bool
DagNode::isReady() const {
  return (prevNodesTodoCount == 0);
}

/**
 * set the node as ready for execution (Madag side)
 * (Notifies the nodequeue if available)
 */
void
DagNode::setAsReady(DagScheduler* scheduler) {
  if (this->myQueue != NULL) {
    this->myQueue->notifyStateChange(this);
  }
  if (scheduler) {
    scheduler->handlerNodeReady(this);
  }
}

/**
 * start the node execution *
 */

void DagNode::start(DagNodeLauncher * launcher) {
  TRACE_TEXT(TRACE_ALL_STEPS, "[" << getId() << "] : starting\n");
  myLauncher = launcher;
  myLauncher->start();
  isStarted = true;
}

/**
 * terminate the node execution (thread)
 */
void DagNode::terminate() {
  std::string pfx = " [" + getCompleteId() + "] : ";
  if (!isStarted) {
    WARNING(__FUNCTION__ << pfx << "cannot terminate thread not started\n");
    return;
  }
  if (isTerminated) {
    WARNING(__FUNCTION__ << pfx << "cannot terminate thread already terminated\n");
    return;
  }
  TRACE_TEXT(TRACE_ALL_STEPS, pfx << "waiting for node termination\n");
  myLauncher->join();
  delete myLauncher;
  isTerminated = true;
}

/**
 * test if the node is running
 */
bool
DagNode::isRunning() const {
  return (isStarted && !isTerminated);
}

/**
 * test if the execution is done (MaDag side)
 * (still used by HEFTscheduler to rank nodes)
 */
bool
DagNode::isDone() const {
  return task_done;
}

/**
 * Set the node status as done (MaDag & client side)
 */
void
DagNode::setAsDone(DagScheduler* scheduler) {
  // update node scheduling info
  task_done = true;
  // the following applies only to MaDag
  if (scheduler) {
    setRealCompTime(scheduler->getRelCurrTime());
    // notify scheduler that node is done (depending on the scheduler, this may trigger
    //  a recursive update of the realCompTime in other nodes)
    scheduler->handlerNodeDone(this);
  }
  // notify the dag
  this->getDag()->setNodeDone(this, scheduler);
}

/**
 * Called when a previous node execution is done (MaDag side)
 */
void DagNode::prevNodeHasDone(DagScheduler* scheduler) {
  prevNodesTodoCount--;
  if (this->isReady()) {
    this->setAsReady(scheduler);
  }
}

/**
 * called when the node execution failed (MaDag & client side) *
 */
void
DagNode::setAsFailed(DagScheduler* scheduler) {
  taskExecFailed =  true;
  this->getDag()->setNodeFailure(this->getId(), scheduler);
}

/**
 * test if the execution failed (client side)
 */
bool
DagNode::hasFailed() const {
  return taskExecFailed;
}

void
DagNode::setStatus(const std::string& statusStr) {
  if (statusStr == "done")
    this->setAsDone();
  else if (statusStr ==  "running") {
    // nothing to do this status cannot be set because it is dynamic
  } else if (statusStr == "failed")
    this->setAsFailed();
  else if (statusStr == "ready") {
    // nothing to do: this status depends on predecessors status
  } else {
    INTERNAL_ERROR(__FUNCTION__ <<"Wrong status value\n", 1);
  }
}

std::string
DagNode::getStatusAsString() const {
  if (this->isDone()) return "done";
  if (this->isRunning()) return "running";
  if (this->hasFailed()) return "failed";
  if (this->isReady()) return "ready";
  return "";
}
