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
 * Revision 1.2  2009/05/27 08:43:41  bisnard
 * added new addPort method to avoid direct access to map
 *
 * Revision 1.1  2009/04/17 08:54:44  bisnard
 * renamed Node class as WfNode
 *
 * Revision 1.35  2009/02/24 14:01:05  bisnard
 * added dynamic parameter mgmt for wf processors
 *
 * Revision 1.34  2009/02/06 14:55:08  bisnard
 * setup exceptions
 *
 * Revision 1.33  2009/01/16 13:55:36  bisnard
 * changes in dag event handling methods
 *
 * Revision 1.32  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.31  2008/10/20 08:02:19  bisnard
 * moved pb name attribute from Node to DagNode class
 *
 * Revision 1.30  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.29  2008/10/02 08:28:47  bisnard
 * new WfPort method to free persistent data
 *
 * Revision 1.28  2008/10/02 07:35:10  bisnard
 * new constants definitions (matrix order and port type)
 *
 * Revision 1.27  2008/09/30 15:32:53  bisnard
 * - using simple port id instead of composite ones
 * - dag nodes linking refactoring
 * - prevNodes and nextNodes data structures modified
 * - prevNodes initialization by Node::setNodePredecessors
 *
 * Revision 1.26  2008/09/30 09:23:29  bisnard
 * removed diet profile initialization from DagWfParser and replaced by node methods initProfileSubmit and initProfileExec
 *
 * Revision 1.25  2008/09/19 14:01:30  bisnard
 * allow compile wf support with or without DAGDA
 *
 * Revision 1.24  2008/09/19 13:11:07  bisnard
 * - added support for containers split/merge in workflows
 * - added support for multiple port references
 * - profile for node execution initialized by port (instead of node)
 * - ports linking managed by ports (instead of dag)
 *
 * Revision 1.23  2008/07/17 13:34:18  bisnard
 * new attribute RealStartTime and get/set for SRPT heuristic
 *
 * Revision 1.22  2008/07/17 10:05:06  bisnard
 * avoid warnings in DTM due to replacement of VOLATILE
 * for node arguments by PERSISTENT
 *
 * Revision 1.21  2008/07/11 07:55:37  bisnard
 * bug due to node simultaneously done and failed
 *
 * Revision 1.20  2008/07/09 13:17:15  rbolze
 * add the reqID in the diet_call DONE trace_text
 *
 * Revision 1.19  2008/07/08 11:15:58  bisnard
 * Correct dag/node destruction with nodequeues
 *
 * Revision 1.18  2008/07/07 16:17:58  bisnard
 * Stop calling next nodes if dag cancelled
 *
 * Revision 1.17  2008/07/04 10:00:07  bisnard
 * for DAGDA compatibility: use PERSISTENT_RETURN data
 *
 * Revision 1.16  2008/06/25 10:07:12  bisnard
 * - removed debug messages
 * - Node index in wf_response stored in Node class (new attribute submitIndex)
 *
 * Revision 1.15  2008/06/19 10:17:41  bisnard
 * remove some debug mess
 *
 * Revision 1.14  2008/06/18 15:00:32  bisnard
 * use new Node attribute estDuration to store job duration for each node
 *
 * Revision 1.13  2008/06/04 07:52:37  bisnard
 * SeD mapping done by MaDag just before node execution
 *
 * Revision 1.12  2008/06/03 12:14:29  bisnard
 * New lastQueue attribute to allow node go back to prev queue
 *
 * Revision 1.11  2008/06/02 08:35:39  bisnard
 * Avoid MaDag crash in case of client-SeD comm failure
 *
 * Revision 1.10  2008/06/01 14:06:56  rbolze
 * replace most ot the cout by adapted function from debug.cc
 * there are some left ...
 *
 * Revision 1.9  2008/06/01 09:16:56  rbolze
 * remove myreqID attribute from the RunnableNode
 * add getReqID() method which return the reqID stored in the diet_profile_t
 *
 * Revision 1.8  2008/05/28 20:53:33  rbolze
 * now DIET_PARAMSTRING type can be use in DAG.
 *
 * Revision 1.7  2008/05/16 12:33:32  bisnard
 * cleanup outputs of workflow node
 *
 * Revision 1.6  2008/04/30 07:28:56  bisnard
 * use relative timestamps for estimated and real completion time
 *
 * Revision 1.5  2008/04/28 12:06:03  bisnard
 * changed constructor for Node (new param wfReqId)
 * Node delay at execution: new attr & methods
 *
 * Revision 1.4  2008/04/21 14:36:59  bisnard
 * use nodeQueue to manage multiwf scheduling
 * use wf request identifer instead of dagid to reference client
 * renamed WfParser as DagWfParser
 *
 * Revision 1.3  2008/04/15 14:19:54  bisnard
 * - Postpone sed mapping until wf node is executed
 * - Removed diet_call_common2 function
 *
 * Revision 1.2  2008/04/14 09:10:40  bisnard
 *  - Workflow rescheduling (CltReoMan) no longer used with MaDag v2
 *  - AbstractWfSched and derived classes no longer used with MaDag v2
 *
 * Revision 1.1  2008/04/10 08:38:50  bisnard
 * New version of the MaDag where workflow node execution is triggered by the MaDag agent and done by a new CORBA object CltWfMgr located in the client
 *
 * Revision 1.13  2007/06/29 14:28:19  rbolze
 * update code following the change of the function diet_call_common
 * now the dietReqID is in the diet_profile
 *
 * Revision 1.12  2007/05/30 11:16:37  aamar
 * Updating workflow runtime to support concurrent call (Reordering is not
 * working now - TO FIX -).
 *
 * Revision 1.11  2006/11/27 10:15:12  aamar
 * Correct headers of files used in workflow support.
 *
 * Revision 1.10  2006/11/13 12:31:58  aamar
 * Remove reference to FD (not used).
 *
 * Revision 1.9  2006/11/06 15:16:06  aamar
 * Workflow support: some correction about reqID.
 *
 * Revision 1.8  2006/11/06 12:23:59  aamar
 * Workflow support: correct the value of reqID passed to RunnableNode.
 *
 * Revision 1.7  2006/11/06 11:57:07  aamar
 * *** empty log message ***
 *
 * Revision 1.6  2006/11/02 17:13:13  rbolze
 * dd some debug info
 *
 * Revision 1.5  2006/10/28 23:17:08  aamar
 * Free the persistent data when they are not needed anymore.
 * Replace cout/cerr debug message by debug.hh macros.
 *
 * Revision 1.4  2006/10/23 23:50:19  aamar
 * Coorect a problem with the use of INTERNAL_ERROR macro
 *
 * Revision 1.3  2006/10/20 08:42:14  aamar
 * Merging the base class BasicNode in Node class.
 * Adding some functions.
 * Correcting some bugs.
 *
 * Revision 1.2  2006/07/10 11:07:37  aamar
 * - Adding Matrix data type support
 * - The toXML (for Node and Ports) method that return XML
 * representation
 * - Some function and attributes used for scheduling and
 * rescheduling
 *
 * Revision 1.1  2006/04/14 13:50:40  aamar
 * Class representing a Dag node (source). This is a BasicNode
 * subclass that includes necessary code for execution).
 *
 ****************************************************************************/

