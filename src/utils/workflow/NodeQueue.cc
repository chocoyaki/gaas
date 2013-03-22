/**
* @file  NodeQueue.cc
* 
* @brief  The base class for workflow node queueing container
* 
* @author  Benjamin ISNARD (Benjamin.Isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/****************************************************************************/
/* The base class for workflow node queueing container                      */
/* and the derived class that handles priority of nodes                     */
/****************************************************************************/


#include "NodeQueue.hh"
#include "DagNode.hh"
#include "debug.hh"

/****************************************************************************/
/*                    NodeQueue - PUBLIC METHODS                            */
/****************************************************************************/

NodeQueue::NodeQueue() : nodeCounter(0) {
}

NodeQueue::NodeQueue(std::string name) : nodeCounter(0), myName(name) {
}

NodeQueue::~NodeQueue() {
  // Important: the queue must be empty before being destroyed
}

std::string
NodeQueue::getName() {
  return myName;
}

void
NodeQueue::pushNode(DagNode * node) {
  // nodes must be able to notify the queue it belongs to
  node->setNodeQueue(this);
  this->nodeCounter++;
}

void
NodeQueue::pushNodes(std::vector<DagNode *> nodes) {
  std::vector<DagNode *>::iterator iter = nodes.begin();
  while (iter != nodes.end()) {
    this->pushNode(*iter);
    ++iter;
  }
}

void
NodeQueue::removeNode(DagNode * node) {
  node->setNodeQueue(NULL);
  this->nodeCounter--;
}

int
NodeQueue::size() {
  return this->nodeCounter;
}

bool
NodeQueue::isEmpty() { return (this->nodeCounter == 0); }

/****************************************************************************/
/*                    ChainedNodeQueue - PUBLIC METHODS                     */
/****************************************************************************/

ChainedNodeQueue::ChainedNodeQueue(NodeQueue * outputQ)
  : outputQ(outputQ) {
}

ChainedNodeQueue::ChainedNodeQueue(std::string name, NodeQueue * outputQ)
  : NodeQueue(name), outputQ(outputQ) {
}

ChainedNodeQueue::~ChainedNodeQueue() {
}

bool
ChainedNodeQueue::notifyStateChange(DagNode * node) {
  if (this->outputQ != NULL) {
    this->outputQ->pushNode(node);
    this->nodeCounter--;
    return true;
  } else {
    return false;
  }
}


/****************************************************************************/
/*                OrderedNodeQueue - PUBLIC METHODS                         */
/****************************************************************************/

OrderedNodeQueue::OrderedNodeQueue() { }
OrderedNodeQueue::OrderedNodeQueue(std::string name) : NodeQueue(name) { }
OrderedNodeQueue::~OrderedNodeQueue() { }

// The basic version of this method inserts a ready node at the end
// of the ready nodes list without doing any re-ordering
void
OrderedNodeQueue::pushNode(DagNode * node) {
  orderedNodes.push_back(node);
  node->setNodeQueue(this);
  this->nodeCounter++;
}

DagNode *
OrderedNodeQueue::popFirstNode() {
  if (!orderedNodes.empty()) {
    DagNode * nodePtr = orderedNodes.front();  // get the ref to the first node
    orderedNodes.pop_front();  // removes the node from the queue
    nodePtr->setNodeQueue(NULL);
    this->nodeCounter--;
    return nodePtr;
  } else {
    return NULL;
  }
}

void
OrderedNodeQueue::removeNode(DagNode * node) {
  std::list<DagNode*>::iterator np = orderedNodes.begin();
  while (np != orderedNodes.end()) {
    if (*np == node) {
      np = orderedNodes.erase(np);
      node->setNodeQueue(NULL);
      this->nodeCounter--;
    } else {
      ++np;
    }
  }
}


void
OrderedNodeQueue::removeLastNodes(int nbNodesToKeep) {
  std::list<DagNode*>::iterator  nodeIter = this->begin();
  // Go to the first item to delete
  int ix = 0;
  while ((nodeIter != this->end()) && (ix++ < nbNodesToKeep)) {
    ++nodeIter;
  }
  // Delete remaining items
  if (nodeIter != this->end()) {
    orderedNodes.erase(nodeIter, this->end());
    this->nodeCounter = nbNodesToKeep;
  }
}


bool
OrderedNodeQueue::notifyStateChange(DagNode * node) {
  // do nothing
  return true;
}

std::list<DagNode *>::iterator
OrderedNodeQueue::begin() {
  return orderedNodes.begin();
}

std::list<DagNode *>::iterator
OrderedNodeQueue::end() {
  return orderedNodes.end();
}

/****************************************************************************/
/*                PriorityNodeQueue - PUBLIC METHODS                        */
/****************************************************************************/

PriorityNodeQueue::PriorityNodeQueue() {
}

PriorityNodeQueue::PriorityNodeQueue(std::string name)
  : OrderedNodeQueue(name) {
}

PriorityNodeQueue::~PriorityNodeQueue() {
}

// The version that implements ordering based on node priority
// It orders the ready nodes in DECREASING priority
void
PriorityNodeQueue::pushNode(DagNode * insNode) {
  double insNodePrio = insNode->getPriority();
  std::list<DagNode*>::iterator  nodeIter = orderedNodes.begin();
  DagNode *curNode   = NULL;
  while ((nodeIter != orderedNodes.end())
         && (curNode = (DagNode *) *nodeIter)
         && (curNode->getPriority() >= insNodePrio)) {
    ++nodeIter;
  }
  orderedNodes.insert(nodeIter, insNode);
  insNode->setNodeQueue(this);
  this->nodeCounter++;
}
