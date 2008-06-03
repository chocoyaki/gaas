/****************************************************************************/
/* The base class for workflow node queueing container                      */
/* and the derived class that handles priority of nodes                     */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.3  2008/06/03 12:18:12  bisnard
 * Get iterator on ordered queue
 *
 * Revision 1.2  2008/04/28 12:15:00  bisnard
 * new NodeQueue implementation for FOFT
 * nodes sorting done by Dag instead of scheduler
 * method to propagate delay at execution (FOFT)
 *
 * Revision 1.1  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 ****************************************************************************/

#include "NodeQueue.hh"
#include "debug.hh"

/****************************************************************************/
/*                    NodeQueue - PUBLIC METHODS                            */
/****************************************************************************/

NodeQueue::NodeQueue() : nodeCounter(0) { }
NodeQueue::NodeQueue(string name) : nodeCounter(0), myName(name) { }
NodeQueue::~NodeQueue() { }

string
NodeQueue::getName() {
  return myName;
}

void
NodeQueue::pushNode(Node * node) {
  // nodes must be able to notify the queue it belongs to
  node->setNodeQueue(this);
  this->nodeCounter++;
}

void
NodeQueue::pushNodes(std::vector<Node *> nodes) {
  std::vector<Node *>::iterator iter = nodes.begin();
  while (iter != nodes.end()) {
    this->pushNode(*iter);
    iter++;
  }
}

bool
NodeQueue::isEmpty() { return (this->nodeCounter == 0); }

/****************************************************************************/
/*                    ChainedNodeQueue - PUBLIC METHODS                     */
/****************************************************************************/

ChainedNodeQueue::ChainedNodeQueue(NodeQueue * outputQ)
  : outputQ(outputQ) { }
ChainedNodeQueue::ChainedNodeQueue(string name, NodeQueue * outputQ)
  : NodeQueue(name), outputQ(outputQ) { }
ChainedNodeQueue::~ChainedNodeQueue() { }

bool
ChainedNodeQueue::notifyStateChange(Node * node) {
  if (this->outputQ != NULL) {
    this->outputQ->pushNode(node);
    this->nodeCounter--;
    return true;
  } else return false;
}


/****************************************************************************/
/*                OrderedNodeQueue - PUBLIC METHODS                         */
/****************************************************************************/

OrderedNodeQueue::OrderedNodeQueue() { }
OrderedNodeQueue::OrderedNodeQueue(string name) : NodeQueue(name) { }
OrderedNodeQueue::~OrderedNodeQueue() { }

// The basic version of this method inserts a ready node at the end
// of the ready nodes list without doing any re-ordering
void
OrderedNodeQueue::pushNode(Node * node) {
  TRACE_TEXT (TRACE_ALL_STEPS,
      "Node " << node->getCompleteId() << " inserted at end of queue" << endl);
  orderedNodes.push_back(node);
  node->setNodeQueue(this);
  this->nodeCounter++;
}

// Node *
// OrderedNodeQueue::getFirstNode() {
//   if (!orderedNodes.empty())
//     return orderedNodes.front();
//   else
//     return NULL;
// }

Node *
OrderedNodeQueue::popFirstNode() {
  if (!orderedNodes.empty()) {
    Node * nodePtr = orderedNodes.front();  // get the ref to the first node
    orderedNodes.pop_front(); // removes the node from the queue
    this->nodeCounter--;
    return nodePtr;
  }
  else {
    return NULL;
  }
}

bool
OrderedNodeQueue::notifyStateChange(Node * node) {
  // do nothing
  return true;
}

std::list<Node *>::iterator
OrderedNodeQueue::begin() {
  return orderedNodes.begin();
}

std::list<Node *>::iterator
OrderedNodeQueue::end() {
  return orderedNodes.end();
}

/****************************************************************************/
/*                PriorityNodeQueue - PUBLIC METHODS                        */
/****************************************************************************/

PriorityNodeQueue::PriorityNodeQueue() { }
PriorityNodeQueue::PriorityNodeQueue(string name) : OrderedNodeQueue(name) { }
PriorityNodeQueue::~PriorityNodeQueue() { }

// The version that implements ordering based on node priority
// It orders the ready nodes in DECREASING priority
void
PriorityNodeQueue::pushNode(Node * insNode) {
  double insNodePrio = insNode->getPriority();
  std::list<Node*>::iterator  nodeIter = orderedNodes.begin();
  Node *                      curNode   = NULL;
  while ((nodeIter != orderedNodes.end())
          && (curNode != NULL)
          && (curNode->getPriority() >= insNodePrio)) {
    curNode = (Node *) *nodeIter;
    nodeIter++;
  }
  orderedNodes.insert(nodeIter, insNode);
  insNode->setNodeQueue(this);
  this->nodeCounter++;

  if (curNode != NULL) {
    TRACE_TEXT (TRACE_ALL_STEPS,
      "Node " << insNode->getCompleteId() << " inserted before "
          << curNode->getCompleteId() << " in queue" << endl);
  } else {
    TRACE_TEXT (TRACE_ALL_STEPS,
      "Node " << insNode->getCompleteId() << " inserted first in queue" << endl);
  }
}
