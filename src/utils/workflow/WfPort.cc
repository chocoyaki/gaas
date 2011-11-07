/**
* @file  WfPort.cc
* 
* @brief  The port classes used in workflow node (WfPort, WfOutPort and WfInPort) 
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/



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

std::string
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

std::string
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

