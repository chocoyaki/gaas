/****************************************************************************/
/* The port classes used in workflow node                                   */
/* The main classes are WfPort, WfOutPort and WfInPort                      */
/*                                                                          */
/* Author(s):                                                               */
/* - Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)                          */
/* - Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.16  2009/06/15 12:24:30  bisnard
 * new class DagNodeArgPort (arg ports not used for funct wf anymore)
 * use WfDataWriter class to display data
 *
 * Revision 1.15  2009/05/27 08:49:43  bisnard
 * - modified condition output: new IF_THEN and IF_ELSE port types
 * - implemented MERGE and FILTER workflow nodes
 *
 * Revision 1.14  2009/04/17 08:54:44  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.13  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.12  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.11  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.10  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.9  2008/11/07 13:42:05  bdepardo
 * Added two getters in WfPort:
 * - getDataType
 * - getEltDataType.
 * Use them in DagNode::displayResults
 *
 * Revision 1.8  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.7  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.6  2008/10/02 08:28:47  bisnard
 * new WfPort method to free persistent data
 *
 * Revision 1.5  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.4  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.3  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.2  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.3  2007/04/20 09:43:28  ycaniou
 * Cosmetic changements for less warnings when compiling with doc.
 * Still some errors to correct (parameters to detail) that I cannot do.
 *
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#ifndef _WFPORT_HH_
#define _WFPORT_HH_

#include <string>
#include <list>

#include "WfUtils.hh"

using namespace std;

class WfNode;
class NodeSet;
class WfSimplePortAdapter;
class WfPortAdapter;

class WfPort {

  // single adapter is friend as it manages the connection btw two ports
  friend class WfSimplePortAdapter;

public:

  enum WfPortType{
    PORT_IN,
    PORT_ARG,
    PORT_INOUT,
    PORT_OUT,
    PORT_PARAM,
    PORT_OUT_THEN,
    PORT_OUT_ELSE
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
  WfPort(WfNode * parent, const string& _id, WfPortType _portType,
         WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind);

  virtual ~WfPort();

  /**
   * Set the parameter of a matrix argument
   * @param nbr  Row count
   * @param nbc  Column count
   * @param o    Matrix order
   * @param bt   Data type of matrix elements
   */
  void setMatParams(long nbr, long nbc,
                    WfCst::WfMatrixOrder o,
                    WfCst::WfDataType bt);

  /**
   * Return the port id
   */
  const string&
  getId() const;

  /**
   * Return the complete ID (nodeid#portid)
   */
  virtual string
  getCompleteId() const;

  /**
   * Return the port type
   */
  short
  getPortType() const;

  /**
   * Return a description of the port
   */
  string
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
   *  returns base data type if eltDepth=port depth)
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
  setCardinal(const list<string>& cardList);

  /**
   * Set the source of the input port (Parsing only)
   * @param strRef The source port (output port) reference
   */
  void
  setConnectionRef(const string& strRef);

  /**
   * Set the port adapter
   */
  void
  setPortAdapter(WfPortAdapter* adapter);

  /**
   * Nodes linking (used for dag scheduling)
   * @param contextNodeSet  the node set used to find references to nodes
   */
  void
  setNodePrecedence(NodeSet * contextNodeSet) throw (WfStructException);

  /**
   * Ports linking (used for node execution)
   * @param nodeSet   container for the linked nodes
   */
  void
  connectPorts() throw (WfStructException);

  /**
   * Return true if the port is connected to another port
   * (through an object link)
   */
  bool
  isConnected() const;

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
  string id;

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
  list<string> * card;

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
