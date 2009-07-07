/****************************************************************************/
/* The class representing a functional workflow                             */
/* Contains the workflow nodes and manages their instanciation as one or    */
/* several dags.                                                            */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.16  2009/07/07 09:03:22  bisnard
 * changes for sub-workflows (FWorkflow class now inherits from FProcNode)
 *
 * Revision 1.15  2009/06/23 12:23:58  bisnard
 * removed dependencies (includes)
 *
 * Revision 1.14  2009/06/15 12:11:13  bisnard
 * use new XML Parser (SAX) for data source file
 * use new class WfValueAdapter to avoid data duplication
 * use new method FNodeOutPort::storeData
 * changed method to compute total nb of data items
 *
 * Revision 1.13  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.12  2009/05/15 11:10:20  bisnard
 * release for workflow conditional structure (if)
 *
 * Revision 1.11  2009/04/17 08:54:43  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.10  2009/04/09 09:56:20  bisnard
 * refactoring due to new class FActivityNode
 *
 * Revision 1.9  2009/04/08 09:34:56  bisnard
 * pending nodes mgmt moved to FWorkflow class
 * FWorkflow and FNode state graph revisited
 * FNodePort instanciation refactoring
 *
 * Revision 1.8  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.7  2009/01/16 16:31:54  bisnard
 * added option to specify data source file name
 *
 * Revision 1.6  2009/01/16 13:54:50  bisnard
 * new version of the dag instanciator with iteration strategies for nodes with multiple input ports
 *
 * Revision 1.5  2008/12/09 12:15:59  bisnard
 * pending instanciation handling (uses dag outputs for instanciation
 * of a functional wf)
 *
 * Revision 1.4  2008/12/02 14:17:49  bisnard
 * manage multi-dag cancellation when one dag fails
 *
 * Revision 1.3  2008/12/02 10:09:36  bisnard
 * added instanciation methods
 *
 * Revision 1.2  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.1  2008/10/20 08:09:58  bisnard
 * new classes for functional workflows
 *
 */
//#include <boost/shared_ptr.hpp>
#include <map>
#include <functional>

#include "debug.hh"
#include "DagWfParser.hh"
#include "FWorkflow.hh"
#include "FActivityNode.hh"
#include "FIfNode.hh"
#include "FLoopNode.hh"

FWorkflow::FWorkflow(const string& id,
                     FWorkflow*  parentWf)
  : FProcNode(parentWf, id) {
}

FWorkflow::~FWorkflow() {
  TRACE_TEXT (TRACE_ALL_STEPS,"~FWorkflow() destructor ..." <<  endl);
  // free nodes
  while (! myInterface.empty() ) {
    FNode * p = myInterface.begin()->second;
    myInterface.erase( myInterface.begin() );
    delete p;
  }
  while (! myProc.empty() ) {
    FProcNode * p = myProc.begin()->second;
    myProc.erase( myProc.begin() );
    delete p;
  }
}

WfNode *
FWorkflow::getNode(const string& nodeId) throw (WfStructException) {
  WfNode * node;
  try {
    node = (WfNode *) getProcNode(nodeId);
  } catch (WfStructException& e) {
    node = (WfNode *) getInterfaceNode(nodeId);
  }
  return node;
}

FProcNode *
FWorkflow::getProcNode(const string& id) throw (WfStructException) {
  FProcNode *node = NULL;
  myLock.lock();    /** LOCK */
  map<string, FProcNode*>::iterator p = this->myProc.find(id);
  if ( p != this->myProc.end())
    node = p->second;
  myLock.unlock();  /** UNLOCK */
  if (node == NULL)
    throw WfStructException(WfStructException::eUNKNOWN_NODE,"FNode id="+id);
  return node;
}

FNode *
FWorkflow::getInterfaceNode(const string& id) throw (WfStructException) {
  FNode *node = NULL;
  myLock.lock();    /** LOCK */
  map<string, FNode*>::iterator p = this->myInterface.find(id);
  if ( p != this->myInterface.end())
    node = p->second;
  myLock.unlock();  /** UNLOCK */
  if (node == NULL)
    throw WfStructException(WfStructException::eUNKNOWN_NODE,"FNode id="+id);
  return node;
}

