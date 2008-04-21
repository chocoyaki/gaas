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

#ifndef _NODEQUEUE_HH_
#define _NODEQUEUE_HH_

#include <list>
#include "Node.hh"

class NodeQueue {

  public:
    NodeQueue(int wfReqId);
    virtual ~NodeQueue();

    virtual void
        pushNode(Node * node); // adds a node into the queue

    virtual void
        pushNodes(std::vector<Node *> nodes); // adds a vector of nodes into the queue

    virtual void
        notifyStateChange(Node * node); // notify the queue that a node has a new state

    virtual Node *
        popFirstReadyNode(); // get the first node in the ready nodes queue

    int
        getWfReqId();

  protected:

      // implements selection and ordering method (eg random or based on rank)
    virtual void
        insertReadyNode(Node * node);

  private:
//       // this list contains all the nodes in scheduling order
//       list<Node *> qNodes;

      // this list contains all the ready nodes
    list<Node *> rNodes;

      // the workflow request ID
    int myReqId;

}; // end class NodeQueue

class NodeException {
  public:
    enum NodeErrorType { eSERVICE_NOT_FOUND };
    NodeException(NodeErrorType t) { this->why = t; }
    NodeErrorType TYpe()  { return this->why; }
  private:
    NodeErrorType why;
}; // end class NodeException


#endif  /* not defined _NODEQUEUE_HH_ */
