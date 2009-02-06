/****************************************************************************/
/* Port adapter classes used to split or merge container content            */
/*                                                                          */
/*                                                                          */
/* Author(s):                                                               */
/* - Benjamin Isnard (benjamin.isnard@ens-lyon.fr)                          */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.11  2009/02/06 14:54:43  bisnard
 * - setup exceptions
 * - added data type & depth check
 *
 * Revision 1.10  2008/12/02 10:14:51  bisnard
 * modified nodes links mgmt to handle inter-dags links
 *
 * Revision 1.9  2008/11/14 09:35:28  bdepardo
 * Bug correction in the multiple port adapter initialization
 *
 * Revision 1.8  2008/10/22 09:29:00  bisnard
 * replaced uint by standard type
 *
 * Revision 1.7  2008/10/14 13:31:01  bisnard
 * new class structure for dags (DagNode,DagNodePort)
 *
 * Revision 1.6  2008/10/02 09:10:51  bisnard
 * incorrect constant definition
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
 * Revision 1.2  2008/09/19 14:01:30  bisnard
 * allow compile wf support with or without DAGDA
 *
 * Revision 1.1  2008/09/19 13:16:07  bisnard
 * New class to manage containers split/merge in workflows
 *
 */

#include <sstream>
#include "DagWfParser.hh"
#include "Dag.hh"
#include "Node.hh"
#include "WfPort.hh"
#include "DagNode.hh"
#include "debug.hh"
#if HAVE_DAGDA
extern "C" {
#include "DIET_Dagda.h"
}
#endif

using namespace std;

/**
 * Base descructor
 */
WfPortAdapter::~WfPortAdapter() {}

/**
 * Static factory method for adapters
 * Note: the caller is responsible for freeing the memory
 */
WfPortAdapter*
WfPortAdapter::createAdapter(const string& strRef) {
  string::size_type refSep = strRef.find("(");
  if (refSep == string::npos) {
    return new WfSimplePortAdapter(strRef);
  } else {
    string::size_type refSepLast = strRef.rfind(")");
    if (refSepLast == string::npos) {
      INTERNAL_ERROR("No closing bracket in " << strRef << endl, 1);
    }
    return new WfMultiplePortAdapter(strRef.substr(refSep+1, refSepLast-1));
  }
}

/**
 * Constructor for simple port adapter
 * PARSING of a simple reference (with or without subports)
 */
WfSimplePortAdapter::WfSimplePortAdapter(const string& strRef) : dagName() {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter ref=" << strRef << endl);
  string::size_type nodeSep = strRef.find(":");
  string::size_type nodeStart = 0;
  if (nodeSep != string::npos) {
    this->dagName = strRef.substr(0, nodeSep);
    nodeStart = nodeSep + 1;
  }
  string::size_type portSep = strRef.find("#",nodeStart);
  if (portSep != string::npos) {
    this->nodeName = strRef.substr(nodeStart, portSep-nodeStart);
    string::size_type idxListLeft = strRef.find("[");
    if (idxListLeft == string::npos) {
      this->portName = strRef.substr(portSep+1);
    } else {
      this->portName = strRef.substr(portSep+1, idxListLeft-portSep-1);
      while (idxListLeft != string::npos) {
        string::size_type idxListRight = strRef.find("]",idxListLeft);
        if (idxListRight == string::npos) {
          // throw exception (brackets not closed)
        }
        unsigned int idx = atoi(strRef.substr(idxListLeft+1, idxListRight-idxListLeft-1).c_str());
        this->eltIdxList.push_back(idx);
        idxListLeft = strRef.find("[",idxListRight);
      }
    }
  } else {
    throw XMLParsingException(XMLParsingException::eINVALID_REF,
              "Invalid port reference (missing #) : " + strRef);
  }
}

WfSimplePortAdapter::WfSimplePortAdapter(WfPort * port,
                                         const string& portDagName) {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter TO port "
//                               << port->getId() << endl);
  nodePtr  = port->getParent();
  portPtr  = port;
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = portDagName;
}