void
FWorkflow::checkPrec(NodeSet* contextNodeSet) throw (WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING WF PRECEDENCE START" << endl);
  // processor nodes
  for (map<string, FProcNode * >::iterator p = myProc.begin();
       p != myProc.end();
       ++p ) {
    FProcNode * node = (FProcNode*) p->second;
    node->setNodePrecedence(contextNodeSet);
  }
  // interface nodes (does sth only for sink nodes)
  for (map<string, FNode * >::iterator p = myInterface.begin();
       p != myInterface.end();
       ++p ) {
    FNode * node = (FNode *) p->second;
    node->setNodePrecedence(contextNodeSet);
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING WF PRECEDENCE END" << endl);
}

FActivityNode*
FWorkflow::createActivity(const string& id) throw (WfStructException)
{
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating activity node : " << id << endl);
    FActivityNode* node = new FActivityNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FIfNode*
FWorkflow::createIf(const string& id) throw (WfStructException)
{
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating IF node : " << id << endl);
    FIfNode* node = new FIfNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FMergeNode*
FWorkflow::createMerge(const string& id) throw (WfStructException)
{
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating MERGE node : " << id << endl);
    FMergeNode* node = new FMergeNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FFilterNode*
FWorkflow::createFilter(const string& id) throw (WfStructException)
{
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating FILTER node : " << id << endl);
    FFilterNode* node = new FFilterNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FLoopNode*
FWorkflow::createLoop(const string& id) throw (WfStructException)
{
  // check if node does not already exist
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating LOOP node : " << id << endl);
    FLoopNode* node = new FLoopNode(this, id);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FWorkflow*
FWorkflow::createSubWorkflow(const string& id) throw (WfStructException)
{
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating sub-workflow node : " << id << endl);
    FWorkflow* node = new FWorkflow(id, this);
    myProc[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FSourceNode*
FWorkflow::createSource(const string& id, WfCst::WfDataType type)
    throw (WfStructException)
{
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating source node : " << id << endl);
    FSourceNode* node = new FSourceNode(this, id, type);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FConstantNode*
FWorkflow::createConstant(const string& id, WfCst::WfDataType type)
    throw (WfStructException)
{
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating constant node : " << id << endl);
    FConstantNode* node = new FConstantNode(this, id, type);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

FSinkNode*
FWorkflow::createSink(const string& id, WfCst::WfDataType type, unsigned int depth)
    throw (WfStructException)
{
  try {
    getInterfaceNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating sink node : " << id << endl);
    FSinkNode* node = new FSinkNode(this, id, type, depth);
    myInterface[id] = node;
    return node;
  }
  throw WfStructException(WfStructException::eDUPLICATE_NODE,"FNode id="+id);
}

/**
 * Specific DFS search method
 * Skips the back nodes
 * NOT THREAD SAFE
 */

struct DFSNodeInfo {
  bool  explored;
  bool  ongoing;
  short end;
  DFSNodeInfo() {};
  DFSNodeInfo(bool _expl, bool _ongoing, short _end)
  : explored(_expl),ongoing(_ongoing),end(_end) {};
};

void DFS(WfNode* node,
         map<WfNode*, DFSNodeInfo>& DFSInfo,
         short& endCount) {
  DFSInfo[node].ongoing = true;
  TRACE_TEXT (TRACE_ALL_STEPS, "Starting DFS search for node : " << node->getId() << endl);
  for(list<WfNode*>::iterator nextIter = node->nextNodesBegin();
      nextIter != node->nextNodesEnd();
      ++nextIter) {
    WfNode* nextNode = (WfNode*) *nextIter;
    TRACE_TEXT (TRACE_ALL_STEPS, "  Processing next node : " << nextNode->getId() << endl);
    // look for next node DFS information
    map<WfNode*, DFSNodeInfo>::iterator nextNodeInfoSearch = DFSInfo.find(nextNode);
    // if not found skip the node (means next node is not a processor)
    if (nextNodeInfoSearch == DFSInfo.end())
      continue;
    DFSNodeInfo& nextNodeInfo = nextNodeInfoSearch->second;
    if (!nextNodeInfo.explored) {
      if (!nextNodeInfo.ongoing) {
        DFS(nextNode, DFSInfo, endCount);
      } else {
        // this is a BACK edge
        TRACE_TEXT (TRACE_ALL_STEPS, "Found back edge!!" << endl);
      }
    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, "  Next node was already explored" << endl);
    }
  }
  DFSInfo[node].explored = true;
  DFSInfo[node].end      = endCount++;
  TRACE_TEXT (TRACE_ALL_STEPS, "End of DFS search for node : " << node->getId()
            << " (count=" << endCount-1 << ")" << endl);
}

/**
 * FWorkflow XML Parsing
 */
void
FWorkflow::initFromXmlFile(const string& xmlWfFileName,
                           const string& xmlDataFileName) {

  string pfx = "[" + getId() + "] : ";
  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "*** Parsing WORKFLOW XML" << endl);
  FWfParser reader(*this, xmlWfFileName);
  reader.parseXml();

  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "*** Checking WORKFLOW XML structure" << endl);
  checkPrec(this);

  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "*** Initialize functional wf ****" << endl);
  setDataSrcXmlFile(xmlDataFileName);
//   initialize();
}

/**
 * FWorkflow initialization
 * NOT THREAD SAFE
 * FIXME make it private ?
 */
void
FWorkflow::initialize() throw (WfStructException) {
  string pfx = "[" + getId() + "] : ";

  if (getPortNb() > 0) {  // used for sub-workflows
    FProcNode::initialize();
    // All input ports must be interfaced to sources & outputs to sinks
    TRACE_TEXT (TRACE_ALL_STEPS, pfx << "Checking interfaces" << endl);
    for (map<string,WfPort*>::iterator portIter = ports.begin();
      portIter != ports.end();
      ++portIter) {
      FNodePort* wfPort = dynamic_cast<FNodePort*>((WfPort*) portIter->second);
      try {
        FNode* interfNode = getInterfaceNode(wfPort->getInterfaceRef());
        interfNode->connectToWfPort(wfPort);
      } catch (const WfStructException& e) {
        throw WfStructException(e.Type(), string("Unknown reference for sub-workflow interface : ")
                                          + e.Info());
      }
    }
  }

  TRACE_TEXT (TRACE_ALL_STEPS,pfx << "Initializing Interface..." << endl);
  for(map<string,FNode*>::iterator iter = myInterface.begin();
      iter != myInterface.end();
      ++iter) {
    ((FNode*) iter->second)->initialize();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,pfx << "Initializing Processors..." << endl);
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    ((FProcNode*) iter->second)->initialize();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,pfx << "Sorting processors..." << endl);
  // create a topologically sorted list of processor nodes
  map<WfNode*, DFSNodeInfo>* DFSInfo = new map<WfNode*,DFSNodeInfo>();
  short DFSEndCount = 0;
  // -> initialization of DFSInfo
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    WfNode* node = (WfNode*) iter->second;
    (*DFSInfo)[node] = DFSNodeInfo(false,false,0);
  }
  // -> DFS search
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    WfNode* node = (WfNode*) iter->second;
    if (!(*DFSInfo)[node].explored) {
      DFS(node,*DFSInfo, DFSEndCount);
    }
  }
  // -> create sorted todo list
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    FProcNode* node = (FProcNode*) iter->second;
    if (todoProc.size() == 0) {
      todoProc.push_back(node);
    } else { // insertion sort
      short insNodePrio = (*DFSInfo)[(WfNode*)node].end;
      list<FProcNode*>::iterator todoIter = todoProc.begin();
      while ((insNodePrio < (*DFSInfo)[(WfNode*) *todoIter].end)
              && (todoIter != todoProc.end()))
           ++todoIter;
      todoProc.insert(todoIter, node);
    }
  }
}

/**
 * Set the path of the file used for data sources instanciation
 */
void
FWorkflow::setDataSrcXmlFile(const string& dataFileName) {
  this->dataSrcXmlFile = dataFileName;
}

/**
 * Returns the path of the file used for data sources instanciation
 */
const string&
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

  string pfx = "[" + getId() + "] : ";

  TRACE_TEXT (TRACE_MAIN_STEPS, pfx << "########## WORKFLOW '" << getId()
                            << "' INSTANCIATION START ##########" << endl);

  if (getPortNb() > 0) {  // used for sub-workflows
    FProcNode::instanciate(dag); // run the iterator and call createReal/VoidInstance
  }
  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "###### Instanciate sources/constants ..." << endl);
  // initialize the INTERFACE (this will create the input data items if source is XML)
  for(map<string,FNode*>::iterator iter = myInterface.begin();
      iter != myInterface.end();
      ++iter) {
    FNode* interfNode = (FNode*) iter->second;
    interfNode->resumeInstanciation();
    if (interfNode->instanciationReady())
      interfNode->instanciate(NULL);
  }

  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "###### Instanciate processors..." << endl);
  // resume instanciation (if node was put on hold during a previous instanciate call)
  for(list<FProcNode*>::iterator iter = todoProc.begin();
      iter != todoProc.end();
      ++iter) {
     ((FProcNode*) *iter)->resumeInstanciation();
  }
  unsigned int nbTodoProc = todoProc.size();
  // loop until all todo nodes are either fully instanciated or on hold
  while (nbTodoProc) {
    // for each node in the todo list and not on hold
    list<FProcNode*>::iterator procIter = todoProc.begin();
    while (procIter != todoProc.end()) {
      FProcNode* currProc = (FProcNode*) *procIter;
      // instantiate node
      try {
        currProc->instanciate(dag);
      } catch (WfDataHandleException& e) {
        WARNING("Failure during node " << currProc->getId() << " instanciation:"
                << endl << e.ErrorMsg() << endl);
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
        TRACE_TEXT (TRACE_MAIN_STEPS,"#### Processor " << currProc->getId()
                      << " instanciation COMPLETE" << endl);
        procIter = todoProc.erase(procIter);
        nbTodoProc--;
        continue;
      }
      // go to next todoProc
      nbTodoProc--;
      ++procIter;
    } // end loop todo list
  } // end while (nbTodoProc)

  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "###### Instanciate sinks ..." << endl);
  for(map<string,FNode*>::iterator iter = myInterface.begin();
      iter != myInterface.end();
      ++iter) {
    FSinkNode* sinkNode = dynamic_cast<FSinkNode*>((FNode*) iter->second);
    if (sinkNode) {
      sinkNode->instanciate(NULL);
    }
  }

  if (instanciationOnHold()) {
    TRACE_TEXT (TRACE_MAIN_STEPS, pfx << "########## WORKFLOW INSTANCIATION ON HOLD ##########" << endl);
  }
  // check if instanciation is pending on node execution
  // the first condition happens when one of the nodes (a sub-wf) is pending
  else if (instanciationPending() || pendingNodes.size() > 0) {
    TRACE_TEXT (TRACE_MAIN_STEPS, pfx << "########## WORKFLOW INSTANCIATION PENDING ##########" << endl);
    myStatus = N_INSTANC_PENDING;
  }
  // check if instanciation is finished
  else if (todoProc.size() == 0) {
    TRACE_TEXT (TRACE_MAIN_STEPS, pfx << "############ WORKFLOW INSTANCIATION END ############" << endl);
    myStatus = N_INSTANC_END;
  }
  else if (getPortNb() > 0) {  // used for sub-workflows
    TRACE_TEXT (TRACE_MAIN_STEPS, pfx << "############ WORKFLOW INSTANCIATION NOT FINISHED ############" << endl);
  }
  // incorrect status
  else {
    WARNING(pfx << "Instanciation stopped in incorrect state!!");
    myStatus = N_INSTANC_STOPPED;
    for (list<FProcNode*>::iterator procIter = todoProc.begin();
         procIter != todoProc.end();
         ++procIter) {
      WARNING(pfx << "Functional node remaining in TODO list: "
                  << ((FProcNode*) *procIter)->getId() << endl);
    }
  }
  // add dag to my list of dags
  myDags.push_back(dag);
}

