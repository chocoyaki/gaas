/**
* @file  WfNode.hh
* 
* @brief  Node description class that contains the i/o ports of a workflow node 
* 
* @author  - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENSE|                                                                
*/
/* $Id$
 * $Log$
 * Revision 1.4  2011/01/12 09:12:18  bdepardo
 * Removed warning during code documentation generation
 *
 * Revision 1.3  2009/10/12 15:00:14  bisnard
 * added some const
 *
 * Revision 1.2  2009/05/27 08:43:42  bisnard
 * added new addPort method to avoid direct access to map
 *
 * Revision 1.1  2009/04/17 08:54:44  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.25  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.24  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.23  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.22  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.21  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.20  2008/10/22 08:52:39  bisnard
 * duplicate parameter name in newPort()
 *
 * Revision 1.19  2008/10/20 08:02:19  bisnard
 * moved pb name attribute from Node to DagNode class
 *
 * Revision 1.18  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode, DagNodePort)
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
#include <list>
#include <map>
#include <set>
#include <string>
#include <vector>
#include "WfUtils.hh"
#include "WfPort.hh"

class NodeSet;

class WfNode  {
public:
  /*********************************************************************/
  /* public methods                                                    */
  /*********************************************************************/

  /**
   * The Node default constructor
   * @param id         the node id
   */
  WfNode(const std::string& id);

  /**
   * Node destructor
   */
  virtual ~WfNode();

  /**
   * To get the node id *
   */
  const std::string&
  getId() const;


  /******************************/
  /* Links with other nodes     */
  /******************************/

  /**
   * initializes the previous nodes using both
   * the control dependencies (\<prec\> tag) and the
   * data dependencies (ports links)
   */
  virtual void
  setNodePrecedence(NodeSet *nodeSet) throw(WfStructException);

  /**
   * Add a new predecessor
   * (may check some constraints before adding the predecessor effectively)
   * @param node  ptr to the predecessor
   * @param fullNodeId  contains the id of the predecessor eventually prefixed (if external)
   */
  virtual void
  addNodePredecessor(WfNode *node, const std::string& fullNodeId);

  /**
   * return the number of previous nodes
   * (setNodePredecessors must be called before)
   */
  unsigned int
  prevNodesNb() const;

  /**
   * return an iterator on the first previous nodes
   */
  std::vector<WfNode*>::iterator
  prevNodesBegin();

  /**
   * return an iterator on the end of previous nodes
   */
  std::vector<WfNode*>::iterator
  prevNodesEnd();

  /**
   * return the number of next nodes
   */
  unsigned int
  nextNodesNb() const;

  /**
   * return an iterator on the first next node
   */
  std::list<WfNode*>::iterator
  nextNodesBegin();

  /**
   * return an iterator on the end of next nodes
   */
  std::list<WfNode*>::iterator
  nextNodesEnd();

  /**
   * Link the ports by object references
   */
  virtual void
  connectNodePorts() throw(WfStructException);

  /**
   * return true if the node is an input node *
   * only the nodes with no previous node are considered as dag input  *
   */
  virtual bool
  isAnInput() const;

  /**
   * return true if the node is an output node *
   * only the nodes with no next node are considered as dag exit  *
   */
  virtual bool
  isAnExit() const;

  /******************************/
  /* PORTS                      */
  /******************************/

  /**
   * create and add a new port to the node *
   * @param id        the port identifier
   * @param ind       the port index in diet profile
   * @param portType  the port type (in, out, inout)
   * @param dataType  the data type (constant defined in WfCst class)
   * @param depth     the depth of the list structure (0 if no list)
   */
  virtual WfPort *
  newPort(std::string id,
          unsigned int ind,
          WfPort::WfPortType portType,
          WfCst::WfDataType dataType,
          unsigned int depth) throw(WfStructException) = 0;

  /**
   * check if port already exists
   * @return true if the port id is already defined
   */
  bool
  isPortDefined(const std::string& id);

  /**
   * Get the port by id. If not found returns NULL
   * @param id the port id
   */
  WfPort*
  getPort(const std::string& id) throw(WfStructException);

  /**
   * Get nb of ports
   */
  unsigned int
  getPortNb() const;

  /**
   * Get nb of ports by type
   */
  unsigned int
  getPortNb(WfPort::WfPortType portType) const;

  /**
   * Get port by index
   */
  const WfPort*
  getPortByIndex(unsigned int portIdx) const;

  /**
   * Get ports description
   * @return a std::string (example: "IN DIET_INT, OUT DIET_DOUBLE")
   */
  std::string
  getPortsDescr() const;

protected:
  /**
   * add a previous node using the node id
   * (this is the only way to add new predecessors; after adding all node
   * ids the method setNodePredecessors is called to convert the
   * predecessors ids into object references)
   */
  void
  addPrevId(const std::string& nodeId);

  /**
   * remove a previous node using the node id
   */
  void
  remPrevId(const std::string& nodeId);

  /**
   * Set the precedence relationship between two nodes
   * ie add the successor to the list of successors of the predecessor
   * and add the predecessor to the list of predecessors of the successor
   * @param index the index of the predecessor (in the list of predecessors)
   * @param node  the predecessor ref
   */
  virtual void
  setPrev(int index, WfNode * node);

  /**
   * Add a new next node reference *
   * @param n the next node reference
   */
  void
  addNext(WfNode * n);

  /**
   * Node id *
   */
  std::string myId;

  /**
   * Method to add a port
   * @param portId  identifier of the port
   * @param port    pointer to the port
   * @return  same pointer as argument
   * @exception WfStructException if duplicate port (pointer is deleted)
   */
  WfPort *
  addPort(const std::string& portId, WfPort* port)
    throw(WfStructException);

  /**
   * ports map<id, reference> *
   */
  std::map<std::string, WfPort*> ports;

  /**
   * The previous nodes list
   * (use a vector because nb of items is known by prevNodeIds)
   */
  std::vector<WfNode*> prevNodes;

  /**
   * The next nodes list
   * (use a list because nb of items is not known in advance)
   */
  std::list<WfNode*> nextNodes;

private:

  /**
   * the previous nodes ids
   * (use a set to avoid searching duplicate when inserting)
   */
  std::set<std::string> prevNodeIds;
};

#endif   /* not defined _NODE_HH */
