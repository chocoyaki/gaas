/****************************************************************************/
/* Node description class                                                   */
/* This class contains the diet profile, the i/o ports and the execution    */
/* object                                                                   */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.18  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.17  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.16  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.15  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.14  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.13  2008/07/17 13:34:18  bisnard
 * new attribute RealStartTime and get/set for SRPT heuristic
 *
 * Revision 1.12  2008/06/25 10:07:12  bisnard
 * - removed debug messages
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 *
 * Revision 1.11  2008/06/18 15:00:32  bisnard
 * use new Node attribute estDuration to store job duration for each node
 *
 * Revision 1.10  2008/06/04 07:52:38  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.9  2008/06/03 12:14:29  bisnard
 * New lastQueue attribute to allow node go back to prev queue
 *
 * Revision 1.8  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.7  2008/06/01 09:16:57  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 *
 * Revision 1.6  2008/05/16 12:33:32  bisnard
 * cleanup outputs of workflow node
 *
 * Revision 1.5  2008/04/30 07:28:56  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.4  2008/04/28 12:06:03  bisnard
 * changed constructor for Node (new param wfReqId)
 * Node delay at execution: new attr & methods
 *
 * Revision 1.3  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 ****************************************************************************/

#ifndef _NODE_HH_
#define _NODE_HH_

#include <iostream>
#include <string>
#include <set>
#include <vector>
#include <list>
#include <map>

#include "WfUtils.hh"
#include "WfPort.hh"

class NodeSet;

using namespace std;

class Node  {

public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /**
   * The Node default constructor
   * @param id         the node id
   * @param pbName     the node service name
   */
  Node(const string& id,
       const string& pbName);

  /**
   * Node destructor
   */
  virtual ~Node();

  /**
   * To get the node id *
   */
  const string&
  getId();

  /**
   * To get the node pb *
   */
  const string&
  getPb();

  /**
   * node container
   */
  virtual NodeSet*
  getNodeSet() = 0;

  /******************************/
  /* Links with other nodes     */
  /******************************/

  /**
   * add a previous node using the node id
   * (this is the only way to add new predecessors; after adding all node
   * ids the method setNodePredecessors is called to convert the
   * predecessors ids into object references)
   */
  void
  addPrevId(string nodeId);

  /**
   * initializes the previous nodes using both
   * the control dependencies (<prec> tag) and the
   * data dependencies (ports links)
   */
  bool
  setNodePrecedence(NodeSet* nodeSet);

  /**
   * return the number of previous nodes
   * (setNodePredecessors must be called before)
   */
  unsigned int
  prevNodesNb();

  /**
   * return an iterator on the first previous nodes
   */
  vector<Node*>::iterator
  prevNodesBegin();

  /**
   * return an iterator on the end of previous nodes
   */
  vector<Node*>::iterator
  prevNodesEnd();

  /**
   * Add a new next node reference *
   * @param n the next node reference
   */
  void
  addNext(Node * n);

    /**
   * return the number of next nodes
   */
  unsigned int
  nextNodesNb();

  /**
   * return an iterator on the first next node
   */
  list<Node*>::iterator
  nextNodesBegin();

  /**
   * return an iterator on the end of next nodes
   */
  list<Node*>::iterator
  nextNodesEnd();

  /**
   * Link the ports by object references
   */
  virtual void
  connectNodePorts();

  /**
   * return true if the node is an input node *
   * only the nodes with no previous node are considered as dag input  *
   */
  virtual bool
  isAnInput();

  /**
   * return true if the node is an output node *
   * only the nodes with no next node are considered as dag exit  *
   */
  virtual bool
  isAnExit();

  /******************************/
  /* PORTS                      */
  /******************************/

  /**
   * create and add a new port to the node *
   * @param id        the port identifier
   * @param ind       the port index in diet profile
   * @param type      the port type (in, out, inout)
   * @param type      the data type (constant defined in WfCst class)
   * @param depth     the depth of the list structure (0 if no list)
   * @param v         the parameter value
   */
  virtual WfPort *
  newPort(string id, uint ind, WfPort::WfPortType type, WfCst::WfDataType type,
          uint depth) = 0;

  /**
   * Get the port by id. If not found returns NULL
   * @param id the port id
   */
  WfPort*
  getPort(const string& id);

protected:

  virtual void
  setPrev(int index, Node * node);

  /**
   * Node id *
   */
  string myId;

  /**
   * Node problem
   */
  string myPb;

  /**
   * ports map<id, reference> *
   */
  map<string, WfPort*> ports;

  /**
   * The previous nodes list
   * (use a vector because nb of items is known by prevNodeIds)
   */
  vector<Node*> prevNodes;

  /**
   * The next nodes list
   * (use a list because nb of items is not known in advance)
   */
  list<Node*> nextNodes;

private:

  /**
   * the previous nodes ids
   * (use a set to avoid searching duplicate when inserting)
   */
  set<string> prevNodeIds;

};


#endif   /* not defined _NODE_HH */