WfSimplePortAdapter::WfSimplePortAdapter(WfSimplePortAdapter* parentAdapter,
                                         unsigned int index) {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating simple port adapter using parent adapter "
//                                << endl);
  portPtr  = parentAdapter->getSourcePort();
  nodePtr  = portPtr->getParent();
  nodeName = nodePtr->getId();
  portName = portPtr->getId();
  dagName  = parentAdapter->getDagName();
  eltIdxList = parentAdapter->getElementIndexes();
  eltIdxList.push_back(index);
}

WfSimplePortAdapter::~WfSimplePortAdapter() {}

string
WfSimplePortAdapter::getSourceRef() const {
  stringstream ss;
  if (!dagName.empty()) {
    ss << dagName << ":";
  }
  ss << nodeName << "#" << portName;
  for (list<unsigned int>::const_iterator idxIter = eltIdxList.begin();
       idxIter != eltIdxList.end();
       ++idxIter) {
    ss << "[" << (unsigned int) *idxIter << "]";
  }
  return ss.str();
}

void
WfSimplePortAdapter::setNodePrecedence(Node* node, NodeSet* nodeSet) throw (WfStructException) {
  // create the full node name (including dag prefix if needed)
  string dagPrefix;
  if (!dagName.empty()) {
    dagPrefix = dagName + ":";
  }
  // get the node pointer from the nodeSet
  nodePtr = nodeSet->getNode(dagPrefix + nodeName);
  node->addNodePredecessor(nodePtr, dagPrefix + nodePtr->getId());
}

/**
 * Initializes the connection between two WfPorts
 * This method searches the remote port then updates its own pointer
 * and calls the method WfPort::connectToPort on both sides
 * The adapterLevel parameter is used only to check depth compatibility between
 * the adapter and the linked port
 */
void
WfSimplePortAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
    throw (WfStructException)
{
  if (nodePtr == NULL) {
    INTERNAL_ERROR(__FUNCTION__ << "NULL node pointer" << endl, 1);
  }
  WfPort * linkedPort = nodePtr->getPort(portName);
  string errorMsg = "connect " + port->getCompleteId()
                       + " to " + getSourceRef();

  // check data type compatibility
  if (port->getBaseDataType() != linkedPort->getBaseDataType())
    throw WfStructException(WfStructException::eTYPE_MISMATCH,errorMsg);

  this->portPtr = linkedPort;       // SET the port ref FOR THE ADAPTER
  port->connectToPort(linkedPort);  // SET the connection on my port
  linkedPort->connectToPort(port);  // SET the connection on remote port

  // check data depth compatibility
  if (port->getDepth() != adapterLevel + linkedPort->getDepth() - getDepth())
    throw WfStructException(WfStructException::eDEPTH_MISMATCH,errorMsg);
}

WfPort *
WfSimplePortAdapter::getSourcePort() {
  return this->portPtr;
}

const string&
WfSimplePortAdapter::getSourceDataID() {
  DagNodeOutPort* dagNodeOutPortPtr = dynamic_cast<DagNodeOutPort*>(portPtr);
  if (dagNodeOutPortPtr == NULL) {
    INTERNAL_ERROR(__FUNCTION__ << "NULL port reference" << endl, 1);
  }
  if (getDepth() == 0)
    this->dataID = dagNodeOutPortPtr->getDataID();
  else
    this->dataID = dagNodeOutPortPtr->getElementDataID(eltIdxList);

  return this->dataID;
}

void
WfSimplePortAdapter::displayDataAsList(ostream& output) {
  DagNodeOutPort* dagNodeOutPortPtr = dynamic_cast<DagNodeOutPort*>(portPtr);
  if (dagNodeOutPortPtr == NULL) {
    INTERNAL_ERROR(__FUNCTION__ << "NULL port reference" << endl, 1);
  }
  if (getDepth() == 0)
    dagNodeOutPortPtr->displayDataAsList(output);
  else
    dagNodeOutPortPtr->displayDataElementAsList(output, eltIdxList);
}

const string&
WfSimplePortAdapter::getPortName() const {
  return portName;
}

const string&
WfSimplePortAdapter::getNodeName() const {
  return nodeName;
}

const string&
WfSimplePortAdapter::getDagName() const {
  return dagName;
}

unsigned int
WfSimplePortAdapter::getDepth() const {
  return eltIdxList.size();
}

const list<unsigned int>&
WfSimplePortAdapter::getElementIndexes() {
  return eltIdxList;
}

