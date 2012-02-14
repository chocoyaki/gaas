/**
* @file  WfNode.cc
* 
* @brief This class contains the diet profile, the i/o ports and the execution object
* 
* @author  Abdelkader AMAR (Abdelkader.Amar@ens-lyon.fr)
*          Benjamin ISNARD (benjamin.isnard@ens-lyon.fr)
* 
* @section Licence
*   |LICENCE|                                                                
*/

#include "debug.hh"
#include "WfNode.hh"
#include "Dag.hh"  // for NodeSet definition
#ifdef WIN32
#define DIET_API_LIB __declspec(dllexport)
#else
#define DIET_API_LIB
#endif

/****************************************************************************/
/*                       Constructors/Destructor                            */
/****************************************************************************/

WfNode::WfNode(const std::string& id) : myId(id) {}

DIET_API_LIB WfNode::~WfNode() {
  // Free the ports map
  while (!ports.empty()) {
    WfPort * p = ports.begin()->second;
    ports.erase(ports.begin());
    delete p;
  }
}

/****************************************************************************/
/*                              Basic GET/SET                               */
/****************************************************************************/

DIET_API_LIB const std::string&
WfNode::getId() const {
  return this->myId;
}

/****************************************************************************/
/*                       Predecessor/Successors Mgmt                        */
/****************************************************************************/

/**
 * (protected)
 * add a new previous node id *
 */
DIET_API_LIB void
WfNode::addPrevId(const std::string& nodeId) {
  if (nodeId.empty()) {
    INTERNAL_ERROR("WfNode::addPrevId Fatal Error: Empty node id", 1);
  }
  prevNodeIds.insert(nodeId);  // set insertion
}

/**
 * (protected)
 * remove a previous node id *
 */
DIET_API_LIB void
WfNode::remPrevId(const std::string& nodeId) {
  if (nodeId.empty()) {
    INTERNAL_ERROR("WfNode::remPrevId Fatal Error: Empty node id", 1);
  }
  prevNodeIds.erase(nodeId);  // set removal
}

/**
 * (public) used by Dag::checkPrec()
 * initializes the list of node predecessors using both
 * the control dependencies (<prec> tag) and the
 * data dependencies (ports links)
 */
DIET_API_LIB void
WfNode::setNodePrecedence(NodeSet* nodeSet) throw(WfStructException) {
  // The predecessors defined by control links (<prec> tag) were
  // already added by the dag parser.
  // Add the predecessors defined by data links
  TRACE_TEXT(TRACE_ALL_STEPS, "Processing ports of node : "
             << myId << "\n");
  std::map<std::string, WfPort*>::iterator p = ports.begin();
  for (; p != ports.end(); ++p) {
    ((WfPort*)p->second)->setNodePrecedence(nodeSet);
  }
  // convert the predecessors defined by ID in prevNodeIds to
  // direct object references stored in prevNodes
  prevNodes.resize(prevNodeIds.size());
  int prevIdx = 0;
  std::set<std::string>::iterator idIter = prevNodeIds.begin();
  for (; idIter != prevNodeIds.end(); ++idIter) {
    WfNode * prevNode = nodeSet->getNode(*idIter);
    setPrev(prevIdx++, prevNode);
  }
}

/**
 * (public) Add a new predecessor
 * (may check some constraints before adding the predecessor effectively)
 */
DIET_API_LIB void
WfNode::addNodePredecessor(WfNode * node, const std::string& fullNodeId) {
  // no check is done in this class
  addPrevId(fullNodeId);
}

/**
 * (protected) Set a new previous node *
 * (does not check duplicates)
 */
DIET_API_LIB void
WfNode::setPrev(int index, WfNode * node) {
  prevNodes[index] = node;
  node->addNext(this);
}

/**
 * return the number of previous nodes
 */
unsigned int
WfNode::prevNodesNb() const {
  return prevNodes.size();
}

/**
 * return an iterator on the first previous nodes
 */
DIET_API_LIB std::vector<WfNode*>::iterator
WfNode::prevNodesBegin() {
  return prevNodes.begin();
}

