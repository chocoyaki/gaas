/**
* @file  FWorkflow.cc
* 
* @brief  The class representing a functional workflow Contains the workflow node
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* The class representing a functional workflow                             */
/* Contains the workflow nodes and manages their instanciation as one or    */
/* several dags.                                                            */
/*                                                                          */
/****************************************************************************/

#include <map>
#include <functional>

#include "debug.hh"
#include "Dag.hh"
#include "DagWfParser.hh"
#include "FWorkflow.hh"
#include "FActivityNode.hh"
#include "FIfNode.hh"
#include "FLoopNode.hh"
#include "EventTypes.hh"


FWorkflow::FWorkflow(std::string id, std::string name)
  : FProcNode(NULL, id), myName(name) {
}


FWorkflow::FWorkflow(std::string id, std::string name, FWorkflow*  parentWf)
  : FProcNode(parentWf, id), myName(name) {
}

FWorkflow::~FWorkflow() {
  // free nodes
  while (!myInterface.empty()) {
    FNode * p = myInterface.begin()->second;
    myInterface.erase(myInterface.begin());
    delete p;
  }
  while (!myProc.empty()) {
    FProcNode * p = myProc.begin()->second;
    myProc.erase(myProc.begin());
    delete p;
  }
}

std::string
FWorkflow::getName() const {
  return myName;
}

FWorkflow*
FWorkflow::getRootWorkflow() const {
  if (getWorkflow() == NULL) {
    return const_cast<FWorkflow*>(this);
  }
  return getWorkflow()->getRootWorkflow();
}

WfNode *
FWorkflow::getNode(const std::string& nodeId) throw(WfStructException) {
  WfNode * node;
  try {
    node = (WfNode *) getProcNode(nodeId);
  } catch (WfStructException& e) {
    node = (WfNode *) getInterfaceNode(nodeId);
  }
  return node;
}

FProcNode *
FWorkflow::getProcNode(const std::string& id) throw(WfStructException) {
  FProcNode *node = NULL;
  myLock.lock();    /** LOCK */
  std::map<std::string, FProcNode*>::iterator p = this->myProc.find(id);
  if (p != this->myProc.end()) {
    node = p->second;
  }
  myLock.unlock();  /** UNLOCK */
  if (node == NULL) {
    throw WfStructException(WfStructException::eUNKNOWN_NODE, "FNode id=" + id);
  }
  return node;
}

FNode *
FWorkflow::getInterfaceNode(const std::string& id) throw(WfStructException) {
  FNode *node = NULL;
  myLock.lock();    /** LOCK */
  std::map<std::string, FNode*>::iterator p = this->myInterface.find(id);
  if (p != this->myInterface.end()) {
    node = p->second;
  }
  myLock.unlock();  /** UNLOCK */
  if (node == NULL) {
    throw WfStructException(WfStructException::eUNKNOWN_NODE,
                            "FNode id=" + id);
  }
  return node;
}

void
FWorkflow::checkPrec(NodeSet* contextNodeSet) throw(WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "CHECKING WF PRECEDENCE START\n");
  // processor nodes
  for (std::map<std::string, FProcNode * >::iterator p = myProc.begin();
       p != myProc.end();
       ++p) {
    FProcNode * node = (FProcNode*) p->second;
    node->setNodePrecedence(contextNodeSet);
    // sub-workflows
    FWorkflow* subWf = dynamic_cast<FWorkflow*>(node);
    if (subWf) {
      subWf->checkPrec(subWf);
    }
  }
  // interface nodes (does sth only for sink nodes)
  for (std::map<std::string, FNode * >::iterator p = myInterface.begin();
       p != myInterface.end();
       ++p) {
    FNode * node = (FNode *) p->second;
    node->setNodePrecedence(contextNodeSet);
  }
  // ports of workflow (for sub-workflows only)
  for (std::map<std::string, WfPort*>::iterator portIter = ports.begin();
       portIter != ports.end();
       ++portIter) {
    FNodePort* wfPort = dynamic_cast<FNodePort*>((WfPort*) portIter->second);
    try {
      FNode* interfNode = getInterfaceNode(wfPort->getInterfaceRef());
      interfNode->connectToWfPort(wfPort);
    } catch (const WfStructException& e) {
      throw WfStructException(e.Type(),
                              std::string("Unknown reference"
                                          "for sub-workflow interface : ")
                              + e.Info());
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "CHECKING WF PRECEDENCE END\n");
}