#include "debug.hh"
#include "WfNode.hh"
#include "Dag.hh" // for NodeSet definition

/****************************************************************************/
/*                       Constructors/Destructor                            */
/****************************************************************************/

WfNode::WfNode(const string& id) : myId(id) {}

WfNode::~WfNode() {
  // Free the ports map
  while (! ports.empty() ) {
    WfPort * p = ports.begin()->second;
    ports.erase( ports.begin() );
    delete p;
  }
}

/****************************************************************************/
/*                              Basic GET/SET                               */
/****************************************************************************/

const string& WfNode::getId() { return this->myId; }

/****************************************************************************/
/*                       Predecessor/Successors Mgmt                        */
/****************************************************************************/

/**
 * (protected)
 * add a new previous node id *
 */
void
WfNode::addPrevId(const string& nodeId) {
  if (nodeId.empty()) {
    INTERNAL_ERROR("WfNode::addPrevId Fatal Error: Empty node id", 1);
  }
  prevNodeIds.insert(nodeId); // set insertion
}

/**
 * (protected)
 * remove a previous node id *
 */
void
WfNode::remPrevId(const string& nodeId) {
  if (nodeId.empty()) {
    INTERNAL_ERROR("WfNode::remPrevId Fatal Error: Empty node id", 1);
  }
  prevNodeIds.erase(nodeId); // set removal
}

/**
 * (public) used by Dag::checkPrec()
 * initializes the list of node predecessors using both
 * the control dependencies (<prec> tag) and the
 * data dependencies (ports links)
 */
void
WfNode::setNodePrecedence(NodeSet* nodeSet) throw (WfStructException) {
  // The predecessors defined by control links (<prec> tag) were
  // already added by the dag parser.
  // Add the predecessors defined by data links
  TRACE_TEXT (TRACE_ALL_STEPS, "Processing ports of node : " << myId << endl);
  for (map<string, WfPort*>::iterator p = ports.begin();
	 p != ports.end();
	 ++p) {
      ((WfPort*)p->second)->setNodePrecedence(nodeSet);
  }
  // convert the predecessors defined by ID in prevNodeIds to
  // direct object references stored in prevNodes
  prevNodes.resize(prevNodeIds.size());
  int prevIdx = 0;
  for (set<string>::iterator idIter = prevNodeIds.begin();
       idIter != prevNodeIds.end();
       ++idIter) {
    WfNode * prevNode = nodeSet->getNode(*idIter);
    setPrev(prevIdx++, prevNode);
  }
}

