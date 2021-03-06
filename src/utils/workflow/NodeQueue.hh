/**
* @file  NodeQueue.hh
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


#ifndef _NODEQUEUE_HH_
#define _NODEQUEUE_HH_

#include <string>
#include <list>
#include <vector>
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif
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

class DIET_API_LIB NodeQueue {
public:
  NodeQueue();

  explicit NodeQueue(std::string name);

  virtual ~NodeQueue();

  std::string
  getName();  // returns the name of the queue

  virtual void
  pushNode(DagNode * node);  // adds a node into the queue

  // adds a vector of nodes into the queue
  virtual void
  pushNodes(std::vector<DagNode *> nodes);

  virtual void
  removeNode(DagNode * node);  // removes a node from the queue

  // notify the queue that a node's state changed
  virtual bool
  notifyStateChange(DagNode * node) = 0;

  virtual int
  size();  // returns the nb of nodes in the queue

  bool
  isEmpty();

protected:
  // counter of nodes inserted in the queue (for queue destruction)
  int nodeCounter;

private:
  // queue name
  std::string myName;
};

/**
 * Class ChainedNodeQueue
 * This class inherits from abstract class NodeQueue and is used to setup
 * a chain of node queues where a specific state change in a node will
 * trigger a move from one queue to another one.
 */
class DIET_API_LIB ChainedNodeQueue : public NodeQueue {
public:
  explicit ChainedNodeQueue(NodeQueue * outputQ);

  ChainedNodeQueue(std::string name, NodeQueue * outputQ);

  virtual ~ChainedNodeQueue();

  bool
  notifyStateChange(DagNode * node);  // moves the node to the output queue

protected:
  NodeQueue * outputQ;  // pointer to the next queue
};

/**
 * Class OrderedNodeQueue
 * This class only provides a FIFO queue for nodes and does not implement
 * any action on state change
 */
class DIET_API_LIB OrderedNodeQueue : public NodeQueue {
public:
  OrderedNodeQueue();

  explicit OrderedNodeQueue(std::string name);

  virtual ~OrderedNodeQueue();

  void
  pushNode(DagNode * node);  // adds a node into the queue

  bool
  notifyStateChange(DagNode * node);  // do nothing

  virtual void
  removeNode(DagNode * node);  // removes a node from the queue

  virtual DagNode *
  popFirstNode();  // pop out the first node from the nodes queue

  // Remove queue's tail keeping only a maximum nb of nodes
  virtual void
  removeLastNodes(int nbNodesToKeep);

  std::list<DagNode *>::iterator
  begin();        // get an iterator on the list of nodes

  std::list<DagNode *>::iterator
  end();

protected:
  /**
   * The queue is implemented as a list
   */
  std::list<DagNode *> orderedNodes;
};

/**
 * Class PriorityNodeQueue
 *
 * This class inherits from OrderedNodeQueue and implements an ordering
 * method based on the priority of the nodes:
 * when a node is inserted in the queue it is
 * put at a position where its priority is greater or equal
 * to all nodes that follow it in the queue.
 */

class DIET_API_LIB PriorityNodeQueue : public OrderedNodeQueue {
public:
  PriorityNodeQueue();

  explicit PriorityNodeQueue(std::string name);

  virtual ~PriorityNodeQueue();

  /**
   * Insert the node in the queue according to its priority
   */
  void
  pushNode(DagNode * node);
};

#endif  /* not defined _NODEQUEUE_HH_ */