FActivityNode*
FWorkflow::createActivity(const std::string& id) throw(WfStructException) {
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating activity node : " << id << "\n");
    FActivityNode* node = new FActivityNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE, "FNode id=" + id);
}

FIfNode*
FWorkflow::createIf(const std::string& id) throw(WfStructException) {
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating IF node : " << id << "\n");
    FIfNode* node = new FIfNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE, "FNode id=" + id);
}

FMergeNode*
FWorkflow::createMerge(const std::string& id) throw(WfStructException) {
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating MERGE node : " << id << "\n");
    FMergeNode* node = new FMergeNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FFilterNode*
FWorkflow::createFilter(const std::string& id) throw(WfStructException) {
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating FILTER node : " << id << "\n");
    FFilterNode* node = new FFilterNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FLoopNode*
FWorkflow::createLoop(const std::string& id) throw(WfStructException) {
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating LOOP node : " << id << "\n");
    FLoopNode* node = new FLoopNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FWorkflow*
FWorkflow::createSubWorkflow(const std::string& id, const std::string& name)
  throw(WfStructException) {
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating sub-workflow node : " << id << "\n");
    FWorkflow* node = new FWorkflow(id, name, this);
    events::EventManager::getEventMgr()->sendEvent(
      new events::EventCreateObject<FWorkflow, FWorkflow>(node, this));
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FSourceNode*
FWorkflow::createSource(const std::string& id, WfCst::WfDataType type)
  throw(WfStructException) {
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating source node : " << id << "\n");
    FSourceNode* node = new FSourceNode(this, id, type);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FConstantNode*
FWorkflow::createConstant(const std::string& id, WfCst::WfDataType type)
  throw(WfStructException) {
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating constant node : " << id << "\n");
    FConstantNode* node = new FConstantNode(this, id, type);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

FSinkNode*
FWorkflow::createSink(const std::string& id, WfCst::WfDataType type,
                      unsigned int depth) throw(WfStructException) {
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT(TRACE_ALL_STEPS, "Creating sink node : " << id << "\n");
    FSinkNode* node = new FSinkNode(this, id, type, depth);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,
                          "FNode id=" + id);
}

/**
 * Specific DFS search method
 * Skips the back nodes
 * NOT THREAD SAFE
 */

struct DFSNodeInfo {
  bool explored;
  bool ongoing;
  short end;

  DFSNodeInfo() {
  }

  DFSNodeInfo(bool _expl, bool _ongoing, short _end)
    : explored(_expl), ongoing(_ongoing), end(_end) {
  }
};

void
DFS(WfNode* node, std::map<WfNode*, DFSNodeInfo>& DFSInfo, short& endCount) {
  DFSInfo[node].ongoing = true;
  TRACE_TEXT(TRACE_ALL_STEPS, "Starting DFS search for node : "
             << node->getId() << "\n");
  for (std::list<WfNode*>::iterator nextIter = node->nextNodesBegin();
       nextIter != node->nextNodesEnd();
       ++nextIter) {
    WfNode* nextNode = (WfNode*) *nextIter;
    TRACE_TEXT(TRACE_ALL_STEPS, "  Processing next node : "
               << nextNode->getId() << "\n");
    // look for next node DFS information
    std::map<WfNode*, DFSNodeInfo>::iterator nextNodeInfoSearch =
      DFSInfo.find(nextNode);
    // if not found skip the node (means next node is not a processor)
    if (nextNodeInfoSearch == DFSInfo.end()) {
      continue;
    }
    DFSNodeInfo& nextNodeInfo = nextNodeInfoSearch->second;
    if (!nextNodeInfo.explored) {
      if (!nextNodeInfo.ongoing) {
        DFS(nextNode, DFSInfo, endCount);
      } else {
        // this is a BACK edge
        TRACE_TEXT(TRACE_ALL_STEPS, "Found back edge!!\n");
      }
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS, "  Next node was already explored\n");
    }
  }
  DFSInfo[node].explored = true;
  DFSInfo[node].end      = endCount++;
  TRACE_TEXT(TRACE_ALL_STEPS, "End of DFS search for node : " << node->getId()
             << " (count=" << endCount-1 << ")\n");
}

/**
 * FWorkflow initialization
 * NOT THREAD SAFE
 * FIXME make it private ?
 */
void
FWorkflow::initialize() {
  if (getPortNb() > 0) {  // used for sub-workflows
    FProcNode::initialize();
  }

  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Sorting processors...\n");
  // create a topologically sorted list of processor nodes
  std::map<WfNode*, DFSNodeInfo>* DFSInfo =
    new std::map<WfNode*, DFSNodeInfo>();
  short DFSEndCount = 0;
  // -> initialization of DFSInfo
  std::map<std::string, FProcNode*>::iterator iter = myProc.begin();
  for (; iter != myProc.end(); ++iter) {
    WfNode* node = (WfNode*) iter->second;
    (*DFSInfo)[node] = DFSNodeInfo(false, false, 0);
  }
  // -> DFS search
  iter = myProc.begin();
  for (; iter != myProc.end(); ++iter) {
    WfNode* node = (WfNode*) iter->second;
    if (!(*DFSInfo)[node].explored) {
      DFS(node,*DFSInfo, DFSEndCount);
    }
  }
  // -> create sorted todo list
  iter = myProc.begin();
  for (; iter != myProc.end(); ++iter) {
    FProcNode* node = (FProcNode*) iter->second;
    if (todoProc.empty()) {
      todoProc.push_back(node);
    } else {
      // insertion sort
      short insNodePrio = (*DFSInfo)[(WfNode*)node].end;
      std::list<FProcNode*>::iterator todoIter = todoProc.begin();
      while ((insNodePrio < (*DFSInfo)[(WfNode*) *todoIter].end)
             && (todoIter != todoProc.end())) {
        ++todoIter;
      }
      todoProc.insert(todoIter, node);
    }
  }

  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Connecting all nodes...\n");
  iter = myProc.begin();
  for (; iter != myProc.end(); ++iter) {
    ((FProcNode*) iter->second)->connectNodePorts();
  }
  std::map<std::string, FNode*>::iterator iter2 = myInterface.begin();
  for (; iter2 != myInterface.end(); ++iter2) {
    ((FNode*) iter2->second)->connectNodePorts();
  }

  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Initializing Interface...\n");
  iter2 = myInterface.begin();
  for (; iter2 != myInterface.end(); ++iter2) {
    try {
      ((FNode*) iter2->second)->initialize();
    } catch (WfDataException& e) {
      WARNING("Data failure during node " << ((FNode*) iter2->second)->getId()
              << " initialization:\n" << e.ErrorMsg() << "\n");
    } catch (WfDataHandleException& e) {
      WARNING("Failure during node " << ((FNode*) iter2->second)->getId()
              << " initialization:\n" << e.ErrorMsg() << "\n");
    }
  }
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Initializing Processors...\n");
  iter = myProc.begin();
  for (; iter != myProc.end(); ++iter) {
    ((FProcNode*) iter->second)->initialize();
  }
}

std::string
FWorkflow::toString() const {
  return "WORKFLOW " + getId();
}

/**
 * Set the path of the file used for data sources instanciation
 */
void
FWorkflow::setDataSrcXmlFile(const std::string& dataFileName) {
  this->dataSrcXmlFile = dataFileName;
}

/**
 * Returns the path of the file used for data sources instanciation
 */
const std::string&
FWorkflow::getDataSrcXmlFile() {
  return dataSrcXmlFile;
}

/**
 * FWorkflow instanciation
 * NOT THREAD SAFE
 */

void
FWorkflow::instanciate(Dag * dag) {
  // re-initialize status
  myStatus = N_INSTANC_READY;

  TRACE_TEXT(TRACE_MAIN_STEPS, traceId() << "########## WORKFLOW '" << getId()
             << "' INSTANCIATION START ##########\n");
  // used for sub-workflows
  if (getPortNb() > 0) {
      // run the iterator and call createReal/VoidInstance
    FProcNode::instanciate(dag);
  }
  TRACE_TEXT(TRACE_ALL_STEPS,
             traceId() << "###### Instanciate sources/constants ...\n");
  // instanciate the INTERFACE (this will create the input data items
  // if source is XML)
  for (std::map<std::string, FNode*>::iterator iter = myInterface.begin();
       iter != myInterface.end();
       ++iter) {
    // CONSTANT NODES
    FConstantNode* cstNode =
      dynamic_cast<FConstantNode*>((FNode*) iter->second);
    if (cstNode) {
      cstNode->instanciate(dag);
    }
    // SOURCE NODES
    FSourceNode* srcNode = dynamic_cast<FSourceNode*>((FNode*) iter->second);
    if (srcNode) {
      srcNode->resumeInstanciation();
      if (srcNode->instanciationReady()) {
        try {
          srcNode->instanciate(dag);
        } catch (WfDataHandleException& e) {
          WARNING("Failure during node " << srcNode->getId()
                  << " instanciation:\n" << e.ErrorMsg());
        }
      }
      if (srcNode->instanciationCompleted()) {
        srcNode->finalize();
      }
    }
  }

  TRACE_TEXT(TRACE_ALL_STEPS,
             traceId() << "###### Instanciate processors...\n");
  // resume instanciation (if node was put on hold during a previous
  // instanciate call)
  for (std::list<FProcNode*>::iterator iter = todoProc.begin();
       iter != todoProc.end();
       ++iter) {
    ((FProcNode*) *iter)->resumeInstanciation();
  }

  // loop until all nodes have no data to process
  bool dataToProcess = true;
  while (dataToProcess) {
    // for each node in the todo list
    std::list<FProcNode*>::iterator procIter = todoProc.begin();
    while (procIter != todoProc.end()) {
      FProcNode* currProc = (FProcNode*) *procIter;

      // instanciate node
      TRACE_TEXT(TRACE_MAIN_STEPS,
                 "  ## INSTANCIATE NODE : " << currProc->getId() << "\n");
      try {
        currProc->instanciate(dag);
      } catch (WfDataHandleException& e) {
        WARNING("Failure during node " << currProc->getId()
                << " instanciation:\n"
                << e.ErrorMsg());
      } catch (WfDataException& e) {
        WARNING("Data failure during node " << currProc->getId()
                << " instanciation:"
                << "\n" << e.ErrorMsg());
      }
      // if on hold, then set the whole wf on hold
      if (currProc->instanciationOnHold()) {
        myStatus = N_INSTANC_ONHOLD;
      }
      if (currProc->instanciationPending()) {
        myStatus = N_INSTANC_PENDING;
      }
      // if fully instanciated, remove from todo list
      if (currProc->instanciationCompleted()) {
        currProc->finalize();
        TRACE_TEXT(TRACE_MAIN_STEPS, "#### Processor " << currProc->getId()
                   << " instanciation COMPLETE\n");
        procIter = todoProc.erase(procIter);
        continue;
      }
      ++procIter;
    }

    TRACE_TEXT(TRACE_ALL_STEPS,
               traceId() << "###### Check if still data to process...\n");
    // check if nodes have still some data to process
    // (this may happen in case of workflow restart with while loops when
    // activitie inside the loop are re-used, ie they are already completed;
    // then outputs of these activities are submitted to the while node
    // during the same round)
    dataToProcess = false;
    for (std::list<FProcNode*>::iterator procIter = todoProc.begin();
         procIter != todoProc.end();
         ++procIter) {
      FProcNode *proc = (FProcNode*) *procIter;
      if (proc->hasDataToProcess() && !proc->instanciationOnHold()) {
        dataToProcess = true;
        break;
      }
    }
    TRACE_TEXT(TRACE_ALL_STEPS,
               traceId() << (dataToProcess ? "YES" : "NO") << "\n");
  } // end while (dataToProcess)

  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "###### Instanciate sinks ...\n");
  for (std::map<std::string, FNode*>::iterator iter = myInterface.begin();
       iter != myInterface.end();
       ++iter) {
    FSinkNode* sinkNode = dynamic_cast<FSinkNode*>((FNode*) iter->second);
    if (sinkNode) {
      sinkNode->instanciate(dag);
      if (sinkNode->instanciationCompleted()) {
        sinkNode->finalize();
      }
    }
  }

  if (instanciationOnHold()) {
    TRACE_TEXT(TRACE_MAIN_STEPS,
               traceId() << "########## WORKFLOW INSTANCIATION ON HOLD "
               "##########\n");
  } else if (instanciationPending() || !pendingNodes.empty()) {
    // check if instanciation is pending on node execution
    // the first condition happens when one of the nodes (a sub-wf) is pending
    TRACE_TEXT(TRACE_MAIN_STEPS,
               traceId() << "########## WORKFLOW INSTANCIATION PENDING "
               "##########\n");
    myStatus = N_INSTANC_PENDING;
  } else if (todoProc.empty()) {
  // check if instanciation is finished
    TRACE_TEXT(TRACE_MAIN_STEPS,
               traceId() << "############ WORKFLOW INSTANCIATION END "
               "############\n");
    // check if sinks are completed too
    for (std::map<std::string, FNode*>::iterator iter = myInterface.begin();
         iter != myInterface.end();
         ++iter) {
      FSinkNode* sinkNode = dynamic_cast<FSinkNode*>((FNode*) iter->second);
      if (sinkNode && !sinkNode->instanciationCompleted()) {
        WARNING(traceId() << "Sink '"
                << sinkNode->getId()
                << " is not complete (instanciation error)\n");
      }
    }
    myStatus = N_INSTANC_END;
  } else if (getPortNb() > 0) {  // used for sub-workflows
    TRACE_TEXT(TRACE_MAIN_STEPS,
               traceId()
               << "############ WORKFLOW INSTANCIATION NOT FINISHED"
               "############\n");
  } else {
  // incorrect status
    WARNING(traceId() << "Instanciation stopped in incorrect state!!");
    myStatus = N_INSTANC_STOPPED;
    std::list<FProcNode*>::iterator procIter = todoProc.begin();
    for (; procIter != todoProc.end(); ++procIter) {
      WARNING(traceId() << "Functional node remaining in TODO list: "
              << ((FProcNode*) *procIter)->getId() << "\n");
    }
  }
  // add dag to my list of dags
  myDags.push_back(dag);
}

/**
 * Create one instance of the workflow (USED FOR SUB-WORKFLOWS)
 */
void
FWorkflow::createRealInstance(Dag* dag,
                              const FDataTag& currTag,
                              std::vector<FDataHandle*>& currDataLine) {
  // LOOP for each source
  std::map<std::string, FNode *>::iterator interfIter = myInterface.begin();
  for (; interfIter != myInterface.end(); ++interfIter) {
    FSourceNode * source =
      dynamic_cast<FSourceNode*>((FNode*) interfIter->second);
    if (source) {
      source->createInstance(currTag, currDataLine);
    }
  } // end for interface nodes
}

/**
 * Create one VOID instance of the workflow (USED FOR SUB-WORKFLOWS)
 */
void
FWorkflow::createVoidInstance(const FDataTag& currTag,
                              std::vector<FDataHandle*>& currDataLine) {
  createRealInstance(NULL, currTag, currDataLine);
}

/**
 * Update instanciation status: USED FOR SUB-WORKFLOWS ONLY
 */
void
FWorkflow::updateInstanciationStatus() {
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {

      // LOOP for each source - set them as completed
      // (sources cannot update their status themselves as they don't use an
      // iterator)
      std::map<std::string, FNode *>::iterator interfIter = myInterface.begin();
      for (; interfIter != myInterface.end(); ++interfIter) {
        FSourceNode * source =
          dynamic_cast<FSourceNode*>((FNode*) interfIter->second);
        if (source) {
          source->setInstanciationCompleted();
        }
      } // end for interface nodes

      TRACE_TEXT(TRACE_ALL_STEPS,
                 traceId() << "########## ALL INPUTS PROCESSED\n");
      // status not updated here but in the instanciate() method because
      // instanciation is completed only after all nodes of the wf have been
      // completed
    } else {
      TRACE_TEXT(TRACE_ALL_STEPS,
                 traceId() << "########## WAITING FOR INPUTS \n");
      myStatus = N_INSTANC_READY;
    }
  }
}

void
FWorkflow::downloadSinkData() {
  // display all sink results
  std::map<std::string, FNode*>::const_iterator nodeIter = myInterface.begin();
  for (; nodeIter != myInterface.end(); ++nodeIter) {
    FSinkNode *sink = dynamic_cast<FSinkNode*>((FNode*) nodeIter->second);
    if (sink != NULL) {
      sink->downloadResults();
    }
  }
}

void
FWorkflow::displayAllResults(std::ostream& output) {
  // display cancelled dags
  std::list<Dag*>::const_iterator dagIter = myDags.begin();
  for (; dagIter != myDags.end(); ++dagIter) {
    Dag * currDag = (Dag*) *dagIter;
    if (currDag->isCancelled()) {
      output  << "** DAG " << currDag->getId()
              << " was cancelled => no results **\n";
    }
  }
  // display all sink results
  std::map<std::string, FNode*>::const_iterator nodeIter = myInterface.begin();
  for (; nodeIter != myInterface.end(); ++nodeIter) {
    FSinkNode *sink = dynamic_cast<FSinkNode*>((FNode*) nodeIter->second);
    if (sink != NULL) {
      sink->displayResults(output);
    }
  }
}

void
FWorkflow::getSinkContainer(const std::string& sinkName,
                            std::string& containerID) {
  FNode* interfNode = getInterfaceNode(sinkName);
  FSinkNode *sink = dynamic_cast<FSinkNode*>(interfNode);
  if (sink) {
    try {
      sink->getResultsInContainer(containerID);
    } catch (WfDataHandleException& e) {
      WARNING("Cannot get sink container ID (" << e.ErrorMsg() << ")\n");
    }
  } else {
    throw WfStructException(WfStructException::eUNKNOWN_NODE,
                            "Sink name=" + sinkName);
  }
}

void
FWorkflow::displayDagSummary(std::ostream& output) {
  output << "** INSTANCIATED DAGS SUMMARY:\n";
  std::list<Dag*>::iterator dagIter = myDags.begin();
  for (; dagIter != myDags.end(); ++dagIter) {
    Dag * dag = (Dag*) *dagIter;
    output << "DAG ID '" << dag->getId() << "': "
           << "size = " << dag->size() << " node(s), "
           << "makespan = " << dag->getMakespan() << " ms\n";
  }
}

void
FWorkflow::setPendingInstanceInfo(DagNode * dagNode, FDataHandle * dataHdl,
                                  FNodeOutPort * outPort,
                                  FNodeInPort * inPort) {
  pendingDagNodeInfo_t info;
  info.dataHdl = dataHdl;
  info.outPort = outPort;
  info.inPort  = inPort;
  pendingNodes.insert(std::make_pair(dagNode, info));
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "new PENDING dag node ("
             << dagNode->getId() << ") for IN port (" << inPort->getCompleteId()
             << ")\n");
}

/**
 * Handle end of node execution
 */
void
FWorkflow::handlerDagNodeDone(DagNode* dagNode) {
  bool statusChange = false;
  myLock.lock();    /** LOCK */
  // search the instance in the pending list
  std::multimap<DagNode*,
                pendingDagNodeInfo_t>::iterator pendingIter, pendingStart;
  pendingStart = pendingIter = pendingNodes.lower_bound(dagNode);
  // if found, resubmit the datahandle to the in port(s)
  while (pendingIter != pendingNodes.upper_bound(dagNode)) {
    pendingDagNodeInfo_t info = (pendingDagNodeInfo_t) (pendingIter++)->second;
    TRACE_TEXT(TRACE_ALL_STEPS,
               traceId() << " ## RETRY DATA SUBMISSION FOR INSTANCE : "
               << dagNode->getId() << "\n");

    try {
      // update the data ID
      info.dataHdl->downloadDataID();
      // submit data to the in port
      info.outPort->reSendData(info.dataHdl, info.inPort);
    } catch (WfDataHandleException& e) {
      WARNING("Instanciator error due to data handle error :" << e.ErrorMsg());
      myStatus = N_INSTANC_STOPPED;
    } catch (WfDataException& e) {
      WARNING("Instanciator error due to invalid dag data :" << e.ErrorMsg());
      myStatus = N_INSTANC_STOPPED;
    }
    // re-start instanciation
    statusChange = true;

    TRACE_TEXT(TRACE_ALL_STEPS, traceId() << " ## END OF RETRY FOR INSTANCE : "
               << dagNode->getId() << "\n");
  }
  // remove entries from the pending list
  pendingNodes.erase(pendingStart, pendingIter);
  // re-initialize instanciation
  if (statusChange) {
    resumeInstanciation();
  }

  myLock.unlock();  /** UNLOCK */
}

/**
 * Status
 */
void
FWorkflow::writeAllDagsState(std::ostream& output) {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Writing workflow status ...\n");
  output << "<dags>\n";
  std::list<Dag*>::iterator dagIter = myDags.begin();
  for (; dagIter != myDags.end(); ++dagIter) {
    Dag *currDag = ((Dag*) *dagIter);
    if (!currDag->size() == 0) {
      currDag->toXML(output);
    }
  }
  output << "</dags>\n";
}

void
FWorkflow::writeAllSourcesState(std::ostream& output) {
  // my own sources
  for (std::map<std::string, FNode*>::iterator iter = myInterface.begin();
       iter != myInterface.end();
       ++iter) {
    FSourceNode* srcNode = dynamic_cast<FSourceNode*>((FNode*) iter->second);
    if (srcNode && !srcNode->isConnectedToWfPort()) {
      srcNode->toXML(output);
    }
  }
  // idem for all sub-workflows
  for (std::map<std::string, FProcNode*>::iterator iter = myProc.begin();
       iter != myProc.end();
       ++iter) {
    FWorkflow* subWf = dynamic_cast<FWorkflow*>((FProcNode*) iter->second);
    if (subWf) {
      subWf->writeAllSourcesState(output);
    }
  }
}

void
FWorkflow::writeAllSinksState(std::ostream& output) {
  // my own sinks
  std::map<std::string, FNode*>::iterator iter = myInterface.begin();
  for (; iter != myInterface.end(); ++iter) {
    FSinkNode* sinkNode = dynamic_cast<FSinkNode*>((FNode*) iter->second);
    if (sinkNode && !sinkNode->isConnectedToWfPort()) {
      sinkNode->toXML(output);
    }
  }
}

void
FWorkflow::writeAllSourcesAndSinksData(std::ostream& output) {
  output << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  output << "<data>\n";
  this->writeAllSourcesState(output);
  this->writeAllSinksState(output);
  output << "</data>\n";
}

/**
 * Restart workflow
 * This method stores a ptr to the dag nodes that are already done in the
 * transcriptNodes std::map, and include the transcript dags (if not empty) in the workflow
 * dag list
 */
void
FWorkflow::readDagsState(std::list<Dag*>& dagList) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             traceId() << "Using previous execution transcript ...\n");
  while (!dagList.empty()) {
    Dag *currDag = dagList.front();
    if (currDag) {
      // iterate over all nodes
      //        if done => store (node ID ====> dagNode ptr)
      //        if not done => remove from dag
      std::map<std::string, DagNode *>::iterator nodeIter = currDag->begin();
      for (; nodeIter != currDag->end(); ++nodeIter) {
        DagNode * currNode = nodeIter->second;
        if (currNode->isDone()) {
          transcriptNodes[currNode->getId()] = currNode;
        } else {
          currDag->removeNode(currNode->getId());
          delete currNode;
        }
      }
      // store the cleaned dag in current workflow's dag list
      if (!currDag->size() == 0)  {
        myDags.push_back(currDag);
      } else {
        delete currDag;
      }
    }
    dagList.pop_front();
  }
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << " ==> " << transcriptNodes.size()
             << " executed nodes found\n");
}