void
FWorkflow::createRealInstance(Dag* dag,
                              const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
  // LOOP for each source
  for (map<string, FNode *>::iterator interfIter = myInterface.begin();
       interfIter != myInterface.end();
       ++interfIter) {
    FSourceNode * source = dynamic_cast<FSourceNode*>((FNode*) interfIter->second);
    if (source) {
      source->instanciate(currTag, currDataLine); // get DH from DataLine and send it
    }
  } // end for interface nodes
}

void
FWorkflow::createVoidInstance(const FDataTag& currTag,
                              const vector<FDataHandle*>& currDataLine) {
  createRealInstance(NULL,currTag,currDataLine);
}

void
FWorkflow::updateInstanciationStatus() {
  string pfx = "[" + getId() + "] : ";
  if (myRootIterator->isAtEnd()) {
    if (myRootIterator->isDone()) {
      TRACE_TEXT (TRACE_ALL_STEPS, pfx << "########## ALL INPUTS PROCESSED" << endl);
//      myStatus = N_INSTANC_END; (NOT APPLICABLE FOR SUB-WORKFLOWS)

      // CHECK for each out port if the output is empty  TODO check if could be optimized
      for (map<string,WfPort*>::iterator portIter = ports.begin();
           portIter != ports.end();
           ++portIter) {
        WfPort* port = (WfPort*) portIter->second;
        if (port->isOutput()) {
          FNodeOutPort* outPort = dynamic_cast<FNodeOutPort*>(port);
          outPort->checkIfEmptyOutput();
        }
      }

    } else {
      TRACE_TEXT (TRACE_ALL_STEPS, pfx << "########## WAITING FOR INPUTS " << endl);
      myStatus = N_INSTANC_READY;
    }
  }
}

