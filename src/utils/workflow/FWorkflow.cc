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
#include "FWorkflow.hh"

FWorkflow::FWorkflow(const string& id)
  : id(id), dagCounter(0), myStatus(INSTANC_READY) {
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

const string&
FWorkflow::getId() {
  return id;
}

Node *
FWorkflow::getNode(const string& nodeId) throw (WfStructException) {
  Node * node;
  try {
    node = (Node *) getProcNode(nodeId);
  } catch (WfStructException& e) {
    node = (Node *) getInterfaceNode(nodeId);
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

FProcNode*
FWorkflow::createProcessor(const string& id) throw (WfStructException)
{
  try {
    getProcNode(id);
  } catch (WfStructException& e) {
    TRACE_TEXT (TRACE_ALL_STEPS,"Creating processor node : " << id << endl);
    FProcNode* node = new FProcNode(this, id);
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

void DFS(Node* node,
         map<Node*, DFSNodeInfo>& DFSInfo,
         short& endCount) {
  DFSInfo[node].ongoing = true;
  TRACE_TEXT (TRACE_ALL_STEPS, "Starting DFS search for node : " << node->getId() << endl);
  for(list<Node*>::iterator nextIter = node->nextNodesBegin();
      nextIter != node->nextNodesEnd();
      ++nextIter) {
    Node* nextNode = (Node*) *nextIter;
    TRACE_TEXT (TRACE_ALL_STEPS, "  Processing next node : " << nextNode->getId() << endl);
    // look for next node DFS information
    map<Node*, DFSNodeInfo>::iterator nextNodeInfoSearch = DFSInfo.find(nextNode);
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
 * FWorkflow initialization
 * NOT THREAD SAFE
 */
void
FWorkflow::initialize(const string& dataFileName) throw (WfStructException) {
  this->dataSrcXmlFile = dataFileName;
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing Interface..." << endl);
  for(map<string,FNode*>::iterator iter = myInterface.begin();
      iter != myInterface.end();
      ++iter) {
    ((FNode*) iter->second)->initialize();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,"Initializing Processors..." << endl);
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    ((FProcNode*) iter->second)->initialize();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,"Sorting processors..." << endl);
  // create a topologically sorted list of processor nodes
  map<Node*, DFSNodeInfo>* DFSInfo = new map<Node*,DFSNodeInfo>();
  short DFSEndCount = 0;
  // -> initialization of DFSInfo
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    Node* node = (Node*) iter->second;
    (*DFSInfo)[node] = DFSNodeInfo(false,false,0);
  }
  // -> DFS search
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    Node* node = (Node*) iter->second;
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
      short insNodePrio = (*DFSInfo)[(Node*)node].end;
      list<FProcNode*>::iterator todoIter = todoProc.begin();
      while ((insNodePrio < (*DFSInfo)[(Node*) *todoIter].end)
              && (todoIter != todoProc.end()))
           ++todoIter;
      todoProc.insert(todoIter, node);
    }
  }
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

Dag *
FWorkflow::instanciateDag() {
  Dag * currDag = new Dag();
  // set a temporary ID for the DAG
  // this ID will be changed when the DAG is submitted to the MaDag
  // the temp ID must not match any real ID (which are numbers)
  currDag->setId("WF_" + this->id + "_DAG_" + itoa(dagCounter++));
  currDag->setWorkflow(this);
  TRACE_TEXT (TRACE_MAIN_STEPS,"@@@@@ DAG " << currDag->getId()
               << " GENERATION START @@@@@" << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"Instanciate interface..." << endl);
  // initialize the INTERFACE (this will create the input data items)
  for(map<string,FNode*>::iterator iter = myInterface.begin();
      iter != myInterface.end();
      ++iter) {
    ((FNode*) iter->second)->resumeInstanciation();
    ((FNode*) iter->second)->instanciate(NULL);
  }
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
      currProc->instanciate(currDag);
      // if fully instanciated, remove from todo list
      if (currProc->instanciationCompleted()) {
        TRACE_TEXT (TRACE_MAIN_STEPS,"#### Processor " << currProc->getId()
                      << " instanciation COMPLETE" << endl);
        procIter = todoProc.erase(procIter);
        nbTodoProc--;
      } else {
        nbTodoProc--;
        if (currProc->instanciationPending()) {
          TRACE_TEXT (TRACE_MAIN_STEPS,"#### Processor " << currProc->getId()
                      << " instanciation PENDING" << endl);
          myStatus = INSTANC_PENDING;
        }
        ++procIter;
      }
    } // end loop todo list
  } // end while (nbTodoProc)
  // check dag emptyness
  if (currDag->size() == 0) {
    WARNING("Instanciation of EMPTY DAG!!! Problem during instanciation.");
    myStatus = INSTANC_END;
    delete currDag;
    return NULL;
  }
  // check if instanciation is finished
  if (todoProc.size() == 0) {
    TRACE_TEXT (TRACE_MAIN_STEPS,"############ WORKFLOW INSTANCIATION END ############" << endl);
    myStatus = INSTANC_END;
  }
  // add dag to my list of dags
  myDags.push_back(currDag);
  return currDag;
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

bool
FWorkflow::instanciationReady() {
  return myStatus == INSTANC_READY;
}

bool
FWorkflow::instanciationPending() {
  return myStatus == INSTANC_PENDING;
}

bool
FWorkflow::instanciationCompleted() {
  return myStatus == INSTANC_END;
}

void
FWorkflow::stopInstanciation() {
  myStatus = INSTANC_STOPPED;
}

bool
FWorkflow::instanciationStopped() {
  return myStatus == INSTANC_STOPPED;
}

/**
 * Handle end of node execution
 */
void
FWorkflow::handlerDagNodeDone(DagNode* dagNode) throw (WfDataException) {
  FProcNode* fNode = dagNode->getFNode();
  if (fNode == NULL) {
    INTERNAL_ERROR("handlerDagNodeDone: Missing FNode ref in DagNode",1);
  }
  // call the FProcNode to process pending data handles
  bool statusChange = false;
  myLock.lock();    /** LOCK */
  try {
    fNode->instanceIsDone(dagNode, statusChange);
  } catch (WfDataHandleException& e) {
    WARNING("Data handle error :" << e.ErrorMsg());
    myStatus = INSTANC_STOPPED;
  } catch (WfDataException& e) {
    WARNING("Data error :" << e.ErrorMsg());
    myStatus = INSTANC_STOPPED;
  }
  // re-initialize instanciation
  if (statusChange && !instanciationStopped()) {
    myStatus = INSTANC_READY;
  }
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
