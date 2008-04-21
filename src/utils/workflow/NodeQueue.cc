/****************************************************************************/
/* The base abstract class for workflow node queueing container             */
/* and the derived class specific to nodes of a dag                         */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.1  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 ****************************************************************************/

#include "NodeQueue.hh"
#include "debug.hh"

/****************************************************************************/
/*                         PUBLIC METHODS                                   */
/****************************************************************************/

NodeQueue::NodeQueue(int wfReqId) : myReqId(wfReqId) {
}

NodeQueue::~NodeQueue() {
}

void
NodeQueue::pushNode(Node * node) {
  node->setWfReqId(this->myReqId);  // set the wfReqId for the node
  node->setNodeQueue(this); // nodes must notify queue when becoming ready
//  qNodes.push_back(node); // add the node to the queue of non-ready nodes
}

void
NodeQueue::pushNodes(std::vector<Node *> nodes) {
  std::vector<Node *>::iterator iter = nodes.begin();
  while (iter != nodes.end()) {
    this->pushNode(*iter);
    iter++;
  }
}

void
NodeQueue::notifyStateChange(Node * node) {
  if (node->isReady()) {
    this->insertReadyNode(node);
  }
}

Node *
NodeQueue::popFirstReadyNode() {
  if (!rNodes.empty()) {
    Node * nodePtr = rNodes.front();  // get the ref to the first ready node
    rNodes.pop_front(); // removes the node from the queue
    return nodePtr;
  }
  else {
    return NULL;
  }
}

int
NodeQueue::getWfReqId() { return this->myReqId; }

/****************************************************************************/
/*                         PROTECTED METHODS                                */
/****************************************************************************/

// The basic version of this method inserts a ready node at the end
// of the ready nodes list without doing any re-ordering
void
NodeQueue::insertReadyNode(Node * node) {
  TRACE_TEXT (TRACE_ALL_STEPS,
      "Node " << node->getCompleteId() << " inserted in ready queue" << endl);
  rNodes.push_back(node);
}