void
FWorkflow::findDagNodeTranscript(const std::string& dagNodeId,
                                 DagNode* & dagNodePtr,
                                 bool& isDone) {
  std::map<std::string, DagNode*>::iterator nodeIter =
    transcriptNodes.find(dagNodeId);
  if (nodeIter != transcriptNodes.end()) {
    // node found in current workflow transcript std::map
    dagNodePtr = (DagNode*) nodeIter->second;
    isDone = dagNodePtr->isDone();
  } else if (getWorkflow() != NULL) {
    // ask parent workflow (if sub-workflow)
    getWorkflow()->findDagNodeTranscript(dagNodeId, dagNodePtr, isDone);
  } else {
    // node not found
    dagNodePtr = NULL;
    isDone = false;
  }
}

/**
 * Memory free
 */

void
FWorkflow::deleteAllResults() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Deleting all dag results ...\n");
  for (std::list<Dag*>::iterator dagIter = myDags.begin();
       dagIter != myDags.end();
       ++dagIter) {
    ((Dag*) *dagIter)->deleteAllResults();
  }
}

void
FWorkflow::deleteAllInputData(MasterAgent_var& MA) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             traceId() << "Deleting all wf input data...\n");
  for (std::map<std::string, FNode*>::iterator iter = myInterface.begin();
       iter != myInterface.end();
       ++iter) {
    ((FNode*) iter->second)->freeNodePersistentData(MA);
  }
}

void
FWorkflow::deleteAllIntermediateData(MasterAgent_var& MA) {
  TRACE_TEXT(TRACE_ALL_STEPS,
             traceId() << "Deleting all wf intermediate data...\n");
  for (std::map<std::string, FProcNode*>::iterator iter = myProc.begin();
       iter != myProc.end();
       ++iter) {
    ((FNode*) iter->second)->freeNodePersistentData(MA);
  }
}

void
FWorkflow::freeNodePersistentData(MasterAgent_var& MA) {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Deleting sub-workflow data...\n");
  deleteAllInputData(MA);
  deleteAllIntermediateData(MA);
}

void
FWorkflow::deleteAllDags() {
  TRACE_TEXT(TRACE_ALL_STEPS, traceId() << "Deleting all wf dags ...\n");
  while (!myDags.empty()) {
    Dag * p = myDags.front();
    myDags.pop_front();
    delete p;
  }
}
