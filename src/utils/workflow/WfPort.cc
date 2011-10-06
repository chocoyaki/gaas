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
 * Revision 1.21  2010/08/26 07:48:39  bisnard
 * added check for non-connected input ports
 *
 * Revision 1.20  2009/08/26 10:32:11  bisnard
 * corrected  warnings
 *
 * Revision 1.19  2009/07/10 12:55:59  bisnard
 * implemented while loop workflow node
 *
 * Revision 1.18  2009/07/07 09:04:40  bisnard
 * new method isOutput to handle output ports that have different types
 *
 * Revision 1.17  2009/06/15 12:24:30  bisnard
 * new class DagNodeArgPort (arg ports not used for funct wf anymore)
 * use WfDataWriter class to display data
 *
 * Revision 1.16  2009/04/17 08:54:44  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.15  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.14  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.13  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.12  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
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
#include "WfPortAdapter.hh"
#include "WfNode.hh"
#include "debug.hh"

WfPort::WfPort(WfNode * parent, const std::string& _id, WfPortType _portType,
               WfCst::WfDataType _type, unsigned int _depth, unsigned int _ind)
  : id(_id), portType(_portType), type(_type), depth(_depth), card(NULL),
    index(_ind), nb_r(0), nb_c(0), adapter(NULL), myParent(parent),
    connected(false)  {
  if (!parent) {
    INTERNAL_ERROR("Missing parent for port creation", 0);
  }
  if (_depth > 0) {
    eltType = _type;  // store the elements type
    type = WfCst::TYPE_CONTAINER;
  }
}

WfPort::~WfPort() {
  delete adapter;
  delete card;
}

void
WfPort::setMatParams(long nbr, long nbc,
                     WfCst::WfMatrixOrder o,
                     WfCst::WfDataType bt) {
  this->nb_r = nbr;
  this->nb_c = nbc;
  this->order = o;
  this->eltType = bt;
}

const std::string&
WfPort::getId() const {
  return this->id;
}

string
WfPort::getCompleteId() const {
  return (this->myParent->getId() + "#" + this->id);
}

short
WfPort::getPortType() const {
  return this->portType;
}

bool
WfPort::isOutput() const {
  return ((portType == PORT_OUT)
          || (portType == PORT_INOUT)
          || (portType == PORT_OUT_THEN)
          || (portType == PORT_OUT_ELSE)
          || (portType == PORT_OUT_LOOP));
}

bool
WfPort::isInput() const {
  return ((portType == PORT_IN)
          || (portType == PORT_IN_LOOP)
          || (portType == PORT_INOUT));
}

string
WfPort::getPortDescr() const {
  std::string portDescr;
  switch (portType) {
  case PORT_PARAM:
    portDescr = "param ";
    break;
  case PORT_IN:
    portDescr = "in ";
    break;
  case PORT_ARG:
    portDescr = "arg ";
    break;
  case PORT_INOUT:
    portDescr = "inout ";
    break;
  case PORT_OUT:
    portDescr = "out ";
    break;
  case PORT_OUT_THEN:
    portDescr = "outThen ";
    break;
  case PORT_OUT_ELSE:
    portDescr = "outElse ";
    break;
  case PORT_IN_LOOP:
    portDescr = "inLoop ";
    break;
  case PORT_OUT_LOOP:
    portDescr = "outLoop ";
    break;
  }
  portDescr += WfCst::cvtWfToStrType(getDataType());
  return portDescr;
}

WfNode*
WfPort::getParent() const {
  return this->myParent;
}

unsigned int
WfPort::getIndex() const {
  return this->index;
}

unsigned int
WfPort::getDepth() const {
  return this->depth;
}

WfCst::WfDataType
WfPort::getDataType() const {
  return this->type;
}

WfCst::WfDataType
WfPort::getDataType(unsigned int eltDepth) const {
  if (eltDepth == this->depth) {
    return getBaseDataType();
  } else if (eltDepth < this->depth) {
    return WfCst::TYPE_CONTAINER;
  } else {
    INTERNAL_ERROR(
      "Wrong eltDepth in getDataType (greater than port depth)", 1);
  }
}

WfCst::WfDataType
WfPort::getEltDataType() const {
  if (!WfCst::isMatrixType(type)) {
    INTERNAL_ERROR(
      "getEltDataType() should not be used for types other than matrix", 1);
  }
  return this->eltType;
}

WfCst::WfDataType
WfPort::getBaseDataType() const {
  return (depth > 0) ? eltType : type;
}

void
WfPort::setCardinal(const std::list<std::string>& cardList) {
  if (!card) {
    card = new std::list<std::string>(cardList);
  } else {
    *card = cardList;
  }
}

void
WfPort::setConnectionRef(const std::string& strRef) {
  // create the appropriate adapter (simple or split) depending on ref parsing
  this->adapter = WfPortAdapter::createAdapter(strRef);
}

void
WfPort::setPortAdapter(WfPortAdapter* adapter) {
  this->adapter = adapter;
}

void
WfPort::setNodePrecedence(NodeSet* contextNodeSet) throw(WfStructException) {
  if (adapter) {  // in case this method is called on an argument port
    adapter->setNodePrecedence(getParent(), contextNodeSet);
  }
}

void
WfPort::connectPorts() throw(WfStructException) {
  if (adapter) {
    adapter->connectPorts(this, 0);
  }
}

bool
WfPort::isConnected() const {
  return connected;
}

void
WfPort::setInterfaceRef(const std::string& strInterface) {
  myInterfaceRef = strInterface;
}

const std::string&
WfPort::getInterfaceRef() {
  return myInterfaceRef;
}

/*************************************/
/*          PROTECTED                */
/*************************************/

void
WfPort::connectToPort(WfPort * remPort) {
  this->connected = true;
}

