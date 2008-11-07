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
 * Revision 1.11  2008/11/07 13:42:05  bdepardo
 * Added two getters in WfPort:
 * - getDataType
 * - getEltDataType.
 * Use them in DagNode::displayResults
 *
 * Revision 1.10  2008/10/27 16:19:58  bisnard
 * correction of bug in connectPorts
 *
 * Revision 1.9  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.8  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.7  2008/10/02 08:28:47  bisnard
 * new WfPort method to free persistent data
 *
 * Revision 1.6  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.5  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.4  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.3  2008/09/19 14:01:30  bisnard
 * allow compile wf support with or without DAGDA
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
 * Revision 1.2  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 ****************************************************************************/

#include "WfPort.hh"
#include "debug.hh"

WfPort::WfPort(Node * parent, const string& _id, WfPortType _portType,
               WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind) :
  myParent(parent), id(_id), portType(_portType), type(_type),
  depth(_depth), index(_ind), adapter(NULL), nb_r(0), nb_c(0), connected(false) {
  if (_depth > 0) {
    eltType = _type; // store the elements type
    type = WfCst::TYPE_CONTAINER;
  }
}

WfPort::~WfPort() { }

void
WfPort::setMatParams(long nbr, long nbc,
		    WfCst::WfMatrixOrder o,
		    WfCst::WfDataType bt) {
  this->nb_r = nbr;
  this->nb_c = nbc;
  this->order = o;
  this->eltType = bt;
}

string
WfPort::getId() {
  return this->id;
}

short
WfPort::getPortType() {
  return this->portType;
}

Node*
WfPort::getParent() {
  return this->myParent;
}

unsigned int
WfPort::getIndex() {
  return this->index;
}

unsigned int
WfPort::getDepth() {
  return this->depth;
}

WfCst::WfDataType
WfPort::getDataType() {
  return this->type;
}

WfCst::WfDataType
WfPort::getEltDataType() {
  return this->eltType;
}


void
WfPort::setConnectionRef(const string& strRef) {
  // create the appropriate adapter (simple or split) depending on ref parsing
  this->adapter = WfPortAdapter::createAdapter(strRef);
}

void
WfPort::setAsConnected() {
  this->connected = true;
}

bool
WfPort::isConnected() {
  return connected;
}

bool
WfPort::setNodePrecedence(NodeSet* nodeSet) {
  if (adapter) { // in case this method is called on an argument port
    if (!adapter->setNodePrecedence(getParent(), nodeSet))
      return false;
  }
  return true;
}

void
WfPort::connectPorts(NodeSet* nodeSet) {
  if (adapter) { // this method may be called on an argument port
    adapter->connectPorts(this, nodeSet);
    this->setAsConnected();
  }
}