/**
 * Constructor for multiple port adapter
 * Builds a hierarchy of adapters
 */
WfMultiplePortAdapter::WfMultiplePortAdapter(const string& strRef) {
//   TRACE_TEXT (TRACE_ALL_STEPS,"Creating multiple ports adapter ref=[" << strRef << "]" << endl);
  string::size_type refStart = 0;
  while (refStart < strRef.length()) {
    string::size_type parLeft  = strRef.find("(",refStart);
    string::size_type refSep   = strRef.find(";",refStart);
    if (parLeft < refSep) { // multiple refs inside ( )
      string::size_type parRight = strRef.find(")",refStart);
      if (parRight != string::npos) {
        WfMultiplePortAdapter* mulAd =
            new WfMultiplePortAdapter(strRef.substr(parLeft+1, parRight-parLeft-1));
        adapters.push_back(mulAd);
        refStart = parRight+2; // right ) is either followed by ; or is at the end
      } else {
      // exception non-closed parenthesis
      }
    } else {  // simple ref
      string::size_type refEnd = (refSep == string::npos) ? strRef.length()-1 : refSep-1;
      WfSimplePortAdapter* splAd =
           new WfSimplePortAdapter(strRef.substr(refStart, refEnd-refStart+1));
      adapters.push_back(splAd);
      refStart = refEnd+2;
    }
  } // end while
}

WfMultiplePortAdapter::WfMultiplePortAdapter() {
}

WfMultiplePortAdapter::WfMultiplePortAdapter(const WfMultiplePortAdapter& mpa) {
  // WfMultiplePortAdapter: COPY CONSTRUCTOR NOT DEFINED
}

WfMultiplePortAdapter::~WfMultiplePortAdapter() {
  // Free the adapters list
  while (! adapters.empty() ) {
    WfPortAdapter * p = adapters.front();
    adapters.pop_front();
    delete p;
  }
}

void
WfMultiplePortAdapter::addSubAdapter(WfPortAdapter* subAdapter) {
  adapters.push_back(subAdapter);
}

void
WfMultiplePortAdapter::setNodePrecedence(Node* node, NodeSet* nodeSet)
    throw (WfStructException)
{
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->setNodePrecedence(node, nodeSet);
  }
}

void
WfMultiplePortAdapter::connectPorts(WfPort* port, unsigned int adapterLevel)
    throw (WfStructException)
{
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
    (*iter)->connectPorts(port, adapterLevel+1);
  }
}

string WfMultiplePortAdapter::errorID = string("ID_Error");

string
WfMultiplePortAdapter::getSourceRef() const {
  string s = "(";
  list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
  while (adaptIter != adapters.end()) {
    s += ((WfPortAdapter*) *adaptIter)->getSourceRef();
    if (++adaptIter != adapters.end())
      s += ";";
  }
  s += ")";
  return s;
}

const string&
WfMultiplePortAdapter::getSourceDataID() {
  char* idCont;
#if HAVE_DAGDA
  TRACE_TEXT (TRACE_ALL_STEPS,"## Creating container to merge ports" << endl);
  dagda_create_container(&idCont);
  int ix=0;
  for (list<WfPortAdapter*>::iterator iter = adapters.begin();
       iter != adapters.end();
       ++iter) {
     const string& idElt = (*iter)->getSourceDataID();
     TRACE_TEXT(TRACE_ALL_STEPS, "## merging " << idElt << " into " << idCont << std::endl);
     dagda_add_container_element(idCont,idElt.c_str(),ix++);
  }
  containerID = idCont;
  TRACE_TEXT (TRACE_ALL_STEPS,"## End of merge ports" << endl);
#else
  ERROR("WfMultiplePortAdapter Error: trying to use containers without Dagda enabled" << endl, errorID);
#endif
  return containerID;
}

void
WfMultiplePortAdapter::displayDataAsList(ostream& output) {
  output << "(";
  list<WfPortAdapter*>::const_iterator adaptIter = adapters.begin();
  while (adaptIter != adapters.end()) {
    ((WfPortAdapter*) *adaptIter)->displayDataAsList(output);
    if (++adaptIter != adapters.end())
      output << ",";
  }
  output << ")";
}

