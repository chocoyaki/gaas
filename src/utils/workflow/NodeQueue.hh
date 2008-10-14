/****************************************************************************/
/* The base class for workflow node queueing container                      */
/* and the derived classes that handles priority of nodes                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.7  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.6  2008/09/04 14:34:36  bisnard
 * new method removeLastNodes
 *
 * Revision 1.5  2008/07/08 11:15:58  bisnard
 * Correct dag/node destruction with nodequeues
 *
 * Revision 1.4  2008/06/25 09:56:37  bisnard
 * new method size()
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

#ifndef _NODEQUEUE_HH_
#define _NODEQUEUE_HH_

#include <string>
#include <list>
#include <vector>

using namespace std;

/**
 * Class NodeQueue (ABSTRACT)
 *
 * This class manages a list of workflow nodes in order to provide an order
 * in the execution of nodes.
 * It is used by the MultiWfScheduler class to handle a set of nodes (that can
 * be the nodes of a dag but not necessarily).
 * After a node has been inserted in the queue (with pushNode) it must call the
 * notifyStateChange method to be moved to the ready nodes queue where it will
 * be selected later on a popFirstReadyNode() call.
 *
 * This base class does not implement any ordering method ie the order of nodes
 * in the ready nodes queue is determined by the order of the calls to
 * notifyStateChange.
 */

class DagNode;

class NodeQueue {

  public:
    NodeQueue();
    NodeQueue(string name);
    virtual ~NodeQueue();

    string
        getName(); // returns the name of the queue

    virtual void
        pushNode(DagNode * node); // adds a node into the queue

    virtual void
        pushNodes(vector<DagNode *> nodes); // adds a vector of nodes into the queue

    virtual void
        removeNode(DagNode * node); // removes a node from the queue

    virtual bool
        notifyStateChange(DagNode * node) = 0; // notify the queue that a node's state changed

    virtual int
        size(); // returns the nb of nodes in the queue

    bool
        isEmpty();

  protected:

    // counter of nodes inserted in the queue (for queue destruction)
    int nodeCounter;


  private:

    // queue name
    string myName;

}; // end class NodeQueue

/**
 * Class ChainedNodeQueue
 * This class inherits from abstract class NodeQueue and is used to setup
 * a chain of node queues where a specific state change in a node will
 * trigger a move from one queue to another one.
 */

class ChainedNodeQueue : public NodeQueue {

  public:
    ChainedNodeQueue(NodeQueue * outputQ);
    ChainedNodeQueue(string name, NodeQueue * outputQ);
    virtual ~ChainedNodeQueue();

    bool
    notifyStateChange(DagNode * node); // moves the node to the output queue

  protected:
    NodeQueue * outputQ;  // pointer to the next queue

}; // end class ChainedNodeQueue

/**
 * Class OrderedNodeQueue
 * This class only provides a FIFO queue for nodes and does not implement
 * any action on state change
 */

class OrderedNodeQueue : public NodeQueue {

  public:
    OrderedNodeQueue();
    OrderedNodeQueue(string name);
    virtual ~OrderedNodeQueue();

    void
        pushNode(DagNode * node); // adds a node into the queue

    bool
        notifyStateChange(DagNode * node); // do nothing

/*    virtual DagNode *
        getFirstNode(); // returns a ref to the first node in the queue (or NULL)
 */
    virtual void
        removeNode(DagNode * node); // removes a node from the queue

    virtual DagNode *
        popFirstNode(); // pop out the first node from the nodes queue

    virtual void
        removeLastNodes(int nbNodesToKeep); // Remove queue's tail keeping only a maximum nb of nodes

    list<DagNode *>::iterator
        begin();        // get an iterator on the list of nodes

    list<DagNode *>::iterator
        end();

  protected:
    /**
     * The queue is implemented as a list
     */
    list<DagNode *> orderedNodes;

}; // end class OrderedNodeQueue

/**
 * Class PriorityNodeQueue
 *
 * This class inherits from OrderedNodeQueue and implements an ordering
 * method based on the priority of the nodes:
 * when a node is inserted in the queue it is
 * put at a position where its priority is greater or equal
 * to all nodes that follow it in the queue.
 */

class PriorityNodeQueue : public OrderedNodeQueue {

  public:
    PriorityNodeQueue();
    PriorityNodeQueue(string name);
    virtual ~PriorityNodeQueue();

    /**
     * Insert the node in the queue according to its priority
     */
    void
        pushNode(DagNode * node);

}; // end class PriorityNodeQueue

/**
 * Class NodeException
 *
 * This class is used as exception for all functions that manage workflow
 * nodes and that may trigger exceptions due to unusual properties of the
 * node:
 *  - eSERVICE_NOT_FOUND: triggered when the service of the node is not
 *                        available in the DIET hierarchy
 */

class NodeException {
  public:
    enum NodeErrorType { eSERVICE_NOT_FOUND };
    NodeException(NodeErrorType t) { this->why = t; }
    NodeErrorType TYpe()  { return this->why; }
  private:
    NodeErrorType why;
}; // end class NodeException


#endif  /* not defined _NODEQUEUE_HH_ */