void
FWorkflow::displayAllResults(ostream& output) {
  // display cancelled dags
  for (list<Dag*>::const_iterator dagIter = myDags.begin();
       dagIter != myDags.end();
       ++dagIter) {
    Dag * currDag = (Dag*) *dagIter;
    if (currDag->isCancelled()) {
       output  << "** DAG " << currDag->getId()
               << " was cancelled => no results **" << endl;
    }
  }
  // display all sink results
  for (map<string,FNode*>::const_iterator nodeIter = myInterface.begin();
       nodeIter != myInterface.end();
       ++nodeIter) {
    FSinkNode *sink = dynamic_cast<FSinkNode*>((FNode*) nodeIter->second);
    if (sink != NULL) {
      sink->displayResults(output);
    }
  }
}

void
FWorkflow::displayDagSummary(ostream& output) {
  output << "** INSTANCIATED DAGS SUMMARY:" << endl;
  for (list<Dag*>::iterator dagIter = myDags.begin();
       dagIter != myDags.end();
       ++dagIter) {
    Dag * dag = (Dag*) *dagIter;
    output << "DAG ID '" << dag->getId() << "': "
           << "size = " << dag->size() << " node(s), "
           << "makespan = " << dag->getMakespan() << " ms" << endl;
  }
}

void
FWorkflow::setPendingInstanceInfo(DagNode * dagNode,
                                  FDataHandle * dataHdl,
                                  FNodeOutPort * outPort,
                                  FNodeInPort * inPort) {
  pendingDagNodeInfo_t info;
  info.dataHdl = dataHdl;
  info.outPort = outPort;
  info.inPort  = inPort;
  pendingNodes.insert(make_pair(dagNode,info));
  string pfx = "[" + getId() + "] : ";
  TRACE_TEXT (TRACE_ALL_STEPS, pfx << "Added one entry (tag=" << dataHdl->getTag().toString()
              << ") in pending list (size = " << pendingNodes.size() << ")" << endl);
}

