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
 * Revision 1.11  2011/01/21 16:42:24  bdepardo
 * Prefer prefix ++/-- operators for non-primitive types.
 *
 * Revision 1.10  2010/03/08 13:50:19  bisnard
 * debug too verbose
 *
 * Revision 1.9  2008/11/08 19:12:40  bdepardo
 * A few warnings removal
 *
 * Revision 1.8  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.7  2008/09/04 14:34:36  bisnard
 * new method removeLastNodes
 *
 * Revision 1.6  2008/07/08 11:15:58  bisnard
 * Correct dag/node destruction with nodequeues
 *
 * Revision 1.5  2008/06/25 09:56:37  bisnard
 * new method size()
 *
 * Revision 1.4  2008/06/18 14:58:05  bisnard
 * Bug in priority queue insertion
 *
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
#include "DagNode.hh"
#include "debug.hh"

using namespace std;

/****************************************************************************/
/*                    NodeQueue - PUBLIC METHODS                            */
/****************************************************************************/

NodeQueue::NodeQueue() : nodeCounter(0) { }
NodeQueue::NodeQueue(string name) : nodeCounter(0), myName(name) { }
NodeQueue::~NodeQueue() {
  // Important: the queue must be empty before being destroyed
}

string
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
NodeQueue::pushNodes(vector<DagNode *> nodes) {
  vector<DagNode *>::iterator iter = nodes.begin();
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
  : outputQ(outputQ) { }
ChainedNodeQueue::ChainedNodeQueue(string name, NodeQueue * outputQ)
  : NodeQueue(name), outputQ(outputQ) { }
ChainedNodeQueue::~ChainedNodeQueue() { }

bool
ChainedNodeQueue::notifyStateChange(DagNode * node) {
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
OrderedNodeQueue::pushNode(DagNode * node) {
  //   TRACE_TEXT (TRACE_ALL_STEPS,
  //       "Node " << node->getCompleteId() << " inserted at end of queue" << endl);
  orderedNodes.push_back(node);
  node->setNodeQueue(this);
  this->nodeCounter++;
}

// DagNode *
// OrderedNodeQueue::getFirstNode() {
//   if (!orderedNodes.empty())
//     return orderedNodes.front();
//   else
//     return NULL;
// }

DagNode *
OrderedNodeQueue::popFirstNode() {
  if (!orderedNodes.empty()) {
    DagNode * nodePtr = orderedNodes.front();  // get the ref to the first node
    orderedNodes.pop_front(); // removes the node from the queue
    nodePtr->setNodeQueue(NULL);
    this->nodeCounter--;
    return nodePtr;
  }
  else {
    return NULL;
  }
}

void
OrderedNodeQueue::removeNode(DagNode * node) {
  list<DagNode*>::iterator np = orderedNodes.begin();
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
  list<DagNode*>::iterator  nodeIter = this->begin();
  // Go to the first item to delete
  int ix=0;
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

PriorityNodeQueue::PriorityNodeQueue() { }
PriorityNodeQueue::PriorityNodeQueue(string name) : OrderedNodeQueue(name) { }
PriorityNodeQueue::~PriorityNodeQueue() { }

// The version that implements ordering based on node priority
// It orders the ready nodes in DECREASING priority
void
PriorityNodeQueue::pushNode(DagNode * insNode) {
  double insNodePrio = insNode->getPriority();
  list<DagNode*>::iterator  nodeIter = orderedNodes.begin();
  DagNode *                      curNode   = NULL;
  while ((nodeIter != orderedNodes.end())
         && (curNode = (DagNode *) *nodeIter)
         && (curNode->getPriority() >= insNodePrio)) {
    ++nodeIter;
  }
  orderedNodes.insert(nodeIter, insNode);
  insNode->setNodeQueue(this);
  this->nodeCounter++;

  //   if (nodeIter != orderedNodes.end()) {
  //     TRACE_TEXT (TRACE_ALL_STEPS,
  //       "Node " << insNode->getCompleteId() << " inserted before "
  //           << curNode->getCompleteId() << " in queue" << endl);
  //   } else if (curNode != NULL) {
  //     TRACE_TEXT (TRACE_ALL_STEPS,
  //       "Node " << insNode->getCompleteId() << " inserted last in queue" << endl);
  //   } else {
  //     TRACE_TEXT (TRACE_ALL_STEPS,
  //       "Node " << insNode->getCompleteId() << " inserted first in queue" << endl);
  //   }
}
