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
  : id(id), dagCounter(0) {
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
FWorkflow::getNode(const string& nodeId) {
  Node * node = (Node *) getProcNode(nodeId);
  if (node == NULL) {
    node = (Node *) getInterfaceNode(nodeId);
    if (node == NULL)
      return NULL;
  }
  return node;
}

FProcNode *
FWorkflow::getProcNode(const string& id) {
  map<string, FProcNode*>::iterator p = this->myProc.find(id);
  if ( p != this->myProc.end())
    return p->second;
  else
    return NULL;
}

FNode *
FWorkflow::getInterfaceNode(const string& id) {
  map<string, FNode*>::iterator p = this->myInterface.find(id);
  if ( p != this->myInterface.end())
    return p->second;
  else
    return NULL;
}

void
FWorkflow::checkPrec(NodeSet* contextNodeSet) throw (WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING WF PRECEDENCE START" << endl);
  // initializes the next/prev tables for each processor node
  for (map<string, FProcNode * >::iterator p = myProc.begin();
       p != myProc.end();
       ++p ) {
    FProcNode * node = (FProcNode*) p->second;
    if (!node->setNodePrecedence(contextNodeSet))
      throw WfStructException(WfStructException::eUNKNOWN_NODE,
                              "Cannot set predecessor relationship for node "
                                  + node->getId());
  }
  TRACE_TEXT(TRACE_ALL_STEPS, "CHECKING WF PRECEDENCE END" << endl);
}

// unsigned int
// FWorkflow::size() {
//   cout << "FWorkflow::size NOT IMPLEMENTED";
//   exit(1);
// }

FProcNode*
FWorkflow::createProcessor(const string& id, short maxInstances) {
  if (getProcNode(id) != NULL)
    return NULL;
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating processor node : " << id << endl);
  FProcNode* node = new FProcNode(this, id, maxInstances);
  myProc[id] = node;
  return node;
}

FSourceNode*
FWorkflow::createSource(const string& id, WfCst::WfDataType type, short maxInstances) {
  if (getInterfaceNode(id) != NULL)
    return NULL;
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating source node : " << id << endl);
  FSourceNode* node = new FSourceNode(this, id, type, maxInstances);
  myInterface[id] = node;
  return node;
}

FConstantNode*
FWorkflow::createConstant(const string& id, WfCst::WfDataType type) {
  if (getInterfaceNode(id) != NULL)
    return NULL;
  TRACE_TEXT (TRACE_ALL_STEPS,"Creating constant node : " << id << endl);
  FConstantNode* node = new FConstantNode(this, id, type);
  myInterface[id] = node;
  return node;
}

/**
 * Specific DFS search method
 * Skips the back nodes
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
  cout << "Starting DFS search for node : " << node->getId() << endl;
  for(list<Node*>::iterator nextIter = node->nextNodesBegin();
      nextIter != node->nextNodesEnd();
      ++nextIter) {
    Node* nextNode = (Node*) *nextIter;
    cout << "  Processing next node : " << nextNode->getId() << endl;
    // look for next node DFS information
    map<Node*, DFSNodeInfo>::iterator nextNodeInfoSearch = DFSInfo.find(nextNode);
    // if not found skip the node (means next node is not a processor)
    if (nextNodeInfoSearch == DFSInfo.end())
      return;
    DFSNodeInfo& nextNodeInfo = nextNodeInfoSearch->second;
    if (!nextNodeInfo.explored) {
      if (!nextNodeInfo.ongoing) {
        DFS(nextNode, DFSInfo, endCount);
      } else {
        // this is a BACK edge
        cout << "Found back edge!!" << endl;
      }
    } else {
      cout << "  Next node was already explored" << endl;
    }
  }
  DFSInfo[node].explored = true;
  DFSInfo[node].end      = endCount++;
  cout << "End of DFS search for node : " << node->getId()
            << " (count=" << endCount-1 << ")" << endl;
}

/**
 * FWorkflow initialization
 */
void
FWorkflow::initialize() {
  TRACE_TEXT (TRACE_ALL_STEPS,"Connecting Fnodes..." << endl);
  // initialize the INTERNAL nodes first (to create the buffers)
  for(map<string,FProcNode*>::iterator iter = myProc.begin();
      iter != myProc.end();
      ++iter) {
    ((FProcNode*) iter->second)->connectNodePorts();
  }
  TRACE_TEXT (TRACE_ALL_STEPS,"Sorting Fnodes..." << endl);
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
 * FWorkflow instanciation
 */

Dag *
FWorkflow::instanciateDag() {
  Dag * currDag = new Dag();
  currDag->setId("WF_" + this->id + "_DAG_" + itoa(dagCounter++));
  TRACE_TEXT (TRACE_ALL_STEPS,"@@@@@ DAG " << currDag->getId()
               << " GENERATION START @@@@@" << endl);
  TRACE_TEXT (TRACE_ALL_STEPS,"Init workflow interface..." << endl);
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
//       if (!currProc->isOnHold()) {
        // instantiate node
        currProc->instanciate(currDag);
        // if fully instanciated, remove from todo list
        if (currProc->isFullyInstantiated()) {
          TRACE_TEXT (TRACE_ALL_STEPS,"#### Processor " << currProc->getId()
                      << " instanciation COMPLETE" << endl);
          procIter = todoProc.erase(procIter);
          nbTodoProc--;
        } else {
//           if (currProc->isOnHold()) {
            nbTodoProc--;
//           }
          ++procIter;
        }
//       } // end if !isOnHold
    } // end loop todo list
  } // end while (nbTodoProc)
  // add dag to my list of dags
  myDags.push_back(currDag);
  return currDag;
}

list<Dag*>&
FWorkflow::getDagList() {
  return myDags;
}

bool
FWorkflow::instanciationCompleted() {
  return (todoProc.size() == 0);
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