/**
 * Handle end of node execution
 */
void
FWorkflow::handlerDagNodeDone(DagNode* dagNode) {
  bool statusChange = false;
  string pfx = "[" + getId() + "] : ";
  myLock.lock();    /** LOCK */
  // search the instance in the pending list
  multimap<DagNode*, pendingDagNodeInfo_t>::iterator pendingIter, pendingStart;
  pendingStart = pendingIter = pendingNodes.lower_bound(dagNode);
  // if found, resubmit the datahandle to the in port(s)
  while (pendingIter != pendingNodes.upper_bound(dagNode)) {
    pendingDagNodeInfo_t info = (pendingDagNodeInfo_t) (pendingIter++)->second;
    TRACE_TEXT (TRACE_ALL_STEPS, pfx << " ## RETRY DATA SUBMISSION FOR INSTANCE : "
                                << dagNode->getId() << endl);

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

    TRACE_TEXT (TRACE_ALL_STEPS, pfx << " ## END OF RETRY FOR INSTANCE : "
                                << dagNode->getId() << endl);
  }
  // remove entries from the pending list
  pendingNodes.erase(pendingStart,pendingIter);
  // re-initialize instanciation
  if (statusChange)
    resumeInstanciation();

  myLock.unlock();  /** UNLOCK */
}

/**
 * Memory free
 */

void
FWorkflow::deleteAllResults() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Deleting all wf results ..." <<  endl);
  for (list<Dag*>::iterator dagIter = myDags.begin();
       dagIter != myDags.end();
       ++dagIter) {
    ((Dag*) *dagIter)->deleteAllResults();
  }
}

void
FWorkflow::deleteAllDags() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Deleting all wf dags ..." <<  endl);
  while (! myDags.empty() ) {
    Dag * p = myDags.front();
    myDags.pop_front();
    delete p;
  }
}