/**
 * return an iterator on the end of previous nodes
 */
DIET_API_LIB std::vector<WfNode*>::iterator
WfNode::prevNodesEnd() {
  return prevNodes.end();
}

/**
 * Add a next node reference *
 * @param node the node to add
 */
DIET_API_LIB void
WfNode::addNext(WfNode *node) {
  nextNodes.push_back(node);
}

/**
 * return the number of next nodes
 */
unsigned int
WfNode::nextNodesNb() const {
  return nextNodes.size();
}

/**
 * return an iterator on the first next node
 */
std::list<WfNode*>::iterator
WfNode::nextNodesBegin() {
  return nextNodes.begin();
}

/**
 * return an iterator on the end of next nodes
 */
std::list<WfNode*>::iterator
WfNode::nextNodesEnd() {
  return nextNodes.end();
}

/**
 * return if the node is an input node *
 * only the nodes with no previous node are considered as dag input  *
 */
bool
WfNode::isAnInput() const {
  return (prevNodes.empty());
}

/**
 * return true if the node is an input node *
 * only the nodes with no next node are considered as dag exit  *
 */
bool
WfNode::isAnExit() const {
  return (nextNodes.empty());
}


/****************************************************************************/
/*                            Ports Mgmt                                    */
/****************************************************************************/

/**
 * link the ports by references
 */
void
WfNode::connectNodePorts() throw(WfStructException) {
  TRACE_TEXT(TRACE_ALL_STEPS,
              "connectNodePorts : processing node " << getId() << "\n");
  std::map<std::string, WfPort*>::iterator p = ports.begin();
  for (; p != ports.end(); ++p) {
    ((WfPort*)(p->second))->connectPorts();
  }
}

/**
 * check if port already exists
 */
bool
WfNode::isPortDefined(const std::string& id) {
  std::map<std::string, WfPort*>::iterator p = ports.find(id);
  return (p != ports.end());
}

/**
 * Add a new port to the ports map
 */
WfPort *
WfNode::addPort(const std::string& portId, WfPort* port)
  throw(WfStructException) {
  if (isPortDefined(portId)) {
    delete port;
    throw WfStructException(WfStructException::eDUPLICATE_PORT,
                            "port id="+portId);
  }
  ports[portId] = port;
  return port;
}

/**
 * Get the input port references by id *
 */
WfPort *
WfNode::getPort(const std::string& id) throw(WfStructException) {
  std::map<std::string, WfPort*>::iterator p = ports.find(id);
  if (p != ports.end()) {
    return ((WfPort*)(p->second));
  } else {
    throw WfStructException(WfStructException::eUNKNOWN_PORT,
                            "node id=" + myId + "/port id=" + id);
  }
}

/**
 * Get nb of ports
 */
unsigned int
WfNode::getPortNb() const {
  return ports.size();
}

/**
 * Get nb of ports by type
 */
unsigned int
WfNode::getPortNb(WfPort::WfPortType portType) const {
  unsigned int count = 0;
  std::map<std::string, WfPort*>::const_iterator portIter = ports.begin();
  while (portIter != ports.end()) {
    WfPort *port = (WfPort*) portIter->second;
    if (port->getPortType() == portType) {
      count++;
    }
    ++portIter;
  }
  return count;
}

/**
 * Get port by index
 */
const WfPort*
WfNode::getPortByIndex(unsigned int portIdx) const {
  std::map<std::string, WfPort*>::const_iterator portIter = ports.begin();
  while (portIter != ports.end()) {
    const WfPort *port = (const WfPort*) portIter->second;
    if (port->getIndex() == portIdx) {
      return port;
    }
    ++portIter;
  }
  return NULL;
}

/**
 * Get description of ports (used for error msg)
 */
std::string
WfNode::getPortsDescr() const {
  std::string descrStr;
  for (unsigned int ix = 0; ix < getPortNb(); ++ix) {
    const WfPort* port = getPortByIndex(ix);
    descrStr += port->getPortDescr();
    if (ix < getPortNb()-1) {
      descrStr += ", ";
    }
  }
  return descrStr;
}