/**
 * (public) Add a new predecessor
 * (may check some constraints before adding the predecessor effectively)
 */
void
WfNode::addNodePredecessor(WfNode * node, const string& fullNodeId) {
  // no check is done in this class
  addPrevId(fullNodeId);
}

/**
 * (protected) Set a new previous node *
 * (does not check duplicates)
 */
void
WfNode::setPrev(int index, WfNode * node) {
    TRACE_TEXT (TRACE_ALL_STEPS, "The node " << myId << " has a new previous node " <<
 		node->getId() << endl);
    prevNodes[index] = node;
    node->addNext(this);
}

/**
 * return the number of previous nodes
 */
unsigned int
WfNode::prevNodesNb() {
  return prevNodes.size();
}

/**
 * return an iterator on the first previous nodes
 */
vector<WfNode*>::iterator
WfNode::prevNodesBegin() {
  return prevNodes.begin();
}

/**
 * return an iterator on the end of previous nodes
 */
vector<WfNode*>::iterator
WfNode::prevNodesEnd() {
  return prevNodes.end();
}

/**
 * Add a next node reference *
 */
void
WfNode::addNext(WfNode * node) {
  nextNodes.push_back(node);
}

/**
 * return the number of next nodes
 */
unsigned int
WfNode::nextNodesNb() {
  return nextNodes.size();
}

/**
 * return an iterator on the first next node
 */
list<WfNode*>::iterator
WfNode::nextNodesBegin() {
  return nextNodes.begin();
}

/**
 * return an iterator on the end of next nodes
 */
list<WfNode*>::iterator
WfNode::nextNodesEnd() {
  return nextNodes.end();
}

/**
 * return if the node is an input node *
 * only the nodes with no previous node are considered as dag input  *
 */
bool
WfNode::isAnInput() {
  return (prevNodes.size()==0);
}

/**
 * return true if the node is an input node *
 * only the nodes with no next node are considered as dag exit  *
 */
bool
WfNode::isAnExit() {
  return (nextNodes.size() == 0);
}


/****************************************************************************/
/*                            Ports Mgmt                                    */
/****************************************************************************/

/**
 * link the ports by references
 */
void
WfNode::connectNodePorts() throw (WfStructException) {
  TRACE_TEXT (TRACE_ALL_STEPS,
 	      "connectNodePorts : processing node " << getId() << endl);
  for (map<string, WfPort*>::iterator p = ports.begin();
       p != ports.end();
       ++p) {
    ((WfPort*)(p->second))->connectPorts();
  }
}

/**
 * check if port already exists
 */
bool
WfNode::isPortDefined(const string& id) {
  map<string, WfPort*>::iterator p = ports.find(id);
  return (p != ports.end());
}

/**
 * Add a new port to the ports map
 */
WfPort *
WfNode::addPort(const string& portId, WfPort* port) throw (WfStructException) {
  if (isPortDefined(portId)) {
    delete port;
    throw WfStructException(WfStructException::eDUPLICATE_PORT,"port id="+portId);
  }
  ports[portId] = port;
  return port;
}

/**
 * Get the input port references by id *
 */
WfPort *
WfNode::getPort(const string& id) throw (WfStructException) {
  map<string, WfPort*>::iterator p = ports.find(id);
  if (p != ports.end())
    return ((WfPort*)(p->second));
  else
    throw WfStructException(WfStructException::eUNKNOWN_PORT,
                            "node id=" + myId + "/port id=" + id);
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
  map<string, WfPort*>::const_iterator portIter = ports.begin();
  while (portIter != ports.end()) {
    WfPort *port = (WfPort*) portIter->second;
    if (port->getPortType() == portType)
      count++;
    portIter++;
  }
  return count;
}

/**
 * Get port by index
 */
const WfPort*
WfNode::getPortByIndex(unsigned int portIdx) const {
  map<string, WfPort*>::const_iterator portIter = ports.begin();
  while (portIter != ports.end()) {
    const WfPort *port = (const WfPort*) portIter->second;
    if (port->getIndex() == portIdx)
      return port;
    portIter++;
  }
  return NULL;
}

/**
 * Get description of ports (used for error msg)
 */
string
WfNode::getPortsDescr() const {
  string descrStr;
  for (int ix=0; ix<getPortNb(); ++ix) {
    const WfPort* port = getPortByIndex(ix);
    descrStr += port->getPortDescr();
    if (ix < getPortNb()-1)
      descrStr += ", ";
  }
  return descrStr;
}
