/**
* @file  WfPort.hh
* 
* @brief  The port classes used in workflow node (WfPort, WfOutPort and WfInPort) 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/


#ifndef _WFPORT_HH_
#define _WFPORT_HH_

#include <list>
#include <string>
#include "WfUtils.hh"

class WfNode;
class NodeSet;
class WfSimplePortAdapter;
class WfPortAdapter;

class WfPort {
  // single adapter is friend as it manages the connection btw two ports
  friend class WfSimplePortAdapter;

public:

  enum WfPortType {
    PORT_IN,
    PORT_ARG,
    PORT_INOUT,
    PORT_OUT,
    PORT_PARAM,
    PORT_OUT_THEN,
    PORT_OUT_ELSE,
    PORT_IN_LOOP,
    PORT_OUT_LOOP
  };

  /**
   * Basic Port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _portType The port type
   * @param _type  The port data base type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   */
  WfPort(WfNode * parent, const std::string& _id, WfPortType _portType,
         WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);

  virtual ~WfPort();

  /**
   * Set the parameter of a matrix argument
   * @param nbr  Row count
   * @param nbc  Column count
   * @param o    Matrix order
   * @param bt   Data type of matrix elements
   */
  void
  setMatParams(long nbr,
               long nbc,
               WfCst::WfMatrixOrder o,
               WfCst::WfDataType bt);

  /**
   * Return the port id
   */
  const std::string&
  getId() const;

  /**
   * Return the complete ID (nodeid#portid)
   */
  virtual std::string
  getCompleteId() const;

  /**
   * Return the port type
   */
  short
  getPortType() const;

  /**
   * Returns true if the port is an input
   */
  bool
  isInput() const;

  /**
   * Returns true if the port is an output
   */
  bool
  isOutput() const;

  /**
   * Return a description of the port
   */
  std::string
  getPortDescr() const;

  /**
   * Return the port's parent node
   */
  WfNode*
  getParent() const;

  /**
   * Return the port index
   */
  unsigned int
  getIndex() const;

  /**
   * Return the port depth (container)
   */
  unsigned int
  getDepth() const;

  /**
   * Returns the data type (constants defined in WfCst class)
   */
  WfCst::WfDataType
  getDataType() const;

  /**
   * Returns the data type at a given depth
   * (returns container if eltDepth lower than port depth,
   *  returns base data type if eltDepth = port depth)
   */
  WfCst::WfDataType
  getDataType(unsigned int eltDepth) const;

  /**
   * Returns the data type of elements (in case of CONTAINER or MATRIX type)
   */
  WfCst::WfDataType
  getEltDataType() const;

  /**
   * Returns the base data type (in all cases)
   */
  WfCst::WfDataType
  getBaseDataType() const;

  /**
   * Set the cardinal of the containers
   * @param cardList  a list of values
   */
  void
  setCardinal(const std::list<std::string>& cardList);

  /******************************/
  /* Links with other ports     */
  /******************************/

  /**
   * Set the source of the input port (Parsing only)
   * @param strRef The source port (output port) reference
   */
  virtual void
  setConnectionRef(const std::string& strRef);

  /**
   * Set the port adapter
   */
  virtual void
  setPortAdapter(WfPortAdapter* adapter);

  /**
   * Nodes linking (used for dag scheduling)
   * @param contextNodeSet  the node set used to find references to nodes
   */
  virtual void
  setNodePrecedence(NodeSet * contextNodeSet) throw(WfStructException);

  /**
   * Ports linking (used for node execution)
   */
  virtual void
  connectPorts() throw(WfStructException);

  /**
   * Return true if the port is connected to another port
   * (through an object link)
   */
  virtual bool
  isConnected() const;

  /****************************************************/
  /* Interface with node's internal sources/sinks     */
  /****************************************************/

  virtual void
  setInterfaceRef(const std::string& strInterface);

  virtual const std::string&
  getInterfaceRef();

protected:

  /**
   * Ports linking on one side only
   * @param remPort  remote port
   */
  virtual void
  connectToPort(WfPort* remPort);

  /**
   * The id of the port
   * (must be unique vithin a given node)
   */
  std::string id;

  /**
   * The port type (in, out, inout)
   */

  WfPortType portType;

  /**
   * The data type (constants defined in WfCst class)
   */
  WfCst::WfDataType type;

  /**
   * The data type of elements (in case of CONTAINER or MATRIX type)
   */
  WfCst::WfDataType eltType;

  /**
   * The depth of the list structure (eg if type="LIST(LIST(INT))")
   */
  unsigned int depth;

  /**
   * The cardinal of the list structure (ie nb of elements at each level)
   * HYP: at a given level all elements have the same cardinal
   * The values stored in the list can be
   *  - an unsigned integer (as a string) if the cardinal is known
   *  - an 'x' if the cardinal is not known
   */
  std::list<std::string> * card;

  /**
   * The index of the port
   */
  unsigned int index;

  /**
   * The row count (for matrix parameter)
   */
  long nb_r;

  /**
   * The column count (for matrix parameter)
   */
  long nb_c;

  /**
   * The matrix order (for matrix parameter)
   */
  WfCst::WfMatrixOrder order;

  /**
   * The adapter to the linked port (source or sink)
   */
  WfPortAdapter* adapter;

  /**
   * The name of interface port to connect to
   */
  std::string myInterfaceRef;

private:

  /**
   * The reference of port node
   */

  WfNode * myParent;

  /**
   * The connection status
   */
  bool connected;
};

#endif   /* not defined _WFPORT_HH */
