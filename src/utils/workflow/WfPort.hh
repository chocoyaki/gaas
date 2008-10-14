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

#include "WfPortAdapter.hh"
#include "WfUtils.hh"

using namespace std;

class Node;
class NodeSet;

class WfPort {
public:

  enum WfPortType{
    PORT_IN,
    PORT_INOUT,
    PORT_OUT
  };

  /**
   * Basic Port constructor *
   * @param parent The node of the port
   * @param _id    The id of the port
   * @param _portType The port type (IN, INOUT or OUT)
   * @param _type  The port data base type
   * @param _depth The depth of the list structure (0 if no list)
   * @param _ind   The index of the parameter in the diet profile
   */
  WfPort(Node * parent, const string& _id, WfPortType _portType,
         WfCst::WfDataType _type, uint _depth, uint _ind);

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
  string
  getId();

  /**
   * Return the port type
   */
  short
  getPortType();

  /**
   * Return the port's parent node
   */
  Node*
  getParent();

  /**
   * Return the port index
   */
  unsigned int
  getIndex();

  /**
   * Return the port depth (container)
   */
  unsigned int
  getDepth();

  /**
   * Set the source of the input port (Parsing only)
   * @param s The source port (output port) reference
   */
  void
  setConnectionRef(const string& strRef);

  /**
   * Set the port as connected
   * (means that there is a linked port, but not that the connectPorts() method
   *  has been called)
   */
  void
  setAsConnected();

  /**
   * Return true if the port is connected to another port
   * (ie source or sink attribute is defined)
   */
  bool
  isConnected();

  /**
   * Nodes linking (used for dag scheduling)
   */
  bool
  setNodePrecedence(NodeSet* nodeSet);

  /**
   * Ports linking (used for node execution)
   * @param dag   the dag that contains the linked ports
   */
  void
  connectPorts(NodeSet* nodeSet);


protected:

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

  Node * myParent;

  /**
   * The connection status
   */
  bool connected;
};

#endif   /* not defined _WFPORT_HH */
