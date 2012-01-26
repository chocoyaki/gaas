/**
* @file  WfNode.hh
* 
* @brief  Node description class that contains the i/o ports of a workflow node 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


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
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

class NodeSet;

class DIET_API_LIB WfNode  {
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
